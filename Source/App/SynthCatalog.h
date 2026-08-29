#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "AlgorithmCatalog.h"
#include "DrumKitPresetCatalog.h"
#include "EffectPresetCatalog.h"

namespace SynthCatalog {

static constexpr uint8_t kTrackCount = 4;
static constexpr uint8_t kMaxPresetsPerTrack = 48;
static constexpr uint8_t kSoundEffectSlotCount = 3;
static constexpr uint8_t kMelodicSamplerPresetStart = 20;

static constexpr std::array<uint8_t, kTrackCount> kPresetCountByTrack = {
    0, // Drums use bundled kit presets from DrumKitPresetCatalog.
    38, // Bass presets: 20 procedural + 18 sampled.
    38, // Chord presets: 20 procedural + 18 sampled.
    38  // Lead presets: 20 procedural + 18 sampled.
};

static constexpr std::array<std::array<std::string_view, kMaxPresetsPerTrack>, kTrackCount> kPresetNames = {{
    std::array<std::string_view, kMaxPresetsPerTrack>{
        "Modular Punch", "Modular Voltage", "Modular Industrial", "Modular Punch Room",
        "Pop Tape", "Pop Quake", "Pop Crisp", "Pop Slam",
        "Semiacoustic Glitch", "Semiacoustic Neon", "Semiacoustic Warm Room", "Semiacoustic LoFi Tape",
        "Simple Lofi", "Simple Sunset", "Simple Dry Punch", "Simple Subtle Room",
        "Subterranean Impact", "Subterranean Bloom", "Subterranean Deep", "Subterranean Rumble",
        "Tight Razor", "Tight Crystal", "Tight Snap", "Tight Stutter",
        "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
        "-", "-", "-", "-", "-", "-", "-", "-"
    },
    std::array<std::string_view, kMaxPresetsPerTrack>{"Clean Sub", "Acid Thread", "Wide Reese", "Short Rubber", "FM Growl", "Folded Saw", "Hollow Tube", "Mono Wire", "Low Bell", "Elastic Bite", "Detune Wire", "Dust Drone", "Pulse FM", "Saw Stack", "Phase Grit", "Click Pluck", "Sub Chorus", "Square Bite", "Tri Drift", "Pipe Organ Bass", "808 Clean", "808 Pump", "808 Dirty", "Buzz Clean", "Buzz Ripper", "Buzz Wide", "FM Wow Clean", "FM Wow Motion", "FM Wow Crushed", "Hard Clean", "Hard Clip", "Hard Swarm", "Redux Clean", "Redux Dust", "Redux Broken", "Tonal Clean", "Tonal Round", "Tonal Drive", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-"},
    std::array<std::string_view, kMaxPresetsPerTrack>{"Warm Keys", "Short Stab", "Glass Pad", "Velvet Pad", "Organ Bed", "Metal Pluck", "Air Choir", "Detune Cloud", "PWM Keys", "FM Bloom", "Filter Sweep", "Dust Keys", "Tape Pad", "Grit Organ", "Bell Choir", "Pulse Pad", "Dusty Keys", "Wide PWM", "Late Bloom", "Reso Stack", "Analog Clean", "Analog Pump", "Analog Cloud", "Cobra Clean", "Cobra Bite", "Cobra Wide", "FM Piano Clean", "FM Piano Bright", "FM Piano Space", "Retro Tube Clean", "Retro Tube Warm", "Retro Tube Dust", "Sine Clean", "Sine Wash", "Sine Pulse", "Supersaw Clean", "Supersaw Wide", "Supersaw Lift", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-"},
    std::array<std::string_view, kMaxPresetsPerTrack>{"FM Needle", "PWM Cut", "Glide Tone", "Supersaw Lead", "Voxel Lead", "Acid Cry", "Folded Lead", "Whistle", "Reed", "Reso Lead", "Octave Lead", "Mutant Lead", "Razor Edge", "Neon Line", "Soft Sync", "Glass Cut", "Halo Lead", "FM Spark", "Phase Vox", "Wide Reed", "Bell Pluck Clean", "Bell Pluck Echo", "Bell Pluck Glass", "Bottle String Clean", "Bottle String Air", "Bottle String Warp", "FM Squares Clean", "FM Squares Bite", "FM Squares Wide", "Rave Stab Clean", "Rave Stab Throw", "Rave Stab Gate", "Saw Pluck Clean", "Saw Pluck Bright", "Saw Pluck Space", "Tube Delay Clean", "Tube Delay Dub", "Tube Delay Bloom", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-"}
}};

static constexpr std::array<std::array<float, kMaxPresetsPerTrack>, kTrackCount> kPresetLoudnessTrims = {{
    std::array<float, kMaxPresetsPerTrack>{
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    },
    std::array<float, kMaxPresetsPerTrack>{
        0.3548f, 0.5392f, 0.3548f, 0.3548f, 0.6717f, 0.3548f,
        0.3548f, 0.4862f, 0.3548f, 0.3548f, 0.3548f, 0.3548f,
        0.4677f, 0.3548f, 0.3548f, 1.4888f, 0.3567f, 0.3548f,
        0.3548f, 0.3548f, 1.7383f, 1.7580f, 1.9953f, 1.9953f,
        1.9953f, 1.9953f, 1.9953f, 1.9953f, 1.9953f, 1.9953f,
        1.9953f, 1.9953f, 1.9953f, 1.9953f, 1.9953f, 1.9953f,
        1.9953f, 1.9953f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    },
    std::array<float, kMaxPresetsPerTrack>{
        0.3548f, 0.8784f, 1.9953f, 0.3350f, 0.6422f, 1.3482f,
        0.3548f, 0.3548f, 1.8395f, 1.3958f, 0.6582f, 0.3548f,
        0.3548f, 0.4920f, 0.3548f, 1.8057f, 0.3548f, 1.9953f,
        1.1544f, 0.3548f, 0.9911f, 1.1274f, 1.8450f, 0.5210f,
        0.6529f, 1.0500f, 1.9953f, 1.9953f, 1.9953f, 0.6105f,
        0.6320f, 1.0144f, 0.9118f, 1.1201f, 1.9136f, 1.0089f,
        1.2469f, 1.9953f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    },
    std::array<float, kMaxPresetsPerTrack>{
        0.3548f, 0.3548f, 1.0792f, 0.3548f, 0.3548f, 0.3548f,
        0.3548f, 1.7891f, 0.3548f, 0.3548f, 0.6112f, 0.3548f,
        0.3548f, 0.3548f, 0.3548f, 0.3548f, 1.0196f, 0.3548f,
        0.3548f, 0.3548f, 1.9953f, 1.9953f, 1.9953f, 1.9218f,
        1.9953f, 1.9953f, 0.5253f, 1.9953f, 1.0748f, 0.9808f,
        1.9953f, 1.9697f, 1.0958f, 1.9953f, 1.9953f, 1.0424f,
        1.9953f, 1.5613f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    }
}};

static constexpr float kChordTrackLoudnessTrim = 0.5012f; // -6 dB

// Sound genre tags use the same bit layout as GenreCatalog:
// bit 0=House, bit 1=UK Garage, bit 2=DnB, bit 3=Trap,
// bit 4=Hip-Hop, bit 5=Techno, bit 6=Trance, bit 7=Synth Pop.
static constexpr std::array<std::array<uint32_t, kMaxPresetsPerTrack>, kTrackCount> kPresetGenreMasks = {{
    std::array<uint32_t, kMaxPresetsPerTrack>{
        0x61, 0xA1, 0x20, 0x01, 0x90, 0x08, 0x80, 0x81,
        0x24, 0x22, 0x91, 0x14, 0x18, 0x80, 0x11, 0x10,
        0x28, 0x04, 0x04, 0x0C, 0x20, 0x60, 0x20, 0x24,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    std::array<uint32_t, kMaxPresetsPerTrack>{
        0x31, 0x60, 0x24, 0x03, 0x24, 0x60, 0x50, 0x20,
        0x90, 0x81, 0xA0, 0x14, 0x61, 0x21, 0x20, 0x12,
        0x81, 0x88, 0xC0, 0x80, 0x18, 0x08, 0x0C, 0x12,
        0x24, 0x64, 0xA0, 0x60, 0x24, 0x21, 0x20, 0x60,
        0x14, 0x10, 0x2C, 0x91, 0x81, 0x29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    std::array<uint32_t, kMaxPresetsPerTrack>{
        0x91, 0x2B, 0xC0, 0xC0, 0x81, 0x20, 0xD0, 0xE0,
        0x81, 0x90, 0x60, 0x14, 0xC0, 0x21, 0x90, 0x60,
        0x11, 0xA1, 0xC0, 0x60, 0x91, 0x21, 0xE0, 0x22,
        0x20, 0x62, 0x90, 0x80, 0xC0, 0x91, 0x81, 0x14,
        0x50, 0xC0, 0x60, 0xC1, 0xC0, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    std::array<uint32_t, kMaxPresetsPerTrack>{
        0x60, 0xA1, 0x61, 0xC0, 0x80, 0x28, 0x60, 0x90,
        0x90, 0x60, 0xA1, 0x24, 0x20, 0x22, 0xC0, 0xC0,
        0xC0, 0x90, 0xA0, 0x80, 0x90, 0xC0, 0xC0, 0x90,
        0x80, 0xC0, 0x90, 0xA0, 0xE0, 0x21, 0x60, 0x60,
        0x91, 0x81, 0xC0, 0x24, 0x22, 0x64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
}};

inline constexpr uint16_t fx(uint16_t displayId) {
    return EffectPresetCatalog::displayIdToPresetId(displayId);
}

inline constexpr std::array<std::array<std::array<uint16_t, kSoundEffectSlotCount>, kMaxPresetsPerTrack>, kTrackCount> kPresetEffectSlots = {{
    std::array<std::array<uint16_t, kSoundEffectSlotCount>, kMaxPresetsPerTrack>{{
        std::array<uint16_t, kSoundEffectSlotCount>{fx(32), 0, 0},            // Punch
        std::array<uint16_t, kSoundEffectSlotCount>{fx(15), fx(13), 0},       // Tape
        std::array<uint16_t, kSoundEffectSlotCount>{fx(88), fx(71), fx(107)}, // Glitch
        std::array<uint16_t, kSoundEffectSlotCount>{fx(16), 0, 0},            // Lofi
        std::array<uint16_t, kSoundEffectSlotCount>{fx(33), fx(31), 0},       // Impact
        std::array<uint16_t, kSoundEffectSlotCount>{fx(34), fx(36), fx(79)},  // Razor
        std::array<uint16_t, kSoundEffectSlotCount>{fx(43), 0, 0},            // Voltage
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(26), 0},      // Quake
        std::array<uint16_t, kSoundEffectSlotCount>{fx(49), fx(63), fx(19)},  // Neon
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},                 // Sunset
        std::array<uint16_t, kSoundEffectSlotCount>{fx(107), fx(11), 0},     // Bloom
        std::array<uint16_t, kSoundEffectSlotCount>{fx(114), fx(65), fx(13)}, // Crystal
        std::array<uint16_t, kSoundEffectSlotCount>{fx(75), 0, 0},            // Dust
        std::array<uint16_t, kSoundEffectSlotCount>{fx(84), fx(30), fx(20)},  // Festival
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},                 // Root
        std::array<uint16_t, kSoundEffectSlotCount>{fx(87), fx(78), 0},       // Oddment
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},                 // Punch Dry
        std::array<uint16_t, kSoundEffectSlotCount>{fx(33), fx(26), fx(30)},  // Punch Slam
        std::array<uint16_t, kSoundEffectSlotCount>{fx(18), fx(13), 0},       // Tape Dub
        std::array<uint16_t, kSoundEffectSlotCount>{fx(15), fx(75), fx(79)},  // Tape Crushed
        std::array<uint16_t, kSoundEffectSlotCount>{fx(88), fx(103), fx(71)}, // Glitch Scatter
        std::array<uint16_t, kSoundEffectSlotCount>{fx(34), fx(84), 0},       // Glitch Tight
        std::array<uint16_t, kSoundEffectSlotCount>{fx(75), fx(122), 0},      // Lofi Dust
        std::array<uint16_t, kSoundEffectSlotCount>{fx(14), 0, 0},            // Lofi Room
        std::array<uint16_t, kSoundEffectSlotCount>{fx(84), fx(33), 0},       // Impact Pump
        std::array<uint16_t, kSoundEffectSlotCount>{fx(82), fx(36), fx(84)},  // Razor Gate
        std::array<uint16_t, kSoundEffectSlotCount>{fx(70), fx(55), 0},       // Voltage Flange
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(84), fx(20)}, // Quake Rumble
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), fx(49), 0},       // Neon Wide
        std::array<uint16_t, kSoundEffectSlotCount>{fx(107), fx(21), fx(13)}, // Bloom Cloud
        std::array<uint16_t, kSoundEffectSlotCount>{fx(114), fx(103), 0},     // Crystal Freeze
        std::array<uint16_t, kSoundEffectSlotCount>{fx(87), fx(78), fx(67)},  // Oddment Broken
    }},
    std::array<std::array<uint16_t, kSoundEffectSlotCount>, kMaxPresetsPerTrack>{{
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(39), fx(63), fx(84)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(23), fx(59), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(34), fx(18), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(52), fx(59), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(39), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(43), fx(14), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(24), fx(63), fx(27)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(113), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(63), fx(43), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(97), fx(59), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(75), fx(26), fx(14)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(91), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), fx(24), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(63), fx(35), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(32), fx(122), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(59), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(97), fx(47), fx(13)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(43), fx(116), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(18), fx(75), fx(115)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(91), fx(41), fx(88)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(95), fx(27), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(54), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(32), fx(35), fx(115)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), fx(44), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(89), fx(122), fx(47)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(25), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(41), fx(63), fx(20)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), 0, 0},
    }},
    std::array<std::array<uint16_t, kSoundEffectSlotCount>, kMaxPresetsPerTrack>{{
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(35), fx(83), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(10), fx(60)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(14), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(123), fx(63), fx(27)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(39), fx(59), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(102), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(61), fx(97), fx(13)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(59), fx(79), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(99), fx(10), fx(63)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(8), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(75), fx(15), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(16), fx(59), fx(14)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(53), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(99), fx(10), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(82), fx(60), fx(18)},
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(61), fx(20), fx(27)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(11), fx(97), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(43), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(59), fx(10), fx(115)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(113), fx(107), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(18), fx(83), fx(16)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(107), 0, 0},            // Cobra Clean
        std::array<uint16_t, kSoundEffectSlotCount>{fx(126), fx(64), fx(18)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(53), fx(27), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(102), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(113), fx(99), fx(13)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(65), fx(18), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(35), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(16), fx(102), fx(14)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(41), fx(79), 0},
    }},
    std::array<std::array<uint16_t, kSoundEffectSlotCount>, kMaxPresetsPerTrack>{{
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(59), fx(64), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(97), fx(18), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), fx(24), fx(31)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(123), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(39), fx(63), fx(83)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(39), fx(52), fx(18)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(122), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(123), fx(10), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(18), fx(59)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(98), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(52), fx(63), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(36), fx(63), fx(30)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(66), fx(20), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(35), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(111), fx(10), fx(59)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(102), fx(12), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(123), fx(63), fx(10)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(60), fx(44), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(56), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(75), fx(107), fx(18)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(102), fx(123), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(17), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(35), fx(99), fx(63)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(99), fx(111), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(107), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(40), fx(56), fx(122)},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(97), fx(65), 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(126), 0, 0},
        std::array<uint16_t, kSoundEffectSlotCount>{fx(89), fx(79), fx(18)},  // Rave Stab Throw
        std::array<uint16_t, kSoundEffectSlotCount>{fx(114), fx(99), fx(20)},
    }},
}};

static constexpr std::array<std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>, kTrackCount> kDefaultPresetByAlgorithm = {{
    std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>{0, 1, 6, 8, 4, 5, 9, 10, 2, 7, 11, 3, 6, 0, 9, 5, 1, 10, 8, 4},
    std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>{0, 1, 2, 4, 5, 7, 10, 12, 13, 9, 14, 8, 16, 3, 11, 15, 6, 17, 18, 19},
    std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>{0, 1, 2, 3, 7, 5, 10, 12, 13, 9, 14, 8, 16, 4, 11, 15, 6, 17, 18, 19},
    std::array<uint8_t, AlgorithmCatalog::kAlgorithmsPerTrack>{0, 1, 3, 4, 7, 5, 10, 12, 13, 9, 14, 8, 16, 2, 11, 15, 6, 17, 18, 19}
}};

inline constexpr uint16_t getPresetCountForTrack(uint8_t trackIndex) {
    if (trackIndex == 0) {
        return DrumKitPresetCatalog::getPresetCount();
    }

    return trackIndex < kTrackCount ? kPresetCountByTrack[trackIndex] : 0;
}

inline constexpr uint16_t getProceduralPresetCountForTrack(uint8_t trackIndex) {
    if (trackIndex == 0) {
        return getPresetCountForTrack(trackIndex);
    }

    return trackIndex < kTrackCount ? kMelodicSamplerPresetStart : 0;
}

inline constexpr uint16_t getAutoSelectablePresetCountForTrack(uint8_t trackIndex) {
    return getProceduralPresetCountForTrack(trackIndex);
}

inline constexpr bool isMelodicSamplerPreset(uint8_t trackIndex, uint8_t presetId) {
    return trackIndex > 0
        && trackIndex < kTrackCount
        && presetId >= kMelodicSamplerPresetStart
        && presetId < getPresetCountForTrack(trackIndex);
}

inline constexpr uint8_t getMaxPresetIdForTrack(uint8_t trackIndex) {
    const uint16_t count = getPresetCountForTrack(trackIndex);
    return count > 0 ? static_cast<uint8_t>(count - 1) : 0;
}

inline constexpr uint32_t getSoundGenreMask(uint8_t trackIndex, uint8_t presetId) {
    if (trackIndex >= kTrackCount || presetId >= getPresetCountForTrack(trackIndex)) {
        return 0;
    }

    return kPresetGenreMasks[trackIndex][presetId];
}

inline constexpr bool soundHasGenre(uint8_t trackIndex, uint8_t presetId, uint8_t genreId) {
    if (genreId == 0 || genreId >= 32) {
        return false;
    }

    return (getSoundGenreMask(trackIndex, presetId) >> (genreId - 1)) & 1u;
}

inline constexpr uint8_t getDefaultPresetForAlgorithm(uint8_t trackIndex, uint8_t algorithmId) {
    if (trackIndex >= kTrackCount) {
        return 0;
    }

    const uint16_t algorithmCount = AlgorithmCatalog::getAlgorithmCountForTrack(trackIndex);
    if (algorithmCount == 0) {
        return 0;
    }

    const uint16_t safeAlgorithm = static_cast<uint16_t>(algorithmId % algorithmCount);
    const uint8_t preset = kDefaultPresetByAlgorithm[trackIndex][safeAlgorithm];
    const uint8_t maxPreset = getMaxPresetIdForTrack(trackIndex);
    return preset > maxPreset ? maxPreset : preset;
}

inline constexpr std::string_view getPresetName(uint8_t trackIndex, uint8_t presetId) {
    if (trackIndex == 0) {
        return DrumKitPresetCatalog::isValidPresetId(presetId)
            ? DrumKitPresetCatalog::getPresetName(presetId)
            : "Unknown";
    }

    if (trackIndex >= kTrackCount || presetId >= kMaxPresetsPerTrack) {
        return "Unknown";
    }

    const uint8_t count = getPresetCountForTrack(trackIndex);
    if (presetId >= count) {
        return "Unknown";
    }

    return kPresetNames[trackIndex][presetId];
}

inline constexpr float getPresetLoudnessTrim(uint8_t trackIndex, uint8_t presetId) {
    if (trackIndex == 0) {
        return DrumKitPresetCatalog::getPresetLoudnessTrim(presetId);
    }

    if (trackIndex >= kTrackCount || presetId >= getPresetCountForTrack(trackIndex)) {
        return 1.0f;
    }

    const float trackTrim = trackIndex == 2 ? kChordTrackLoudnessTrim : 1.0f;
    return kPresetLoudnessTrims[trackIndex][presetId] * trackTrim;
}

inline constexpr bool isValidDisplayIdForTrack(uint8_t trackIndex, uint16_t displayId) {
    const uint16_t count = getPresetCountForTrack(trackIndex);
    return displayId >= 1 && displayId <= count;
}

inline constexpr uint16_t displayIdToPresetId(uint16_t displayId) {
    return displayId > 0 ? static_cast<uint16_t>(displayId - 1) : 0;
}

inline constexpr std::string_view getPresetNameByDisplayId(uint8_t trackIndex, uint16_t displayId) {
    if (!isValidDisplayIdForTrack(trackIndex, displayId)) {
        return "Invalid";
    }
    return getPresetName(trackIndex, static_cast<uint8_t>(displayIdToPresetId(displayId)));
}

inline constexpr std::array<uint16_t, kSoundEffectSlotCount> getPresetEffectSlots(uint8_t trackIndex, uint8_t presetId) {
    if (trackIndex >= kTrackCount || presetId >= kMaxPresetsPerTrack) {
        return std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0};
    }

    const uint16_t count = getPresetCountForTrack(trackIndex);
    if (presetId >= count) {
        return std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0};
    }

    if (trackIndex == 0) {
        if (presetId < DrumKitPresetCatalog::getPresetCount()) {
            return DrumKitPresetCatalog::getPresetEffectSlots(presetId);
        }
        return std::array<uint16_t, kSoundEffectSlotCount>{0, 0, 0};
    }

    if (isMelodicSamplerPreset(trackIndex, presetId)) {
        const uint8_t variant = static_cast<uint8_t>((presetId - kMelodicSamplerPresetStart) % 3);

        if (trackIndex == 1) {
            switch (variant) {
                case 0: return {fx(26), fx(82), 0};       // clean bass
                case 1: return {fx(84), fx(23), fx(29)};  // pumped bass
                case 2: return {fx(35), fx(16), fx(41)};  // dirty bass
                default: break;
            }
        }

        if (trackIndex == 2) {
            switch (variant) {
                case 0: return {fx(10), fx(59), 0};       // clean chords
                case 1: return {fx(83), fx(18), fx(13)};  // rhythmic chords
                case 2: return {fx(11), fx(61), fx(20)};  // cloud chords
                default: break;
            }
        }

        if (trackIndex == 3) {
            switch (variant) {
                case 0: return {fx(23), fx(29), 0};       // clean lead
                case 1: return {fx(19), fx(48), fx(95)};  // echo lead
                case 2: return {fx(56), fx(115), fx(11)}; // glass lead
                default: break;
            }
        }
    }

    return kPresetEffectSlots[trackIndex][presetId];
}

inline constexpr uint16_t getPresetEffectSlot(uint8_t trackIndex, uint8_t presetId, uint8_t slotIndex) {
    if (slotIndex >= kSoundEffectSlotCount) {
        return 0;
    }

    return getPresetEffectSlots(trackIndex, presetId)[slotIndex];
}

} // namespace SynthCatalog
