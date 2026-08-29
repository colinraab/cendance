#include "FileRecorder.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

namespace cendance {

FileRecorder::FileRecorder() = default;

FileRecorder::~FileRecorder() {
    stop();
}

bool FileRecorder::start(AudioCaptureBus& bus, const Info& info) {
    if (recording_.load(std::memory_order_acquire)) {
        lastError_ = "Already recording";
        return false;
    }

    if (info.filePath.empty()) {
        lastError_ = "Empty file path";
        return false;
    }

    if (info.sampleRate <= 0 || info.numChannels <= 0) {
        lastError_ = "Invalid sample rate or channel count";
        return false;
    }

    bus_ = &bus;
    info_ = info;
    stopRequested_.store(false, std::memory_order_release);
    overrun_.store(false, std::memory_order_relaxed);
    totalSamplesWritten_.store(0, std::memory_order_relaxed);
    lastError_.clear();
    startTime_ = std::chrono::steady_clock::now();

    recording_.store(true, std::memory_order_release);
    writerThread_ = std::thread(&FileRecorder::writerThreadFunc, this);
    return true;
}

void FileRecorder::stop() {
    if (!recording_.load(std::memory_order_acquire))
        return;

    stopRequested_.store(true, std::memory_order_release);

    if (writerThread_.joinable()) {
        writerThread_.join();
    }

    recording_.store(false, std::memory_order_release);
}

FileRecorder::Status FileRecorder::getStatus() const {
    Status status;
    status.recording = recording_.load(std::memory_order_acquire);
    status.overrun = overrun_.load(std::memory_order_relaxed);
    status.totalSamplesWritten = totalSamplesWritten_.load(std::memory_order_relaxed);
    status.filePath = info_.filePath;

    if (status.recording) {
        auto now = std::chrono::steady_clock::now();
        status.durationSeconds = std::chrono::duration<double>(now - startTime_).count();
    }

    if (bus_) {
        auto busState = bus_->getState();
        status.totalSamplesDropped = busState.totalSamplesDropped;
    }

    status.lastError = lastError_;
    return status;
}

std::string FileRecorder::formatToExtension(Format format) {
    switch (format) {
        case Format::WavF32:
        case Format::WavS16: return ".wav";
        case Format::Flac24:
        case Format::Flac16: return ".flac";
    }
    return ".wav";
}

void FileRecorder::writerThreadFunc() {
    if (!bus_)
        return;

    const int numChannels = info_.numChannels;
    const int chunkSize = 4096; // frames per write

    // Create the output file and writer in this thread
    juce::File file(info_.filePath);
    file.deleteFile();
    file.getParentDirectory().createDirectory();

    // Create format
    std::unique_ptr<juce::AudioFormat> format;
    switch (info_.format) {
        case Format::WavF32:
        case Format::WavS16:
            format = std::make_unique<juce::WavAudioFormat>();
            break;
        case Format::Flac24:
        case Format::Flac16:
            format = std::make_unique<juce::FlacAudioFormat>();
            break;
    }

    if (!format) {
        lastError_ = "Unsupported format";
        return;
    }

    // Create output stream and writer
    std::unique_ptr<juce::OutputStream> outputStream(file.createOutputStream());
    if (!outputStream) {
        lastError_ = "Failed to create output stream: " + info_.filePath;
        return;
    }

    const int bitsPerSample = (info_.format == Format::WavF32) ? 32
                            : (info_.format == Format::WavS16) ? 16
                            : (info_.format == Format::Flac24) ? 24
                            : 16;

    juce::StringPairArray metadata;
    metadata.set("encoder", "cendance");

    // Use the deprecated createWriterFor — the newer AudioFormatWriterOptions
    // builder API is not worth the complexity for this use case.
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    std::unique_ptr<juce::AudioFormatWriter> writer(
        format->createWriterFor(outputStream.get(),
                                static_cast<double>(info_.sampleRate),
                                static_cast<unsigned int>(info_.numChannels),
                                bitsPerSample,
                                metadata,
                                0));
    #pragma clang diagnostic pop

    if (!writer) {
        lastError_ = "Failed to create audio writer for: " + info_.filePath;
        return;
    }

    // Release the outputStream — writer now owns it
    outputStream.release();

    // Pre-allocate an AudioSampleBuffer for writing
    juce::AudioSampleBuffer audioBuffer(numChannels, chunkSize);

    // Pre-allocate temp buffer for reading from ring buffer (interleaved)
    std::vector<float> interleaved(static_cast<size_t>(chunkSize * numChannels));

    while (!stopRequested_.load(std::memory_order_acquire)) {
        int framesRead = bus_->pop(interleaved.data(), chunkSize);

        if (framesRead <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Check for overrun
        auto busState = bus_->getState();
        if (busState.overrun) {
            overrun_.store(true, std::memory_order_relaxed);
        }

        // Deinterleaved into AudioSampleBuffer
        audioBuffer.clear();
        for (int ch = 0; ch < numChannels; ++ch) {
            float* dest = audioBuffer.getWritePointer(ch);
            for (int i = 0; i < framesRead; ++i) {
                dest[i] = interleaved[static_cast<size_t>(i * numChannels + ch)];
            }
        }

        // Write to file
        if (!writer->writeFromAudioSampleBuffer(audioBuffer, 0, framesRead)) {
            lastError_ = "Write error at sample " + std::to_string(totalSamplesWritten_.load());
            break;
        }

        totalSamplesWritten_.fetch_add(static_cast<uint64_t>(framesRead * numChannels),
                                       std::memory_order_relaxed);
    }

    // Flush and close
    writer->flush();
    writer.reset();
}

} // namespace cendance
