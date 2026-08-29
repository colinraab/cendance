#include "AudioCaptureBus.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <new>

namespace cendance {

AudioCaptureBus::AudioCaptureBus()
    : numChannels_(2)
    , sampleRate_(48000)
    , capacity_(0)
    , buffer_(nullptr)
    , writeIndex_(0)
    , readIndex_(0)
    , active_(false)
    , totalWritten_(0)
    , totalDropped_(0)
    , overrun_(false)
{
}

AudioCaptureBus::AudioCaptureBus(const Config& config)
    : AudioCaptureBus()
{
    // Capacity in frames (one frame = numChannels samples)
    const int frames = config.sampleRate * config.capacitySeconds;
    capacity_ = frames * config.numChannels;

    // Round up to power of 2 for fast modulo
    int pow2 = 1;
    while (pow2 < capacity_)
        pow2 <<= 1;
    capacity_ = pow2;

    numChannels_ = config.numChannels;
    sampleRate_ = config.sampleRate;

    buffer_ = new (std::nothrow) float[static_cast<size_t>(capacity_)];
    if (buffer_)
        std::fill_n(buffer_, capacity_, 0.0f);
}

AudioCaptureBus::~AudioCaptureBus() {
    delete[] buffer_;
}

int AudioCaptureBus::push(const float* const* channels, int numSamples) {
    if (!buffer_ || !active_.load(std::memory_order_acquire) || numSamples <= 0)
        return 0;

    const int writeIdx = writeIndex_.load(std::memory_order_relaxed);
    const int readIdx = readIndex_.load(std::memory_order_acquire);

    const int totalSamples = numSamples * numChannels_;
    const int mask = capacity_ - 1;

    // Available space
    const int written = (writeIdx - readIdx + capacity_) & mask;
    const int available = capacity_ - written - 1; // leave one slot empty

    if (available <= 0) {
        // Buffer full — drop all
        totalDropped_.fetch_add(static_cast<uint64_t>(totalSamples), std::memory_order_relaxed);
        overrun_.store(true, std::memory_order_relaxed);
        return 0;
    }

    const int toWrite = std::min(totalSamples, available);
    const int framesToWrite = toWrite / numChannels_;
    const int samplesToWrite = framesToWrite * numChannels_;

    // Interleave and write
    for (int ch = 0; ch < numChannels_; ++ch) {
        if (channels[ch] == nullptr) continue;
        int bufIdx = (writeIdx + ch) & mask;
        for (int i = 0; i < framesToWrite; ++i) {
            buffer_[bufIdx] = channels[ch][i];
            bufIdx = (bufIdx + numChannels_) & mask;
        }
    }

    // If we couldn't write everything, count dropped samples
    if (samplesToWrite < totalSamples) {
        totalDropped_.fetch_add(static_cast<uint64_t>(totalSamples - samplesToWrite), std::memory_order_relaxed);
        overrun_.store(true, std::memory_order_relaxed);
    }

    // Advance write index
    writeIndex_.store((writeIdx + samplesToWrite) & mask, std::memory_order_release);
    totalWritten_.fetch_add(static_cast<uint64_t>(samplesToWrite), std::memory_order_relaxed);

    return framesToWrite;
}

int AudioCaptureBus::pop(float* output, int numSamples) {
    if (!buffer_ || numSamples <= 0)
        return 0;

    const int writeIdx = writeIndex_.load(std::memory_order_acquire);
    const int readIdx = readIndex_.load(std::memory_order_relaxed);

    const int mask = capacity_ - 1;
    const int available = (writeIdx - readIdx + capacity_) & mask;
    const int framesAvailable = available / numChannels_;
    const int framesToRead = std::min(numSamples, framesAvailable);
    const int samplesToRead = framesToRead * numChannels_;

    if (samplesToRead <= 0)
        return 0;

    // Deinterleave and read
    for (int ch = 0; ch < numChannels_; ++ch) {
        int bufIdx = (readIdx + ch) & mask;
        for (int i = 0; i < framesToRead; ++i) {
            output[i * numChannels_ + ch] = buffer_[bufIdx];
            bufIdx = (bufIdx + numChannels_) & mask;
        }
    }

    readIndex_.store((readIdx + samplesToRead) & mask, std::memory_order_release);

    return framesToRead;
}

AudioCaptureBus::RecordingState AudioCaptureBus::getState() const {
    RecordingState state;
    state.active = active_.load(std::memory_order_acquire);
    state.overrun = overrun_.load(std::memory_order_relaxed);
    state.totalSamplesWritten = totalWritten_.load(std::memory_order_relaxed);
    state.totalSamplesDropped = totalDropped_.load(std::memory_order_relaxed);
    state.sampleRate = sampleRate_;
    state.numChannels = numChannels_;
    return state;
}

void AudioCaptureBus::reset() {
    writeIndex_.store(0, std::memory_order_relaxed);
    readIndex_.store(0, std::memory_order_relaxed);
    totalWritten_.store(0, std::memory_order_relaxed);
    totalDropped_.store(0, std::memory_order_relaxed);
    overrun_.store(false, std::memory_order_relaxed);
    if (buffer_)
        std::fill_n(buffer_, capacity_, 0.0f);
}

} // namespace cendance
