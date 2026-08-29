#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <functional>

namespace cendance {

/**
 * StreamSink — Drains an AudioCaptureBus and outputs raw PCM to a sink function
 * on a background thread.
 *
 * The sink function receives interleaved float32 samples.
 * For stdout streaming, the sink writes binary data to fd 1.
 * For TCP streaming, the sink writes to a connected socket.
 *
 * Real-time safety: This class does NOT touch the audio callback.
 * It only reads from the lock-free ring buffer via AudioCaptureBus::pop().
 *
 * NOTE: AudioCaptureBus is SPSC. To support both FileRecorder and StreamSink
 * as concurrent consumers, StreamSink uses a separate ring buffer that is
 * fed directly from the audio callback via push().
 */
class StreamSink {
public:
    enum class Format {
        F32LE,   // 32-bit float little-endian (native)
        S16LE,   // 16-bit signed int little-endian
    };

    struct Config {
        int numChannels = 2;
        int sampleRate = 48000;
        int capacitySeconds = 5;
        Format format = Format::F32LE;
    };

    struct Status {
        bool streaming = false;
        bool overrun = false;
        double durationSeconds = 0.0;
        uint64_t totalSamplesWritten = 0;
        uint64_t totalSamplesDropped = 0;
        std::string lastError;
    };

    using SinkFn = std::function<bool(const void* data, size_t bytes)>;

    StreamSink();
    ~StreamSink();

    StreamSink(const StreamSink&) = delete;
    StreamSink& operator=(const StreamSink&) = delete;

    // Start streaming. The sink function is called from the background thread
    // with raw PCM bytes. Returns false on error.
    bool start(const Config& config, SinkFn sink);

    // Stop streaming and wait for the thread to finish.
    void stop();

    // Push audio from the audio callback (real-time safe).
    // Returns number of frames actually written.
    int push(const float* const* channels, int numSamples);

    // Query status (thread-safe).
    Status getStatus() const;

private:
    struct RingBuffer;

    std::unique_ptr<RingBuffer> ring_;
    Config config_;
    SinkFn sinkFn_;
    std::thread thread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> active_{false};
    std::atomic<bool> overrun_{false};
    std::atomic<uint64_t> totalWritten_{0};
    std::atomic<uint64_t> totalDropped_{0};
    std::string lastError_;

    void streamThreadFunc();
};

} // namespace cendance
