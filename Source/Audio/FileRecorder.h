#pragma once

#include "AudioCaptureBus.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

namespace cendance {

/**
 * FileRecorder — Drains an AudioCaptureBus and writes to a WAV or FLAC file
 * on a background thread.
 *
 * Real-time safety: This class does NOT touch the audio callback.
 * It only reads from the lock-free ring buffer via AudioCaptureBus::pop().
 */
class FileRecorder {
public:
    enum class Format {
        WavF32,   // WAV, 32-bit float
        WavS16,   // WAV, 16-bit signed int
        Flac24,   // FLAC, 24-bit
        Flac16,   // FLAC, 16-bit
    };

    struct Info {
        std::string filePath;
        Format format = Format::WavF32;
        int sampleRate = 48000;
        int numChannels = 2;
    };

    struct Status {
        bool recording = false;
        bool overrun = false;
        double durationSeconds = 0.0;
        uint64_t totalSamplesWritten = 0;
        uint64_t totalSamplesDropped = 0;
        std::string filePath;
        std::string lastError;
    };

    FileRecorder();
    ~FileRecorder();

    // Non-copyable, non-movable
    FileRecorder(const FileRecorder&) = delete;
    FileRecorder& operator=(const FileRecorder&) = delete;

    // Start recording. Returns false on error (check status.lastError).
    bool start(AudioCaptureBus& bus, const Info& info);

    // Stop recording and wait for the writer thread to finish.
    void stop();

    // Query current status (thread-safe).
    Status getStatus() const;

    // Convert format enum to file extension.
    static std::string formatToExtension(Format format);

private:
    void writerThreadFunc();

    AudioCaptureBus* bus_ = nullptr;
    Info info_;
    std::thread writerThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> recording_{false};
    std::atomic<bool> overrun_{false};
    std::atomic<uint64_t> totalSamplesWritten_{0};
    std::string lastError_;
    std::chrono::steady_clock::time_point startTime_;
};

} // namespace cendance
