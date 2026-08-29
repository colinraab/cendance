#pragma once

#include <cstdint>
#include <ftxui/dom/elements.hpp>
#include <string>

enum class ArrangementModalFocus : uint8_t {
    Preset = 0,
    SectionCount,
    CurrentSection,
    SectionLength,
    ProgressionSource,
    TrackMask,
    TrackParameter,
};

ftxui::Element ArrangementModal(uint8_t sectionCount,
                                uint8_t currentSection,
                                uint8_t sectionLengthBars,
                                bool progressionUsesGlobal,
                                uint8_t progressionDisplayId,
                                const std::string& progressionLabel,
                                uint8_t trackMask,
                                bool sectionParametersEnabled,
                                uint8_t parameterTrackIndex,
                                uint8_t parameterIndex,
                                float parameterValue,
                                const std::string& presetLabel,
                                const std::string& chainStatus,
                                ArrangementModalFocus focus,
                                const std::string& statusMessage,
                                bool statusIsError);
