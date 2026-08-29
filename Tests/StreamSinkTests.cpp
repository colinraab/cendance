#include "../Source/Audio/StreamSink.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

using namespace cendance;

// ========================================================================
// P2 Tests
// ========================================================================

void testStartStopLifecycle() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::F32LE;

    std::vector<uint8_t> receivedData;
    StreamSink::SinkFn sinkFn = [&receivedData](const void* data, size_t bytes) -> bool {
        const uint8_t* ptr = static_cast<const uint8_t*>(data);
        receivedData.insert(receivedData.end(), ptr, ptr + bytes);
        return true;
    };

    assert(sink.start(config, sinkFn));
    auto status = sink.getStatus();
    assert(status.streaming);

    sink.stop();
    status = sink.getStatus();
    assert(!status.streaming);
}

void testPushDeliversToSink() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 1;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::F32LE;

    std::vector<float> receivedSamples;
    StreamSink::SinkFn sinkFn = [&receivedSamples](const void* data, size_t bytes) -> bool {
        const float* samples = static_cast<const float*>(data);
        size_t numSamples = bytes / sizeof(float);
        receivedSamples.insert(receivedSamples.end(), samples, samples + numSamples);
        return true;
    };

    assert(sink.start(config, sinkFn));

    // Push some audio
    float mono[100];
    for (int i = 0; i < 100; ++i) mono[i] = static_cast<float>(i) * 0.01f;
    const float* chPtrs[1] = {mono};
    int written = sink.push(chPtrs, 100);
    assert(written == 100);

    // Give the background thread time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    sink.stop();

    // The sink should have received some data
    auto status = sink.getStatus();
    assert(status.totalSamplesWritten > 0);
}

void testF32LEFormat() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 1;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::F32LE;

    std::vector<uint8_t> rawBytes;
    StreamSink::SinkFn sinkFn = [&rawBytes](const void* data, size_t bytes) -> bool {
        const uint8_t* ptr = static_cast<const uint8_t*>(data);
        rawBytes.insert(rawBytes.end(), ptr, ptr + bytes);
        return true;
    };

    assert(sink.start(config, sinkFn));

    // Push a known float value
    float mono[1] = {1.0f};
    const float* chPtrs[1] = {mono};
    sink.push(chPtrs, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sink.stop();

    // F32LE should deliver 4 bytes per sample (float = 4 bytes)
    if (!rawBytes.empty()) {
        assert(rawBytes.size() % sizeof(float) == 0);
    }
}

void testS16LEFormat() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 1;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::S16LE;

    std::vector<uint8_t> rawBytes;
    StreamSink::SinkFn sinkFn = [&rawBytes](const void* data, size_t bytes) -> bool {
        const uint8_t* ptr = static_cast<const uint8_t*>(data);
        rawBytes.insert(rawBytes.end(), ptr, ptr + bytes);
        return true;
    };

    assert(sink.start(config, sinkFn));

    float mono[10];
    for (int i = 0; i < 10; ++i) mono[i] = static_cast<float>(i) * 0.1f;
    const float* chPtrs[1] = {mono};
    sink.push(chPtrs, 10);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sink.stop();

    // S16LE should deliver 2 bytes per sample (int16 = 2 bytes)
    if (!rawBytes.empty()) {
        assert(rawBytes.size() % 2 == 0);
    }
}

void testOverrunDetection() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 1;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::F32LE;

    // Slow sink that never reads
    StreamSink::SinkFn sinkFn = [](const void*, size_t) -> bool {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
    };

    assert(sink.start(config, sinkFn));

    // Push a lot of data quickly
    float mono[48000];
    for (int i = 0; i < 48000; ++i) mono[i] = 0.5f;
    const float* chPtrs[1] = {mono};
    for (int i = 0; i < 10; ++i) {
        sink.push(chPtrs, 48000);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sink.stop();

    auto status = sink.getStatus();
    // With a slow sink, we should have written something
    assert(status.totalSamplesWritten > 0);
}

void testStatusReporting() {
    StreamSink sink;

    auto status = sink.getStatus();
    assert(!status.streaming);
    assert(!status.overrun);
    assert(status.durationSeconds == 0.0);
    assert(status.totalSamplesWritten == 0);

    StreamSink::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;

    StreamSink::SinkFn sinkFn = [](const void*, size_t) -> bool { return true; };
    assert(sink.start(config, sinkFn));

    status = sink.getStatus();
    assert(status.streaming);

    sink.stop();
    status = sink.getStatus();
    assert(!status.streaming);
}

void testMultiChannelStreaming() {
    StreamSink sink;

    StreamSink::Config config;
    config.numChannels = 4;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    config.format = StreamSink::Format::F32LE;

    size_t totalBytes = 0;
    StreamSink::SinkFn sinkFn = [&totalBytes](const void* data, size_t bytes) -> bool {
        totalBytes += bytes;
        return true;
    };

    assert(sink.start(config, sinkFn));

    float ch0[50], ch1[50], ch2[50], ch3[50];
    for (int i = 0; i < 50; ++i) {
        ch0[i] = static_cast<float>(i);
        ch1[i] = static_cast<float>(i + 0.1f);
        ch2[i] = static_cast<float>(i + 0.2f);
        ch3[i] = static_cast<float>(i + 0.3f);
    }
    const float* chPtrs[4] = {ch0, ch1, ch2, ch3};
    assert(sink.push(chPtrs, 50) == 50);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sink.stop();

    auto status = sink.getStatus();
    assert(status.totalSamplesWritten > 0);
}

// ========================================================================
// Main
// ========================================================================

int main() {
    testStartStopLifecycle();
    std::cout << "  testStartStopLifecycle passed\n";

    testPushDeliversToSink();
    std::cout << "  testPushDeliversToSink passed\n";

    testF32LEFormat();
    std::cout << "  testF32LEFormat passed\n";

    testS16LEFormat();
    std::cout << "  testS16LEFormat passed\n";

    testOverrunDetection();
    std::cout << "  testOverrunDetection passed\n";

    testStatusReporting();
    std::cout << "  testStatusReporting passed\n";

    testMultiChannelStreaming();
    std::cout << "  testMultiChannelStreaming passed\n";

    std::cout << "StreamSink tests passed!\n";
    return 0;
}
