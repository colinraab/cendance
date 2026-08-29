#include "StreamSink.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>

namespace cendance {

// ──────────────────────────────────────────────────────────────────────
// Ring buffer (SPSC, power-of-2 size, lock-free)
// ──────────────────────────────────────────────────────────────────────

struct StreamSink::RingBuffer {
    int numChannels = 2;
    int capacity = 0;          // total floats (must be power of 2)
    float* buffer = nullptr;
    alignas(64) std::atomic<int> writeIdx{0};
    alignas(64) std::atomic<int> readIdx{0};

    RingBuffer() = default;

    void init(int channels, int capacitySeconds, int sampleRate) {
        numChannels = channels;
        int frames = sampleRate * capacitySeconds;
        int total = frames * channels;
        int pow2 = 1;
        while (pow2 < total) pow2 <<= 1;
        capacity = pow2;
        buffer = new (std::nothrow) float[static_cast<size_t>(capacity)];
        if (buffer)
            std::fill_n(buffer, capacity, 0.0f);
    }

    ~RingBuffer() { delete[] buffer; }

    // Audio callback pushes interleaved samples
    int push(const float* const* channels, int numSamples) {
        if (!buffer || numSamples <= 0) return 0;

        int w = writeIdx.load(std::memory_order_relaxed);
        int r = readIdx.load(std::memory_order_acquire);
        int mask = capacity - 1;
        int totalSamples = numSamples * numChannels;
        int written = (w - r + capacity) & mask;
        int available = capacity - written - 1;

        if (available <= 0) return 0; // full, drop

        int toWrite = std::min(totalSamples, available);
        int framesToWrite = toWrite / numChannels;
        int samplesToWrite = framesToWrite * numChannels;

        for (int ch = 0; ch < numChannels; ++ch) {
            if (!channels[ch]) continue;
            int bufIdx = (w + ch) & mask;
            for (int i = 0; i < framesToWrite; ++i) {
                buffer[bufIdx] = channels[ch][i];
                bufIdx = (bufIdx + numChannels) & mask;
            }
        }

        writeIdx.store((w + samplesToWrite) & mask, std::memory_order_release);
        return framesToWrite;
    }

    // Consumer pops interleaved samples into output
    int pop(float* output, int numFrames) {
        if (!buffer || numFrames <= 0) return 0;

        int w = writeIdx.load(std::memory_order_acquire);
        int r = readIdx.load(std::memory_order_relaxed);
        int mask = capacity - 1;
        int available = (w - r + capacity) & mask;
        int framesAvailable = available / numChannels;
        int framesToRead = std::min(numFrames, framesAvailable);
        int samplesToRead = framesToRead * numChannels;

        if (samplesToRead <= 0) return 0;

        for (int ch = 0; ch < numChannels; ++ch) {
            int bufIdx = (r + ch) & mask;
            for (int i = 0; i < framesToRead; ++i) {
                output[i * numChannels + ch] = buffer[bufIdx];
                bufIdx = (bufIdx + numChannels) & mask;
            }
        }

        readIdx.store((r + samplesToRead) & mask, std::memory_order_release);
        return framesToRead;
    }
};

// ──────────────────────────────────────────────────────────────────────
// StreamSink
// ──────────────────────────────────────────────────────────────────────

StreamSink::StreamSink() = default;

StreamSink::~StreamSink() {
    stop();
}

bool StreamSink::start(const Config& config, SinkFn sink) {
    if (active_.load(std::memory_order_acquire)) {
        lastError_ = "Already streaming";
        return false;
    }

    ring_ = std::make_unique<RingBuffer>();
    ring_->init(config.numChannels, config.capacitySeconds, config.sampleRate);

    if (!ring_->buffer) {
        lastError_ = "Failed to allocate ring buffer";
        return false;
    }

    config_ = config;
    sinkFn_ = std::move(sink);
    stopRequested_.store(false, std::memory_order_release);
    overrun_.store(false, std::memory_order_relaxed);
    totalWritten_.store(0, std::memory_order_relaxed);
    totalDropped_.store(0, std::memory_order_relaxed);
    lastError_.clear();

    active_.store(true, std::memory_order_release);
    thread_ = std::thread(&StreamSink::streamThreadFunc, this);
    return true;
}

void StreamSink::stop() {
    if (!active_.load(std::memory_order_acquire)) return;
    stopRequested_.store(true, std::memory_order_release);
    if (thread_.joinable()) thread_.join();
    active_.store(false, std::memory_order_release);
}

int StreamSink::push(const float* const* channels, int numSamples) {
    if (!ring_ || !active_.load(std::memory_order_acquire) || numSamples <= 0)
        return 0;

    int frames = ring_->push(channels, numSamples);
    if (frames <= 0) {
        overrun_.store(true, std::memory_order_relaxed);
        totalDropped_.fetch_add(static_cast<uint64_t>(numSamples * ring_->numChannels), std::memory_order_relaxed);
        return 0;
    }
    totalWritten_.fetch_add(static_cast<uint64_t>(frames * ring_->numChannels), std::memory_order_relaxed);
    return frames;
}

StreamSink::Status StreamSink::getStatus() const {
    Status s;
    s.streaming = active_.load(std::memory_order_acquire);
    s.overrun = overrun_.load(std::memory_order_relaxed);
    s.totalSamplesWritten = totalWritten_.load(std::memory_order_relaxed);
    s.totalSamplesDropped = totalDropped_.load(std::memory_order_relaxed);
    s.lastError = lastError_;
    return s;
}

void StreamSink::streamThreadFunc() {
    if (!ring_ || !sinkFn_) return;

    const int numChannels = config_.numChannels;
    const int chunkSize = 4096;
    std::vector<float> interleaved(static_cast<size_t>(chunkSize * numChannels));

    while (!stopRequested_.load(std::memory_order_acquire)) {
        int framesRead = ring_->pop(interleaved.data(), chunkSize);
        if (framesRead <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        const int samplesRead = framesRead * numChannels;

        if (config_.format == Format::F32LE) {
            // Native float32 — just dump bytes
            size_t bytes = static_cast<size_t>(samplesRead) * sizeof(float);
            if (!sinkFn_(interleaved.data(), bytes)) {
                lastError_ = "Sink write failed";
                break;
            }
        } else {
            // Convert float32 → int16
            std::vector<int16_t> s16(static_cast<size_t>(samplesRead));
            for (int i = 0; i < samplesRead; ++i) {
                float s = std::clamp(interleaved[static_cast<size_t>(i)], -1.0f, 1.0f);
                s16[static_cast<size_t>(i)] = static_cast<int16_t>(s * 32767.0f);
            }
            size_t bytes = s16.size() * sizeof(int16_t);
            if (!sinkFn_(s16.data(), bytes)) {
                lastError_ = "Sink write failed";
                break;
            }
        }
    }
}

} // namespace cendance
