#pragma once

#include "CommandQueue.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace SpotEffectCatalog {

struct SpotEffectDefinition {
    Command::SpotEffectId id;
    std::string_view name;
    char keyHint;
    uint8_t bitMask;
    float paramA;
    float paramB;
};

inline constexpr std::array<SpotEffectDefinition, 2> kSpotEffects{{
    {Command::SpotEffectId::TapeBrake, "Tape Brake", 'Z', static_cast<uint8_t>(1u << 0), 0.25f, 0.0f},
    {Command::SpotEffectId::Stutter, "Stutter", 'X', static_cast<uint8_t>(1u << 1), 0.5f, 1.0f},
}};

inline constexpr uint8_t getSupportedBitmask() {
    uint8_t mask = 0;
    for (const auto& spotEffect : kSpotEffects) {
        mask = static_cast<uint8_t>(mask | spotEffect.bitMask);
    }
    return mask;
}

inline constexpr bool isValidSpotEffectId(uint16_t rawId) {
    return Command::isValidSpotEffectId(rawId);
}

inline constexpr uint8_t getBitMask(Command::SpotEffectId id) {
    for (const auto& spotEffect : kSpotEffects) {
        if (spotEffect.id == id) {
            return spotEffect.bitMask;
        }
    }
    return 0u;
}

inline constexpr const SpotEffectDefinition& getDefinition(Command::SpotEffectId id) {
    for (const auto& spotEffect : kSpotEffects) {
        if (spotEffect.id == id) {
            return spotEffect;
        }
    }
    return kSpotEffects[0];
}

inline constexpr float getTapeBrakeDefaultStopTimeSeconds() {
    return getDefinition(Command::SpotEffectId::TapeBrake).paramA;
}

inline constexpr float getStutterDefaultRepeatDivision() {
    return getDefinition(Command::SpotEffectId::Stutter).paramA;
}

inline constexpr float getStutterDefaultMix() {
    return getDefinition(Command::SpotEffectId::Stutter).paramB;
}

} // namespace SpotEffectCatalog
