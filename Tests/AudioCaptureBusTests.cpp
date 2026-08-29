#include "../Source/Audio/AudioCaptureBus.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

using namespace cendance;

// ========================================================================
// P1 Tests
// ========================================================================

void testPushPopRoundTrip() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    // Push 100 stereo frames
    const int numFrames = 100;
    float input[200]; // 100 frames * 2 channels
    for (int i = 0; i < numFrames; ++i) {
        input[i * 2] = static_cast<float>(i) / numFrames;     // left
        input[i * 2 + 1] = static_cast<float>(i + 0.5f) / numFrames; // right
    }

    const float* channels[2] = {&input[0], &input[1]};
    // Push as interleaved: channels[0] = left samples, channels[1] = right samples
    // But our input is interleaved, so we need to deinterleave first
    float left[100], right[100];
    for (int i = 0; i < numFrames; ++i) {
        left[i] = input[i * 2];
        right[i] = input[i * 2 + 1];
    }
    const float* chPtrs[2] = {left, right};
    int written = bus.push(chPtrs, numFrames);
    assert(written == numFrames);

    // Pop back
    float output[200];
    int read = bus.pop(output, numFrames);
    assert(read == numFrames);

    // Verify
    for (int i = 0; i < numFrames; ++i) {
        assert(std::abs(output[i * 2] - left[i]) < 0.0001f);
        assert(std::abs(output[i * 2 + 1] - right[i]) < 0.0001f);
    }
}

void testPushPopSingleChannel() {
    AudioCaptureBus::Config config;
    config.numChannels = 1;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    float mono[50];
    for (int i = 0; i < 50; ++i) mono[i] = static_cast<float>(i) * 0.01f;
    const float* chPtrs[1] = {mono};
    assert(bus.push(chPtrs, 50) == 50);

    float output[50];
    assert(bus.pop(output, 50) == 50);
    for (int i = 0; i < 50; ++i) {
        assert(std::abs(output[i] - mono[i]) < 0.0001f);
    }
}

void testOverrunDetection() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1; // 48000 frames capacity
    AudioCaptureBus bus(config);

    bus.setActive(true);

    // Fill the buffer to capacity
    int totalCapacity = 48000 * 2; // total samples (frames * channels)
    std::vector<float> bigInput(totalCapacity * 2); // 2x capacity to force overrun
    std::vector<float> left(totalCapacity), right(totalCapacity);
    for (int i = 0; i < totalCapacity; ++i) {
        left[i] = static_cast<float>(i) * 0.0001f;
        right[i] = static_cast<float>(i) * 0.0001f;
    }
    const float* chPtrs[2] = {left.data(), right.data()};

    // Push more than capacity
    int written = bus.push(chPtrs, totalCapacity * 2);
    // Should have written at most capacity/2 frames (since stereo)
    assert(written <= totalCapacity);

    auto state = bus.getState();
    assert(state.totalSamplesWritten > 0);
    // If we pushed more than capacity, some should have been dropped
    if (state.totalSamplesDropped > 0) {
        assert(state.overrun);
    }
}

void testActiveStateToggle() {
    AudioCaptureBus bus;
    assert(!bus.isActive());

    bus.setActive(true);
    assert(bus.isActive());

    bus.setActive(false);
    assert(!bus.isActive());
}

void testResetClearsBuffer() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    float left[100], right[100];
    for (int i = 0; i < 100; ++i) {
        left[i] = static_cast<float>(i);
        right[i] = static_cast<float>(i);
    }
    const float* chPtrs[2] = {left, right};
    bus.push(chPtrs, 100);

    bus.reset();

    auto state = bus.getState();
    assert(state.totalSamplesWritten == 0);
    assert(state.totalSamplesDropped == 0);
    assert(!state.overrun);

    // Should be able to push again
    assert(bus.push(chPtrs, 100) == 100);
}

void testPartialPop() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    float left[100], right[100];
    for (int i = 0; i < 100; ++i) {
        left[i] = static_cast<float>(i);
        right[i] = static_cast<float>(i);
    }
    const float* chPtrs[2] = {left, right};
    assert(bus.push(chPtrs, 100) == 100);

    // Pop only 50 frames
    float output[100]; // 50 frames * 2 channels
    int read = bus.pop(output, 50);
    assert(read == 50);

    // Verify first 50 frames
    for (int i = 0; i < 50; ++i) {
        assert(std::abs(output[i * 2] - left[i]) < 0.0001f);
        assert(std::abs(output[i * 2 + 1] - right[i]) < 0.0001f);
    }

    // Pop remaining 50
    read = bus.pop(output, 50);
    assert(read == 50);
    for (int i = 0; i < 50; ++i) {
        assert(std::abs(output[i * 2] - left[50 + i]) < 0.0001f);
        assert(std::abs(output[i * 2 + 1] - right[50 + i]) < 0.0001f);
    }
}

void testPartialPush() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    // Push a small number of frames
    float left[10], right[10];
    for (int i = 0; i < 10; ++i) {
        left[i] = static_cast<float>(i);
        right[i] = static_cast<float>(i + 0.5f);
    }
    const float* chPtrs[2] = {left, right};
    assert(bus.push(chPtrs, 10) == 10);

    float output[20];
    assert(bus.pop(output, 10) == 10);
    for (int i = 0; i < 10; ++i) {
        assert(std::abs(output[i * 2] - left[i]) < 0.0001f);
        assert(std::abs(output[i * 2 + 1] - right[i]) < 0.0001f);
    }
}

void testGetStateReturnsCounters() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    float left[50], right[50];
    for (int i = 0; i < 50; ++i) {
        left[i] = 1.0f;
        right[i] = 1.0f;
    }
    const float* chPtrs[2] = {left, right};
    bus.push(chPtrs, 50);

    auto state = bus.getState();
    assert(state.active);
    assert(state.sampleRate == 48000);
    assert(state.numChannels == 2);
    assert(state.totalSamplesWritten >= 50);
}

void testMultiChannelCapture() {
    AudioCaptureBus::Config config;
    config.numChannels = 4;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    float ch0[30], ch1[30], ch2[30], ch3[30];
    for (int i = 0; i < 30; ++i) {
        ch0[i] = static_cast<float>(i);
        ch1[i] = static_cast<float>(i + 0.1f);
        ch2[i] = static_cast<float>(i + 0.2f);
        ch3[i] = static_cast<float>(i + 0.3f);
    }
    const float* chPtrs[4] = {ch0, ch1, ch2, ch3};
    assert(bus.push(chPtrs, 30) == 30);

    float output[120]; // 30 frames * 4 channels
    assert(bus.pop(output, 30) == 30);

    for (int i = 0; i < 30; ++i) {
        assert(std::abs(output[i * 4 + 0] - ch0[i]) < 0.0001f);
        assert(std::abs(output[i * 4 + 1] - ch1[i]) < 0.0001f);
        assert(std::abs(output[i * 4 + 2] - ch2[i]) < 0.0001f);
        assert(std::abs(output[i * 4 + 3] - ch3[i]) < 0.0001f);
    }
}

void testConcurrentPushPop() {
    AudioCaptureBus::Config config;
    config.numChannels = 2;
    config.sampleRate = 48000;
    config.capacitySeconds = 1;
    AudioCaptureBus bus(config);

    bus.setActive(true);

    const int totalFrames = 2000;
    std::atomic<bool> producerDone{false};
    std::atomic<int> totalRead{0};

    std::thread producer([&]() {
        float left[128], right[128];
        int framesSent = 0;
        while (framesSent < totalFrames) {
            for (int i = 0; i < 128; ++i) {
                left[i] = static_cast<float>(framesSent + i);
                right[i] = static_cast<float>(framesSent + i + 0.5f);
            }
            const float* chPtrs[2] = {left, right};
            int written = bus.push(chPtrs, 128);
            if (written > 0) framesSent += written;
            else std::this_thread::yield();
        }
        producerDone = true;
    });

    std::thread consumer([&]() {
        float output[256];
        while (!producerDone.load()) {
            int read = bus.pop(output, 128);
            if (read > 0) totalRead += read;
            else std::this_thread::yield();
        }
        // Drain remaining
        float drain[256];
        while (true) {
            int read = bus.pop(drain, 128);
            if (read <= 0) break;
            totalRead += read;
        }
    });

    producer.join();
    consumer.join();

    // We should have read a significant portion of what was written
    assert(totalRead > 0);
}

// ========================================================================
// Main
// ========================================================================

int main() {
    testPushPopRoundTrip();
    std::cout << "  testPushPopRoundTrip passed\n";

    testPushPopSingleChannel();
    std::cout << "  testPushPopSingleChannel passed\n";

    testOverrunDetection();
    std::cout << "  testOverrunDetection passed\n";

    testActiveStateToggle();
    std::cout << "  testActiveStateToggle passed\n";

    testResetClearsBuffer();
    std::cout << "  testResetClearsBuffer passed\n";

    testPartialPop();
    std::cout << "  testPartialPop passed\n";

    testPartialPush();
    std::cout << "  testPartialPush passed\n";

    testGetStateReturnsCounters();
    std::cout << "  testGetStateReturnsCounters passed\n";

    testMultiChannelCapture();
    std::cout << "  testMultiChannelCapture passed\n";

    testConcurrentPushPop();
    std::cout << "  testConcurrentPushPop passed\n";

    std::cout << "AudioCaptureBus tests passed!\n";
    return 0;
}
