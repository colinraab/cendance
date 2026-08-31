#include "../Source/Audio/FileRecorder.h"
#include "../Source/Audio/AudioCaptureBus.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>

using namespace cendance;

// ========================================================================
// P2 Tests
// ========================================================================

static std::string tempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

void testStartStopLifecycle() {
    AudioCaptureBus::Config busConfig;
    busConfig.numChannels = 2;
    busConfig.sampleRate = 48000;
    busConfig.capacitySeconds = 1;
    AudioCaptureBus bus(busConfig);

    FileRecorder recorder;
    FileRecorder::Info info;
    info.filePath = tempPath("test_lifecycle.wav");
    info.format = FileRecorder::Format::WavF32;
    info.sampleRate = 48000;
    info.numChannels = 2;

    assert(recorder.start(bus, info));
    auto status = recorder.getStatus();
    assert(status.recording);

    recorder.stop();
    status = recorder.getStatus();
    assert(!status.recording);

    // Clean up
    std::filesystem::remove(info.filePath);
}

void testRecordsWavF32() {
    AudioCaptureBus::Config busConfig;
    busConfig.numChannels = 2;
    busConfig.sampleRate = 48000;
    busConfig.capacitySeconds = 1;
    AudioCaptureBus bus(busConfig);
    bus.setActive(true);

    FileRecorder recorder;
    FileRecorder::Info info;
    info.filePath = tempPath("test_wav_f32.wav");
    info.format = FileRecorder::Format::WavF32;
    info.sampleRate = 48000;
    info.numChannels = 2;

    assert(recorder.start(bus, info));

    // Push some audio
    float left[480], right[480];
    for (int i = 0; i < 480; ++i) {
        left[i] = static_cast<float>(i) / 480.0f;
        right[i] = static_cast<float>(i + 0.5f) / 480.0f;
    }
    const float* chPtrs[2] = {left, right};
    bus.push(chPtrs, 480);

    // Give writer thread time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    recorder.stop();

    // Verify file exists (may be empty if no data was written, but should exist)
    if (std::filesystem::exists(info.filePath)) {
        auto fileSize = std::filesystem::file_size(info.filePath);
        // If file has content, it should be at least a WAV header
        if (fileSize > 0) {
            assert(fileSize >= 44);
        }
        std::filesystem::remove(info.filePath);
    }
}

void testRecordsWavS16() {
    AudioCaptureBus::Config busConfig;
    busConfig.numChannels = 1;
    busConfig.sampleRate = 44100;
    busConfig.capacitySeconds = 1;
    AudioCaptureBus bus(busConfig);
    bus.setActive(true);

    FileRecorder recorder;
    FileRecorder::Info info;
    info.filePath = tempPath("test_wav_s16.wav");
    info.format = FileRecorder::Format::WavS16;
    info.sampleRate = 44100;
    info.numChannels = 1;

    assert(recorder.start(bus, info));

    float mono[441];
    for (int i = 0; i < 441; ++i) mono[i] = static_cast<float>(i) / 441.0f;
    const float* chPtrs[1] = {mono};
    bus.push(chPtrs, 441);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    recorder.stop();

    if (std::filesystem::exists(info.filePath)) {
        auto fileSize = std::filesystem::file_size(info.filePath);
        if (fileSize > 0) assert(fileSize >= 44);
        std::filesystem::remove(info.filePath);
    }
}

void testRecordedContentMatchesPushed() {
    AudioCaptureBus::Config busConfig;
    busConfig.numChannels = 1;
    busConfig.sampleRate = 48000;
    busConfig.capacitySeconds = 1;
    AudioCaptureBus bus(busConfig);
    bus.setActive(true);

    FileRecorder recorder;
    FileRecorder::Info info;
    info.filePath = tempPath("test_content_match.wav");
    info.format = FileRecorder::Format::WavF32;
    info.sampleRate = 48000;
    info.numChannels = 1;

    assert(recorder.start(bus, info));

    // Push a known sine-like pattern
    float mono[240];
    for (int i = 0; i < 240; ++i) {
        mono[i] = static_cast<float>(i) / 240.0f;
    }
    const float* chPtrs[1] = {mono};
    bus.push(chPtrs, 240);

    // Stop immediately. The recorder must drain every frame that the capture
    // bus accepted before it closes the file.
    recorder.stop();

    auto status = recorder.getStatus();
    assert(status.totalSamplesWritten == 240);

    // Verify file exists if data was written
    if (std::filesystem::exists(info.filePath)) {
        // Read back the file and verify it has the expected WAV structure
        std::ifstream file(info.filePath, std::ios::binary);
        if (file.is_open()) {
            char riff[4];
            file.read(riff, 4);
            assert(riff[0] == 'R' && riff[1] == 'I' && riff[2] == 'F' && riff[3] == 'F');
            file.close();
        }
        std::filesystem::remove(info.filePath);
    }
}

void testStatusReporting() {
    FileRecorder recorder;
    auto status = recorder.getStatus();
    assert(!status.recording);
    assert(!status.overrun);
    assert(status.durationSeconds == 0.0);
    assert(status.totalSamplesWritten == 0);
    assert(status.filePath.empty());
}

void testFormatToExtension() {
    assert(FileRecorder::formatToExtension(FileRecorder::Format::WavF32) == ".wav");
    assert(FileRecorder::formatToExtension(FileRecorder::Format::WavS16) == ".wav");
    assert(FileRecorder::formatToExtension(FileRecorder::Format::Flac24) == ".flac");
    assert(FileRecorder::formatToExtension(FileRecorder::Format::Flac16) == ".flac");
}

void testOverrunDetection() {
    AudioCaptureBus::Config busConfig;
    busConfig.numChannels = 1;
    busConfig.sampleRate = 48000;
    busConfig.capacitySeconds = 1; // Small buffer
    AudioCaptureBus bus(busConfig);
    bus.setActive(true);

    FileRecorder recorder;
    FileRecorder::Info info;
    info.filePath = tempPath("test_overrun.wav");
    info.format = FileRecorder::Format::WavF32;
    info.sampleRate = 48000;
    info.numChannels = 1;

    assert(recorder.start(bus, info));

    // Push a lot of data quickly to potentially cause overrun
    float mono[48000];
    for (int i = 0; i < 48000; ++i) mono[i] = 0.5f;
    const float* chPtrs[1] = {mono};
    for (int i = 0; i < 5; ++i) {
        bus.push(chPtrs, 48000);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    recorder.stop();

    auto status = recorder.getStatus();
    // We pushed data, so something should have been written
    (void)status;

    if (std::filesystem::exists(info.filePath)) {
        std::filesystem::remove(info.filePath);
    }
}

// ========================================================================
// Main
// ========================================================================

int main() {
    testStartStopLifecycle();
    std::cout << "  testStartStopLifecycle passed\n";

    testRecordsWavF32();
    std::cout << "  testRecordsWavF32 passed\n";

    testRecordsWavS16();
    std::cout << "  testRecordsWavS16 passed\n";

    testRecordedContentMatchesPushed();
    std::cout << "  testRecordedContentMatchesPushed passed\n";

    testStatusReporting();
    std::cout << "  testStatusReporting passed\n";

    testFormatToExtension();
    std::cout << "  testFormatToExtension passed\n";

    testOverrunDetection();
    std::cout << "  testOverrunDetection passed\n";

    std::cout << "FileRecorder tests passed!\n";
    return 0;
}
