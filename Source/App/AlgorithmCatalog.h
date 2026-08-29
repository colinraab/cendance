#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace AlgorithmCatalog {

static constexpr uint8_t kTrackCount = 4;
static constexpr uint8_t kAlgorithmsPerTrack = 20;

static constexpr std::array<std::array<std::string_view, kAlgorithmsPerTrack>, kTrackCount> kAlgorithmNames = {{
    std::array<std::string_view, kAlgorithmsPerTrack>{"FourOnFloor", "Breakbeat", "Euclidean", "DnBBreaks", "AfroClave", "HouseShuffle", "TrapHalfTime", "GlitchPulse", "TechnoRumble", "JerseyClub", "BrokenStepper", "PolyrhythmToms", "ElectroBreaks", "GarageSwing", "LatinPerc", "MinimalClicks", "DubSkank", "Footwork160", "Halfstep", "Industrial"},
    std::array<std::string_view, kAlgorithmsPerTrack>{"WalkingBass", "SyncBass", "Sub808", "UKGarage", "Tumbao", "DubPedal", "ReesePulse", "MotifBass", "AcidTriplet", "GlideCounter", "PulseChop", "OctaveBounce", "ReggaetonSub", "ElectroFunk", "MinimalDrone", "BrokenOctave", "StepperDub", "FunkPop", "NeuroWobble", "ClaveBass"},
    std::array<std::string_view, kAlgorithmsPerTrack>{"BlockChords", "SyncStabs", "HouseStabs", "AmbientPad", "NeoSoul", "TranceGate", "QuartalComp", "VoiceCloud", "GospelLift", "DetuneStack", "BrokenStrum", "PulseCluster", "DubSkanks", "MinimalPlucks", "RNBKeys", "SuspendedPad", "CinematicHits", "FifthDrones", "GarageOrgan", "Polychord"},
    std::array<std::string_view, kAlgorithmsPerTrack>{"Arpeggiator", "Markov", "TranceLead", "RaveStabs", "CallResp", "CineSparse", "EuclidLead", "PhraseMut", "GlideRun", "MicroMotif", "WideInterval", "TripletRush", "PentatonicHook", "AcidLine", "DubEchoLead", "GarageVox", "MinimalPing", "OrnamentRun", "SyncopatedPluck", "LydianFloat"}
}};

inline constexpr uint16_t getAlgorithmCountForTrack(uint8_t trackIndex) {
    return trackIndex < kTrackCount ? kAlgorithmsPerTrack : 0;
}

inline constexpr std::string_view getAlgorithmName(uint8_t trackIndex, uint16_t algorithmId) {
    if (trackIndex >= kTrackCount || algorithmId >= kAlgorithmsPerTrack) {
        return "Unknown";
    }
    return kAlgorithmNames[trackIndex][algorithmId];
}

inline constexpr bool isValidDisplayIdForTrack(uint8_t trackIndex, uint16_t displayId) {
    const uint16_t count = getAlgorithmCountForTrack(trackIndex);
    return displayId >= 1 && displayId <= count;
}

inline constexpr uint16_t displayIdToAlgorithmId(uint16_t displayId) {
    return displayId > 0 ? static_cast<uint16_t>(displayId - 1) : 0;
}

inline constexpr std::string_view getAlgorithmNameByDisplayId(uint8_t trackIndex, uint16_t displayId) {
    if (!isValidDisplayIdForTrack(trackIndex, displayId)) {
        return "Invalid";
    }
    return getAlgorithmName(trackIndex, displayIdToAlgorithmId(displayId));
}

} // namespace AlgorithmCatalog
