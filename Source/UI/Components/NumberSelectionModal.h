#pragma once

#include <cstdint>
#include <ftxui/dom/elements.hpp>
#include <string>

enum class NumberSelectionDomain : uint8_t {
    Algorithm,
    Sound,
    ChordProgression,
    EffectPreset,
    Genre,
};

ftxui::Element NumberSelectionModal(NumberSelectionDomain domain,
                                    const std::string& inputDigits,
                                    const std::string& previewName,
                                    bool isValid,
                                    uint16_t implementedCount,
                                    const std::string& statusMessage);
