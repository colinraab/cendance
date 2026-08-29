#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>

namespace cendance {

/**
 * AudioCaptureBus — A real-time-safe, lock-free SPSC ring buffer
 * for capturing audio from the audio callback to consumer threads.
 *
 * Single producer (audio callback), single consumer (writer thread).
 * Pre-allocated, non-blocking, bounded. If the consumer falls behind,
 * samples are dropped and an overrun counter is incremented.
 *
 * Thread safety:
 *   - push() is called from the audio thread only
 *   - pop() is called from the consumer thread only
 *   - isActive()/setActive() use atomics and are safe from any thread
 */
class AudioCaptureBus {
public:
    struct Config {
        int numChannels = 2;
        int sampleRate = 48000;
        int capacitySeconds = 10; // ring buffer capacity in seconds
    };

    struct RecordingState {
        bool active = false;
        bool overrun = false;
        uint64_t totalSamplesWritten = 0;
        uint64_t totalSamplesDropped = 0;
        int sampleRate = 48000;
        int numChannels = 2;
    };

    AudioCaptureBus();
    explicit AudioCaptureBus(const Config& config);
    ~AudioCaptureBus();

    // Non-copyable, non-movable
    AudioCaptureBus(const AudioCaptureBus&) = delete;
    AudioCaptureBus& operator=(const AudioCaptureBus&) = delete;
    AudioCaptureBus(AudioCaptureBus&&) = delete;
    AudioCaptureBus& operator=(AudioCaptureBus&&) = delete;

    // Called from the audio callback (real-time thread)
    // Copies interleaved samples from channel pointers into the ring buffer.
    // Returns the number of samples actually written (may be less than numSamples if overrun).
    int push(const float* const* channels, int numSamples);

    // Called from the consumer thread
    // Pops up to numSamples interleaved samples into output.
    // output must have enough space for numSamples * numChannels floats.
    // Returns the number of samples actually read.
    int pop(float* output, int numSamples);

    // State queries
    bool isActive() const { return active_.load(std::memory_order_acquire); }
    void setActive(bool active) { active_.store(active, std::memory_order_release); }

    RecordingState getState() const;
    void reset();

    int getNumChannels() const { return numChannels_; }
    int getSampleRate() const { return sampleRate_; }

private:
    int numChannels_;
    int sampleRate_;
    int capacity_;

    // Ring buffer storage (interleaved: ch0, ch1, ch0, ch1, ...)
    float* buffer_;

    // Lock-free SPSC indices
    alignas(64) std::atomic<int> writeIndex_;
    alignas(64) std::atomic<int> readIndex_;

    std::atomic<bool> active_;
    std::atomic<uint64_t> totalWritten_;
    std::atomic<uint64_t> totalDropped_;
    std::atomic<bool> overrun_;
};

} // namespace cendance
