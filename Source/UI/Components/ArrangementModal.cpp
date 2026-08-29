#include "ArrangementModal.h"
#include "../Themes/Colors.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>

namespace {

std::string trackMaskLabel(uint8_t mask) {
    std::string label;
    label.reserve(4);
    label.push_back((mask & 0x1u) ? 'D' : '-');
    label.push_back((mask & 0x2u) ? 'B' : '-');
    label.push_back((mask & 0x4u) ? 'C' : '-');
    label.push_back((mask & 0x8u) ? 'L' : '-');
    return label;
}

const char* trackLabel(uint8_t track) {
    switch (track) {
    case 0: return "Drums";
    case 1: return "Bass";
    case 2: return "Chords";
    default: return "Lead";
    }
}

const char* parameterLabel(uint8_t parameter) {
    switch (parameter) {
    case 0: return "Density";
    case 1: return "Complexity";
    case 2: return "Tone";
    default: return "Move";
    }
}

std::string percentLabel(float value) {
    std::ostringstream out;
    out << static_cast<int>(std::round(std::clamp(value, 0.0f, 1.0f) * 100.0f)) << "%";
    return out.str();
}

ftxui::Element modalRow(const std::string& label,
                        const std::string& value,
                        bool focused) {
    using namespace ftxui;
    auto row = hbox({
        text(label) | bold,
        filler(),
        text(value)
    });

    if (focused) {
        return row | color(Theme::Highlight);
    }

    return row | color(Theme::Foreground);
}

} // namespace

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
                                bool statusIsError) {
    using namespace ftxui;

    const std::string progressionText = progressionUsesGlobal
        ? "Global"
        : ("Override " + std::to_string(progressionDisplayId)
            + (progressionLabel.empty() ? "" : (" (" + progressionLabel + ")")));
    const std::string status = statusMessage.empty()
        ? "Enter=Apply Esc=Cancel Tab=Next Field Arrow Left/Right=Adjust"
        : statusMessage;

    auto body = vbox({
        modalRow("Preset", presetLabel, focus == ArrangementModalFocus::Preset),
        modalRow("Section Count", std::to_string(static_cast<int>(sectionCount)), focus == ArrangementModalFocus::SectionCount),
        modalRow("Current Section", std::to_string(static_cast<int>(currentSection) + 1), focus == ArrangementModalFocus::CurrentSection),
        modalRow("Section Length", std::to_string(static_cast<int>(sectionLengthBars)) + " bars", focus == ArrangementModalFocus::SectionLength),
        modalRow("Progression", progressionText, focus == ArrangementModalFocus::ProgressionSource),
        modalRow("Track Mask", trackMaskLabel(trackMask), focus == ArrangementModalFocus::TrackMask),
        modalRow("Section Slider", std::string(trackLabel(parameterTrackIndex)) + " " + parameterLabel(parameterIndex) + " " + percentLabel(parameterValue)
                 + (sectionParametersEnabled ? "" : " (off)"), focus == ArrangementModalFocus::TrackParameter),
        text("Chain: " + (chainStatus.empty() ? std::string("Linear") : chainStatus)) | color(Theme::Inactive),
        separator(),
        text("Track mask toggles: 1=Drums 2=Bass 3=Chords 4=Lead") | color(Theme::Inactive),
        text("Sliders: T=track P=parameter C=capture section X=toggle section sliders") | color(Theme::Inactive),
        text("Presets: choose Preset row, Left/Right cycle, Enter applies") | color(Theme::Inactive),
        text("Progression: G=Global, +/- or Arrow L/R = override index") | color(Theme::Inactive),
        text(status) | color(statusIsError ? Theme::Error : Theme::Foreground)
    });

    return window(text(" Arrangement Editor ") | bold | center, body) | clear_under | center;
}
