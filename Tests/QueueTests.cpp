#include "../Source/App/CommandQueue.h"
#include "../Source/App/MeterQueue.h"
#include "../Source/App/AppState.h"

#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

void testCommandQueueCapacityAndOrderSingleThread() {
    CommandQueue q;

    // Ring buffer keeps one slot empty to distinguish full vs empty.
    for (size_t i = 0; i < CommandQueue::Capacity - 1; ++i) {
        Command cmd{Command::Type::SetDensity, 0, 0, static_cast<float>(i)};
        assert(q.push(cmd));
    }

    Command overflow{Command::Type::SetDensity, 0, 0, 999.0f};
    assert(!q.push(overflow));

    for (size_t i = 0; i < CommandQueue::Capacity - 1; ++i) {
        Command popped{};
        assert(q.pop(popped));
        assert(popped.type == Command::Type::SetDensity);
        assert(popped.value == static_cast<float>(i));
    }

    Command empty{};
    assert(!q.pop(empty));

    std::cout << "CommandQueue capacity/overflow test passed!\n";
}

void testMeterQueuePopLatestAndCapacitySingleThread() {
    MeterQueue q;

    for (int i = 0; i < 5; ++i) {
        MeterData data;
        data.masterLevel = static_cast<float>(i);
        assert(q.push(data));
    }

    MeterData latest;
    assert(q.popLatest(latest));
    assert(latest.masterLevel == 4.0f);

    MeterData empty;
    assert(!q.popLatest(empty));

    for (size_t i = 0; i < MeterQueue::Capacity - 1; ++i) {
        MeterData data;
        data.masterLevel = static_cast<float>(i);
        assert(q.push(data));
    }

    MeterData overflow;
    assert(!q.push(overflow));

    std::cout << "MeterQueue latest/capacity test passed!\n";
}

void testMeterQueueCarriesAnalyzerFields() {
    MeterQueue q;
    MeterData data;
    data.analyzerValid = true;
    data.analyzerFrame = 42;
    data.activeNotes[0][0] = 12345ULL;
    data.spectrumBins[0] = 0.1f;
    data.spectrumBins[kSpectrumBinCount - 1] = 0.9f;
    data.performanceProfileValid = true;
    data.profileWindowCallbacks = 128;
    data.profileBufferDurationMs = 5.33f;
    data.callbackMsAvg = 1.20f;
    data.callbackMsPeak = 2.10f;
    data.callbackUtilizationAvg = 22.50f;
    data.callbackUtilizationPeak = 39.40f;
    data.commandsMsAvg = 0.05f;
    data.generationMsAvg = 0.70f;
    data.trackFxMsAvg = 0.18f;
    data.masterFxMsAvg = 0.11f;
    data.meteringMsAvg = 0.16f;

    assert(q.push(data));

    MeterData latest;
    assert(q.popLatest(latest));
    assert(latest.analyzerValid);
    assert(latest.analyzerFrame == 42);
    assert(latest.activeNotes[0][0] == 12345ULL);
    assert(latest.spectrumBins[0] == 0.1f);
    assert(latest.spectrumBins[kSpectrumBinCount - 1] == 0.9f);
    assert(latest.performanceProfileValid);
    assert(latest.profileWindowCallbacks == 128);
    assert(latest.profileBufferDurationMs == 5.33f);
    assert(latest.callbackMsAvg == 1.20f);
    assert(latest.callbackMsPeak == 2.10f);
    assert(latest.callbackUtilizationAvg == 22.50f);
    assert(latest.callbackUtilizationPeak == 39.40f);
    assert(latest.commandsMsAvg == 0.05f);
    assert(latest.generationMsAvg == 0.70f);
    assert(latest.trackFxMsAvg == 0.18f);
    assert(latest.masterFxMsAvg == 0.11f);
    assert(latest.meteringMsAvg == 0.16f);

    std::cout << "MeterQueue analyzer payload test passed!\n";
}

void testCommandQueue() {
    CommandQueue q;
    std::atomic<bool> producerDone{false};
    std::vector<float> receivedValues;

    std::thread producer([&]() {
        for (int i = 0; i < 1000; ++i) {
            Command cmd{Command::Type::SetDensity, 0, 0, static_cast<float>(i)};
            while (!q.push(cmd)) {
                std::this_thread::yield();
            }
        }
        producerDone = true;
    });

    std::thread consumer([&]() {
        while (true) {
            Command cmd;
            if (q.pop(cmd)) {
                receivedValues.push_back(cmd.value);
            } else if (producerDone) {
                break; // queue empty and producer finished
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    assert(receivedValues.size() == 1000);
    for (size_t i = 0; i < receivedValues.size(); ++i) {
        assert(receivedValues[i] == static_cast<float>(i));
    }
    std::cout << "CommandQueue test passed! 1000 commands received in order.\n";
}

void testMeterQueue() {
    MeterQueue q;
    std::atomic<bool> producerDone{false};
    std::atomic<int> readCount{0};

    std::thread producer([&]() {
        for (int i = 0; i < 100; ++i) {
            MeterData d;
            d.masterLevel = static_cast<float>(i);
            while (!q.push(d)) {
                std::this_thread::yield(); // spin if full
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Audio blocks (~2ms)
        }
        producerDone = true;
    });

    std::thread consumer([&]() {
        while (!producerDone) {
            MeterData d;
            if (q.popLatest(d)) {
                readCount++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); // UI framerate ~30fps
        }
    });

    producer.join();
    consumer.join();

    std::cout << "MeterQueue test passed! Read " << readCount << " batched updates out of 100 pushed.\n";
}

int main() {
    testCommandQueueCapacityAndOrderSingleThread();
    testMeterQueuePopLatestAndCapacitySingleThread();
    testMeterQueueCarriesAnalyzerFields();
    testCommandQueue();
    testMeterQueue();
    std::cout << "All tests passed!\n";
    return 0;
}
