#pragma once

#include <atomic>
#include <array>
#include <cstdint>

inline constexpr size_t kOscilloscopeSampleCount = 96;
inline constexpr size_t kSpectrumBinCount = 32;

struct MeterData {
    float trackLevels[4] = {0.0f};      // RMS per track
    float masterLevel = 0.0f;           // Master RMS
    uint32_t beatPosition = 0;          // Current beat in bar (0-3)
    uint16_t barNumber = 0;             // Current bar
    uint16_t activeAlgorithm[4] = {0};   // Currently active algorithm per track
    bool isPlaying = false;
    bool analyzerValid = false;
    uint32_t analyzerFrame = 0;
    uint64_t activeNotes[4][2] = {{0}};
    std::array<float, kSpectrumBinCount> spectrumBins{};

    bool performanceProfileValid = false;
    uint32_t profileWindowCallbacks = 0;
    float profileBufferDurationMs = 0.0f;
    float callbackMsAvg = 0.0f;
    float callbackMsPeak = 0.0f;
    float callbackUtilizationAvg = 0.0f;
    float callbackUtilizationPeak = 0.0f;
    float commandsMsAvg = 0.0f;
    float generationMsAvg = 0.0f;
    float trackFxMsAvg = 0.0f;
    float masterFxMsAvg = 0.0f;
    float meteringMsAvg = 0.0f;
};

class MeterQueue {
public:
    static constexpr size_t Capacity = 64; // Smaller capacity for UI updates since we poll at ~30fps

    bool push(const MeterData& data) {
        auto currentWrite = writePos.load(std::memory_order_relaxed);
        auto nextWrite = increment(currentWrite);
        
        if (nextWrite != readPos.load(std::memory_order_acquire)) {
            buffer[currentWrite] = data;
            writePos.store(nextWrite, std::memory_order_release);
            return true;
        }
        return false; // Full
    }

    // Try reading all available, keeping only the most recent
    bool popLatest(MeterData& data) {
        bool found = false;
        MeterData temp;
        uint64_t accumNotes[4][2] = {{0}};
        while (pop(temp)) {
            for (int t = 0; t < 4; ++t) {
                accumNotes[t][0] |= temp.activeNotes[t][0];
                accumNotes[t][1] |= temp.activeNotes[t][1];
            }
            data = temp;
            found = true;
        }
        if (found) {
            for (int t = 0; t < 4; ++t) {
                data.activeNotes[t][0] |= accumNotes[t][0];
                data.activeNotes[t][1] |= accumNotes[t][1];
            }
        }
        return found;
    }

private:
    bool pop(MeterData& data) {
        auto currentRead = readPos.load(std::memory_order_relaxed);
        
        if (currentRead == writePos.load(std::memory_order_acquire)) {
            return false; // Empty
        }
        
        data = buffer[currentRead];
        readPos.store(increment(currentRead), std::memory_order_release);
        return true;
    }

    static constexpr size_t increment(size_t pos) {
        return (pos + 1) % Capacity;
    }

    std::array<MeterData, Capacity> buffer;
    alignas(64) std::atomic<size_t> writePos{0};
    alignas(64) std::atomic<size_t> readPos{0};
};
