#pragma once

#include "DrumSampleCatalog.h"
#include "EffectPresetCatalog.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace DrumKitPresetCatalog {

constexpr uint16_t kEmbeddedSampleIdStart = 100;
constexpr uint16_t kEmbeddedSampleIdEnd = 199;

struct EmbeddedSampleDefinition {
    uint16_t sampleId;
    std::string_view resourceName;
    std::string_view displayName;
};

inline constexpr std::array<EmbeddedSampleDefinition, 24> kEmbeddedSamples{{
    {100, "kit_modular_kick_ogg", "Modular Kick"},
    {101, "kit_modular_snare_ogg", "Modular Snare"},
    {102, "kit_modular_chat_ogg", "Modular Closed Hat"},
    {103, "kit_modular_ohat_ogg", "Modular Open Hat"},
    {104, "kit_pop_kick_ogg", "Pop Kick"},
    {105, "kit_pop_snare_ogg", "Pop Snare"},
    {106, "kit_pop_chat_ogg", "Pop Closed Hat"},
    {107, "kit_pop_ohat_ogg", "Pop Open Hat"},
    {108, "kit_semiacoustic_kick_ogg", "Semiacoustic Kick"},
    {109, "kit_semiacoustic_snare_ogg", "Semiacoustic Snare"},
    {110, "kit_semiacoustic_chat_ogg", "Semiacoustic Closed Hat"},
    {111, "kit_semiacoustic_ohat_ogg", "Semiacoustic Open Hat"},
    {112, "kit_simple_kick_ogg", "Simple Kick"},
    {113, "kit_simple_snare_ogg", "Simple Snare"},
    {114, "kit_simple_chat_ogg", "Simple Closed Hat"},
    {115, "kit_simple_ohat_ogg", "Simple Open Hat"},
    {116, "kit_subterranean_kick_ogg", "Subterranean Kick"},
    {117, "kit_subterranean_snare_ogg", "Subterranean Snare"},
    {118, "kit_subterranean_chat_ogg", "Subterranean Closed Hat"},
    {119, "kit_subterranean_ohat_ogg", "Subterranean Open Hat"},
    {120, "kit_tight_kick_ogg", "Tight Kick"},
    {121, "kit_tight_snare_ogg", "Tight Snare"},
    {122, "kit_tight_chat_ogg", "Tight Closed Hat"},
    {123, "kit_tight_ohat_ogg", "Tight Open Hat"},
}};

struct SlotConfig {
    uint16_t sampleId = 0;
    float volume = 1.0f;
    float tuneSemitones = 0.0f;
    float startOffset = 0.0f;
    float decay = 1.0f;
    float velocitySensitivity = 1.0f;
};

inline constexpr uint8_t kSoundEffectSlotCount = 3;

struct DrumEffectSlots {
    std::array<uint16_t, kSoundEffectSlotCount> slots{0, 0, 0};
};

struct PresetDefinition {
    uint8_t presetId = 0;
    std::string_view name;
    std::array<SlotConfig, DrumSampleCatalog::kSlots.size()> slots{};
    DrumEffectSlots effectSlots{};
};

inline constexpr std::array<PresetDefinition, 24> kPresets{{

    // === MODULAR KIT (presets 0-3) ===
    {   // 0: Modular Punch — punchy, tight kick, crisp snare
        0, "Modular Punch",
        {{{100, 1.00f, -1.0f, 0.00f, 0.92f, 1.00f},
          {101, 0.88f,  0.0f, 0.00f, 0.76f, 0.96f},
          {102, 0.62f,  1.0f, 0.01f, 0.38f, 0.82f},
          {103, 0.72f,  0.0f, 0.02f, 0.56f, 0.82f}}},
        {{FxDisplayId::TransientSnap, FxDisplayId::CompTight, 0}}
    },
    {   // 1: Modular Voltage — layered pop snare, brighter hats
        1, "Modular Voltage",
        {{{100, 1.02f, -1.0f, 0.00f, 0.88f, 1.00f},
          {105, 0.90f,  0.0f, 0.00f, 0.68f, 0.96f},
          {106, 0.62f,  2.0f, 0.01f, 0.32f, 0.82f},
          {103, 0.74f,  0.0f, 0.02f, 0.52f, 0.80f}}},
        {{FxDisplayId::ClipSoftGlue, FxDisplayId::EchoSlapback, FxDisplayId::DuckSoft}}
    },
    {   // 2: Modular Industrial — detuned kick, metallic hats, short decay
        2, "Modular Industrial",
        {{{100, 1.05f, -2.0f, 0.00f, 0.78f, 1.00f},
          {101, 0.92f,  1.0f, 0.00f, 0.62f, 0.90f},
          {102, 0.70f,  3.0f, 0.01f, 0.28f, 0.78f},
          {103, 0.78f,  2.0f, 0.02f, 0.42f, 0.78f}}},
        {{FxDisplayId::TransientTight, FxDisplayId::CombTight, FxDisplayId::Redux12bitClean}}
    },
    {   // 3: Modular Punch Room — roomier decay, slightly detuned
        3, "Modular Punch Room",
        {{{100, 0.98f, -1.0f, 0.00f, 1.00f, 1.00f},
          {101, 0.85f,  0.0f, 0.00f, 0.88f, 0.96f},
          {102, 0.58f,  1.0f, 0.01f, 0.50f, 0.82f},
          {103, 0.68f,  0.0f, 0.02f, 0.72f, 0.82f}}},
        {{FxDisplayId::TransientSnap, FxDisplayId::DrumTightRoom, FxDisplayId::CompGlue}}
    },

    // === POP KIT (presets 4-7) ===
    {   // 4: Pop Tape — warm, rounded, tape-character
        4, "Pop Tape",
        {{{104, 1.02f,  0.0f, 0.00f, 0.88f, 1.00f},
          {105, 0.92f,  0.0f, 0.00f, 0.72f, 0.96f},
          {106, 0.66f,  2.0f, 0.01f, 0.34f, 0.84f},
          {107, 0.78f,  1.0f, 0.02f, 0.52f, 0.84f}}},
        {{FxDisplayId::Redux12bitClean, FxDisplayId::DrumSoftPlate, 0}}
    },
    {   // 5: Pop Quake — deep sub kick, subterranean snare
        5, "Pop Quake",
        {{{116, 1.08f, -4.0f, 0.00f, 1.00f, 1.00f},
          {101, 0.82f, -2.0f, 0.00f, 0.76f, 0.90f},
          {118, 0.56f, -3.0f, 0.01f, 0.44f, 0.72f},
          {119, 0.74f, -2.0f, 0.02f, 0.68f, 0.72f}}},
        {{FxDisplayId::ResonatorBody, FxDisplayId::DuckClub, FxDisplayId::DrumDarkRoom}}
    },
    {   // 6: Pop Crisp — bright, tight, modern pop
        6, "Pop Crisp",
        {{{104, 1.06f,  0.0f, 0.00f, 0.78f, 1.00f},
          {105, 0.96f,  0.0f, 0.00f, 0.62f, 0.96f},
          {106, 0.70f,  2.0f, 0.01f, 0.28f, 0.84f},
          {107, 0.82f,  1.0f, 0.02f, 0.44f, 0.84f}}},
        {{FxDisplayId::TransientBody, FxDisplayId::EQDrumClean, FxDisplayId::LimiterTransparent}}
    },
    {   // 7: Pop Slam — hard-hitting, compressed
        7, "Pop Slam",
        {{{104, 1.08f,  0.0f, 0.00f, 0.82f, 1.00f},
          {105, 0.98f,  0.0f, 0.00f, 0.68f, 0.96f},
          {106, 0.68f,  2.0f, 0.01f, 0.30f, 0.84f},
          {107, 0.80f,  1.0f, 0.02f, 0.48f, 0.84f}}},
        {{FxDisplayId::TransientSnap, FxDisplayId::CompGlue, FxDisplayId::LimiterLoud}}
    },

    // === SEMIACOUSTIC KIT (presets 8-11) ===
    {   // 8: Semiacoustic Glitch — detuned, lo-fi character
        8, "Semiacoustic Glitch",
        {{{108, 0.98f, -2.0f, 0.00f, 0.96f, 0.96f},
          {109, 0.94f, -1.0f, 0.00f, 0.82f, 0.92f},
          {110, 0.58f, -1.0f, 0.01f, 0.42f, 0.76f},
          {111, 0.70f, -1.0f, 0.02f, 0.64f, 0.76f}}},
        {{FxDisplayId::RepeatBlend1_8, FxDisplayId::Redux12bitClean, FxDisplayId::CompTight}}
    },
    {   // 9: Semiacoustic Neon — tight snap, bright ping
        9, "Semiacoustic Neon",
        {{{112, 0.98f,  0.0f, 0.00f, 0.78f, 1.00f},
          {121, 0.92f,  1.0f, 0.00f, 0.56f, 1.00f},
          {122, 0.60f,  3.0f, 0.01f, 0.24f, 0.86f},
          {115, 0.62f,  1.0f, 0.02f, 0.40f, 0.80f}}},
        {{FxDisplayId::PanChopper, FxDisplayId::FlangeJet, FxDisplayId::EchoPingMid}}
    },
    {   // 10: Semiacoustic Warm Room — open, resonant, warm
        10, "Semiacoustic Warm Room",
        {{{108, 0.96f, -2.0f, 0.00f, 1.00f, 0.96f},
          {109, 0.92f, -1.0f, 0.00f, 0.90f, 0.92f},
          {110, 0.56f, -1.0f, 0.01f, 0.52f, 0.76f},
          {111, 0.68f, -1.0f, 0.02f, 0.76f, 0.76f}}},
        {{FxDisplayId::DrumTightRoom, FxDisplayId::CompTight, FxDisplayId::SaturateWarm}}
    },
    {   // 11: Semiacoustic LoFi Tape — degraded, short decay
        11, "Semiacoustic LoFi Tape",
        {{{108, 0.94f, -2.0f, 0.00f, 0.72f, 0.96f},
          {109, 0.90f, -1.0f, 0.00f, 0.60f, 0.92f},
          {110, 0.54f, -1.0f, 0.01f, 0.32f, 0.76f},
          {111, 0.66f, -1.0f, 0.02f, 0.48f, 0.76f}}},
        {{FxDisplayId::Redux8bitCrunch, FxDisplayId::DrumDarkRoom, FxDisplayId::TapeSlapback}}
    },

    // === SIMPLE KIT (presets 12-15) ===
    {   // 12: Simple Lofi — reduced bit-crushed character
        12, "Simple Lofi",
        {{{112, 1.00f,  0.0f, 0.00f, 0.84f, 1.00f},
          {113, 0.86f,  0.0f, 0.00f, 0.68f, 0.92f},
          {114, 0.54f,  0.0f, 0.01f, 0.30f, 0.76f},
          {115, 0.66f,  0.0f, 0.02f, 0.46f, 0.76f}}},
        {{FxDisplayId::Redux8bitCrunch, 0, 0}}
    },
    {   // 13: Simple Sunset — cave club hybrid, deep and spacious
        13, "Simple Sunset",
        {{{116, 1.05f, -3.0f, 0.00f, 0.96f, 1.00f},
          {109, 0.92f, -1.0f, 0.00f, 0.80f, 0.92f},
          {110, 0.58f, -1.0f, 0.01f, 0.42f, 0.76f},
          {111, 0.72f, -1.0f, 0.02f, 0.62f, 0.76f}}},
        {{0, 0, 0}}
    },
    {   // 14: Simple Dry Punch — very dry, tight, no reverb
        14, "Simple Dry Punch",
        {{{112, 1.02f,  0.0f, 0.00f, 0.68f, 1.00f},
          {113, 0.90f,  0.0f, 0.00f, 0.52f, 0.92f},
          {114, 0.56f,  0.0f, 0.01f, 0.22f, 0.76f},
          {115, 0.68f,  0.0f, 0.02f, 0.36f, 0.76f}}},
        {{FxDisplayId::TransientTight, FxDisplayId::EQHPTight, 0}}
    },
    {   // 15: Simple Subtle Room — gentle room, longer decay
        15, "Simple Subtle Room",
        {{{112, 0.98f,  0.0f, 0.00f, 0.96f, 1.00f},
          {113, 0.84f,  0.0f, 0.00f, 0.80f, 0.92f},
          {114, 0.52f,  0.0f, 0.01f, 0.40f, 0.76f},
          {115, 0.64f,  0.0f, 0.02f, 0.58f, 0.76f}}},
        {{FxDisplayId::CompTight, FxDisplayId::DrumTightRoom, 0}}
    },

    // === SUBTERRANEAN KIT (presets 16-19) ===
    {   // 16: Subterranean Impact — deep, powerful, hard-hitting
        16, "Subterranean Impact",
        {{{116, 1.06f, -3.0f, 0.00f, 0.98f, 1.00f},
          {117, 0.88f, -2.0f, 0.00f, 0.78f, 0.90f},
          {118, 0.58f, -3.0f, 0.01f, 0.48f, 0.72f},
          {119, 0.76f, -2.0f, 0.02f, 0.70f, 0.72f}}},
        {{FxDisplayId::TransientBody, FxDisplayId::LimiterBrickwall, 0}}
    },
    {   // 17: Subterranean Bloom — ethereal, long decay, shimmering
        17, "Subterranean Bloom",
        {{{120, 1.00f,  1.0f, 0.00f, 0.72f, 1.00f},
          {101, 0.86f,  1.0f, 0.00f, 0.62f, 0.94f},
          {102, 0.60f,  4.0f, 0.01f, 0.28f, 0.86f},
          {123, 0.68f,  3.0f, 0.02f, 0.38f, 0.86f}}},
        {{FxDisplayId::ChorusSoft, FxDisplayId::DrumSoftPlate, 0}}
    },
    {   // 18: Subterranean Deep — very low, sub-heavy, filtered
        18, "Subterranean Deep",
        {{{116, 1.10f, -4.0f, 0.00f, 1.00f, 1.00f},
          {117, 0.92f, -3.0f, 0.00f, 0.88f, 0.90f},
          {118, 0.60f, -4.0f, 0.01f, 0.56f, 0.72f},
          {119, 0.78f, -3.0f, 0.02f, 0.80f, 0.72f}}},
        {{FxDisplayId::ResonatorBody, FxDisplayId::CompGlue, FxDisplayId::EQHPTight}}
    },
    {   // 19: Subterranean Rumble — roomy, boomy, sustained
        19, "Subterranean Rumble",
        {{{116, 1.08f, -3.0f, 0.00f, 1.00f, 1.00f},
          {117, 0.90f, -2.0f, 0.00f, 0.92f, 0.90f},
          {118, 0.60f, -3.0f, 0.01f, 0.60f, 0.72f},
          {119, 0.78f, -2.0f, 0.02f, 0.84f, 0.72f}}},
        {{FxDisplayId::ResonatorBody, FxDisplayId::DrumDarkRoom, FxDisplayId::LimiterLoud}}
    },

    // === TIGHT KIT (presets 20-23) ===
    {   // 20: Tight Razor — sharp, gated, aggressive
        20, "Tight Razor",
        {{{120, 1.04f,  1.0f, 0.00f, 0.74f, 1.00f},
          {121, 0.94f,  1.0f, 0.00f, 0.58f, 0.98f},
          {122, 0.64f,  3.0f, 0.01f, 0.26f, 0.88f},
          {123, 0.70f,  2.0f, 0.02f, 0.40f, 0.88f}}},
        {{FxDisplayId::TransientTight, FxDisplayId::ClipCrunch, FxDisplayId::GatePump1_8}}
    },
    {   // 21: Tight Crystal — glassy, detuned, shimmering
        21, "Tight Crystal",
        {{{104, 1.02f,  0.0f, 0.00f, 0.86f, 1.00f},
          {117, 0.92f, -1.0f, 0.00f, 0.82f, 0.92f},
          {114, 0.58f,  0.0f, 0.01f, 0.30f, 0.80f},
          {111, 0.74f, -1.0f, 0.02f, 0.60f, 0.78f}}},
        {{FxDisplayId::ChorusSoft, FxDisplayId::PitchMicroDetune, FxDisplayId::DrumSoftPlate}}
    },
    {   // 22: Tight Snap — short, punchy, transient-forward
        22, "Tight Snap",
        {{{120, 1.06f,  1.0f, 0.00f, 0.62f, 1.00f},
          {121, 0.96f,  1.0f, 0.00f, 0.48f, 0.98f},
          {122, 0.66f,  3.0f, 0.01f, 0.20f, 0.88f},
          {123, 0.72f,  2.0f, 0.02f, 0.32f, 0.88f}}},
        {{FxDisplayId::TransientSnap, FxDisplayId::GateSwing1_8, FxDisplayId::CompTight}}
    },
    {   // 23: Tight Stutter — rhythmic, chopped, glitchy
        23, "Tight Stutter",
        {{{120, 1.02f,  1.0f, 0.00f, 0.70f, 1.00f},
          {121, 0.90f,  1.0f, 0.00f, 0.54f, 0.98f},
          {122, 0.62f,  3.0f, 0.01f, 0.24f, 0.88f},
          {123, 0.68f,  2.0f, 0.02f, 0.38f, 0.88f}}},
        {{FxDisplayId::RepeatBlend1_8, FxDisplayId::TransientTight, FxDisplayId::DuckSoft}}
    },
}};

inline constexpr std::array<float, kPresets.size()> kPresetLoudnessTrims{{
    0.7163f, 0.8836f, 0.6051f, 1.0595f, 0.6436f, 1.4399f,
    1.0277f, 1.0346f, 1.9953f, 1.2363f, 0.9730f, 1.1166f,
    0.4197f, 0.4556f, 0.9900f, 0.8246f, 0.7523f, 1.1369f,
    1.9953f, 1.1136f, 0.5662f, 0.5946f, 0.7031f, 1.3502f
}};

inline constexpr bool isEmbeddedSampleId(uint16_t sampleId) {
    if (sampleId < kEmbeddedSampleIdStart || sampleId > kEmbeddedSampleIdEnd) {
        return false;
    }

    for (const auto& sample : kEmbeddedSamples) {
        if (sample.sampleId == sampleId) {
            return true;
        }
    }

    return false;
}

inline constexpr const EmbeddedSampleDefinition* findEmbeddedSampleById(uint16_t sampleId) {
    for (const auto& sample : kEmbeddedSamples) {
        if (sample.sampleId == sampleId) {
            return &sample;
        }
    }

    return nullptr;
}

inline constexpr uint8_t getPresetCount() {
    return static_cast<uint8_t>(kPresets.size());
}

inline constexpr bool isValidPresetId(uint8_t presetId) {
    return presetId < getPresetCount();
}

inline constexpr std::array<uint16_t, kSoundEffectSlotCount> getPresetEffectSlots(uint8_t presetId) {
    if (isValidPresetId(presetId)) {
        const auto displayIds = kPresets[presetId].effectSlots.slots;
        return {
            EffectPresetCatalog::displayIdToPresetId(displayIds[0]),
            EffectPresetCatalog::displayIdToPresetId(displayIds[1]),
            EffectPresetCatalog::displayIdToPresetId(displayIds[2])
        };
    }
    return {0, 0, 0};
}

inline constexpr const PresetDefinition& getPreset(uint8_t presetId) {
    if (isValidPresetId(presetId)) {
        return kPresets[presetId];
    }

    return kPresets[0];
}

inline constexpr std::string_view getPresetName(uint8_t presetId) {
    return getPreset(presetId).name;
}

inline constexpr float getPresetLoudnessTrim(uint8_t presetId) {
    return isValidPresetId(presetId) ? kPresetLoudnessTrims[presetId] : 1.0f;
}

} // namespace DrumKitPresetCatalog
