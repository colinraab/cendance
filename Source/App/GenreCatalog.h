#pragma once

#include "AlgorithmCatalog.h"
#include <array>
#include <cstdint>
#include <string_view>

namespace GenreCatalog {

static constexpr uint8_t kGenreCount = 8;

// Genre definitions: name, min BPM, max BPM
struct GenreDef {
    std::string_view name;
    float minBpm;
    float maxBpm;
};

static constexpr std::array<GenreDef, kGenreCount> kGenreDefinitions = {{
    {"House", 118.0f, 130.0f},
    {"UK Garage", 130.0f, 140.0f},
    {"DnB", 160.0f, 175.0f},
    {"Trap", 130.0f, 150.0f},
    {"Hip-Hop", 80.0f, 100.0f},
    {"Techno", 120.0f, 140.0f},
    {"Trance", 130.0f, 150.0f},
    {"Synth Pop", 100.0f, 130.0f},
}};

// Algorithm genre tags: bitmask per [track][algorithm].
// Bit 0=House(0x01), bit 1=UKGarage(0x02), bit 2=DnB(0x04), bit 3=Trap(0x08),
// bit 4=HipHop(0x10), bit 5=Techno(0x20), bit 6=Trance(0x40), bit 7=SynthPop(0x80).
static constexpr std::array<std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>,
                            AlgorithmCatalog::kTrackCount> kAlgorithmGenreMasks = {{
    // Track 0 - Drums
    {
        0x21, // FourOnFloor: House+Techno
        0x06, // Breakbeat: UKGarage+DnB
        0x20, // Euclidean: Techno
        0x04, // DnBBreaks: DnB
        0x01, // AfroClave: House
        0x01, // HouseShuffle: House
        0x08, // TrapHalfTime: Trap
        0x20, // GlitchPulse: Techno
        0x20, // TechnoRumble: Techno
        0x08, // JerseyClub: Trap
        0x20, // BrokenStepper: Techno
        0x20, // PolyrhythmToms: Techno
        0x20, // ElectroBreaks: Techno
        0x02, // GarageSwing: UKGarage
        0x01, // LatinPerc: House
        0x20, // MinimalClicks: Techno
        0x04, // DubSkank: DnB
        0x04, // Footwork160: DnB
        0x08, // Halfstep: Trap
        0x20, // Industrial: Techno
    },
    // Track 1 - Bass
    {
        0xA0, // WalkingBass: Techno+SynthPop
        0x20, // SyncBass: Techno
        0x08, // Sub808: Trap
        0x02, // UKGarage: UKGarage
        0x20, // Tumbao: Techno
        0x20, // DubPedal: Techno
        0x24, // ReesePulse: Techno+DnB
        0x20, // MotifBass: Techno
        0x08, // AcidTriplet: Trap
        0x40, // GlideCounter: Trance
        0x20, // PulseChop: Techno
        0x20, // OctaveBounce: Techno
        0x08, // ReggaetonSub: Trap
        0x20, // ElectroFunk: Techno
        0x60, // MinimalDrone: Techno+Trance
        0x20, // BrokenOctave: Techno
        0x20, // StepperDub: Techno
        0x80, // FunkPop: SynthPop
        0x24, // NeuroWobble: Techno+DnB
        0x01, // ClaveBass: House
    },
    // Track 2 - Chords
    {
        0x01, // BlockChords: House
        0x10, // SyncStabs: HipHop
        0x01, // HouseStabs: House
        0xC0, // AmbientPad: Trance+SynthPop
        0x80, // NeoSoul: SynthPop
        0x40, // TranceGate: Trance
        0x20, // QuartalComp: Techno
        0x20, // VoiceCloud: Techno
        0x01, // GospelLift: House
        0x20, // DetuneStack: Techno
        0x21, // BrokenStrum: House+Techno
        0x60, // PulseCluster: Techno+Trance
        0x21, // DubSkanks: House+Techno
        0x20, // MinimalPlucks: Techno
        0x10, // RNBKeys: HipHop
        0x40, // SuspendedPad: Trance
        0x20, // CinematicHits: Techno
        0x20, // FifthDrones: Techno
        0x01, // GarageOrgan: House
        0x20, // Polychord: Techno
    },
    // Track 3 - Lead
    {
        0x60, // Arpeggiator: Techno+Trance
        0x20, // Markov: Techno
        0x40, // TranceLead: Trance
        0x20, // RaveStabs: Techno
        0x80, // CallResp: SynthPop
        0x80, // CineSparse: SynthPop
        0x20, // EuclidLead: Techno
        0x80, // PhraseMut: SynthPop
        0x40, // GlideRun: Trance
        0x20, // MicroMotif: Techno
        0xA0, // WideInterval: Techno+SynthPop
        0x60, // TripletRush: Techno+Trance
        0x20, // PentatonicHook: Techno
        0x20, // AcidLine: Techno
        0x20, // DubEchoLead: Techno
        0x21, // GarageVox: Techno+House
        0x20, // MinimalPing: Techno
        0x60, // OrnamentRun: Techno+Trance
        0x20, // SyncopatedPluck: Techno
        0x60, // LydianFloat: Techno+Trance
    },
}};

inline constexpr std::string_view getGenreName(uint8_t genreId) {
    if (genreId < kGenreCount) return kGenreDefinitions[genreId].name;
    return "Unknown";
}

inline constexpr float getGenreMinBpm(uint8_t genreId) {
    if (genreId < kGenreCount) return kGenreDefinitions[genreId].minBpm;
    return 120.0f;
}

inline constexpr float getGenreMaxBpm(uint8_t genreId) {
    if (genreId < kGenreCount) return kGenreDefinitions[genreId].maxBpm;
    return 120.0f;
}

inline constexpr bool isValidGenreId(uint8_t genreId) {
    return genreId > 0 && genreId <= kGenreCount;
}

// Check if an algorithm has the given genre tag (genreId is 1-based)
inline constexpr bool algorithmHasGenre(uint8_t trackIndex, uint16_t algorithmId, uint8_t genreId) {
    if (trackIndex >= AlgorithmCatalog::kTrackCount ||
        algorithmId >= AlgorithmCatalog::kAlgorithmsPerTrack ||
        genreId == 0 || genreId >= 32) return false;
    return (kAlgorithmGenreMasks[trackIndex][algorithmId] >> (genreId - 1)) & 1;
}

// Runtime-aware genre helpers (non-constexpr, for use with custom algorithm IDs >= 2048).
// These resolve custom algorithm genre tags through the AlgorithmPresetRegistry.
class AlgorithmPresetRegistry;
bool algorithmHasGenreRuntime(uint8_t trackIndex, uint16_t algorithmId, uint8_t genreId);
uint32_t getAlgorithmGenreMaskRuntime(uint8_t trackIndex, uint16_t algorithmId);
std::string genreMaskToJson(uint32_t mask);

} // namespace GenreCatalog
