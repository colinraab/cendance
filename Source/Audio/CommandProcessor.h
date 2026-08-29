#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "../App/AppState.h"
#include "../App/CommandQueue.h"
#include "../App/AlgorithmPresetRegistry.h"
#include "../App/DrumSampleLibrary.h"
#include "Generators/GenerativeAlgorithm.h"
#include "Generators/CustomAlgorithmInstance.h"

class EffectProcessor;
class DrumEngine;
class Transport;

class CommandProcessor final
{
public:
    static constexpr uint8_t TrackCount = AppState::kTrackCount;

    struct Delegate
    {
        virtual ~Delegate() = default;
        virtual uint8_t getMaxSynthPresetIdForTrack(uint8_t trackIndex) const = 0;
        virtual void applySoundPreset(uint8_t trackIndex, uint8_t presetId, bool manualOverride) = 0;
        virtual void resetTransportAndArrangement() = 0;
        virtual void setArrangementAnchorInitialized(bool initialized) = 0;
        virtual void resetAlgorithm(uint8_t trackIndex, uint16_t algorithmId) = 0;
        virtual void setDrumSampleForSlot(uint8_t slotIndex, const DrumSampleData* sampleData) = 0;
        virtual void setDrumSampleSlotVolume(uint8_t slotIndex, float value) = 0;
        virtual void setDrumSampleSlotTuneSemitones(uint8_t slotIndex, float value) = 0;
        virtual void setDrumSampleSlotStartOffset(uint8_t slotIndex, float value) = 0;
        virtual void setDrumSampleSlotDecay(uint8_t slotIndex, float value) = 0;
        virtual void setDrumSampleSlotVelocitySensitivity(uint8_t slotIndex, float value) = 0;
    };

    CommandProcessor(AppState& appState,
                     CommandQueue& commandQueue,
                     EffectProcessor& effectProcessor,
                     AlgorithmPresetRegistry* algorithmRegistry,
                     DrumSampleLibrary* drumSampleLibrary,
                     Delegate& delegate);

    void process();
    void rebuildCustomAlgorithmInstances();

    uint16_t getMaxAlgorithmIdForTrack(uint8_t trackIndex) const;
    GenerativeAlgorithm* getTrackAlgorithm(uint8_t trackIndex, uint16_t algorithmId,
                                           const std::array<std::array<GenerativeAlgorithm*, AlgorithmCatalog::kAlgorithmsPerTrack>, TrackCount>& builtinTrackAlgorithms) const;

private:
    AppState& appState;
    CommandQueue& commandQueue;
    EffectProcessor& effectProcessor;
    AlgorithmPresetRegistry* algorithmRegistry = nullptr;
    DrumSampleLibrary* drumSampleLibrary = nullptr;
    Delegate& delegate;

    std::array<uint16_t, TrackCount> customAlgorithmCounts{};
    std::array<std::vector<std::unique_ptr<CustomAlgorithmInstance>>, TrackCount> customTrackAlgorithms;
};
