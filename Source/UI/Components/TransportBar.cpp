#include "TransportBar.h"
#include "../../App/ProjectKey.h"
#include "../../Audio/Harmony/ChordProgression.h"
#include "../Themes/Colors.h"
#include <string>

namespace {

std::string formatProgressionDisplay(uint8_t progressionId, bool compact) {
    const auto& progression = ChordProgression::get(static_cast<int>(progressionId));

    if (compact) {
        // Short form: just the name
        return std::string(progression.name);
    }

    std::string degreeSteps;
    for (size_t i = 0; i < progression.degrees.size(); ++i) {
        if (i > 0) {
            degreeSteps += "→";
        }
        degreeSteps += std::to_string(progression.degrees[i] + 1);
    }

    return std::string(progression.name) + " (" + degreeSteps + ")";
}

std::string makeBeatDots(uint32_t beatPosition, bool isPlaying, bool spaced) {
    std::string s;
    for (int i = 0; i < 4; ++i) {
        if (i == static_cast<int>(beatPosition) && isPlaying) {
            s += "●";
        } else {
            s += "○";
        }
        if (spaced && i < 3) s += " ";
    }
    return s;
}

} // namespace

ftxui::Element TransportBar(float bpm,
                            bool isPlaying,
                            uint16_t barNumber,
                            uint32_t beatPosition,
                            uint8_t progressionId,
                            uint8_t projectKeyRoot,
                            uint8_t projectKeyMode,
                            int terminalWidth) {
    using namespace ftxui;

    const bool wide = (terminalWidth >= 100);

    const std::string playStr = isPlaying ? "▶ PLAY" : "⏸ STOP";
    const std::string bpmStr  = "BPM:" + std::to_string(static_cast<int>(bpm));
    const std::string barStr  = "Bar:" + std::to_string(barNumber + 1);
    const std::string beatStr = makeBeatDots(beatPosition, isPlaying, wide);
    const std::string keyStr  = "Key:" + ProjectKey::format(projectKeyRoot, projectKeyMode);
    const std::string progStr = "Prog:" + formatProgressionDisplay(progressionId, !wide);

    auto playEl = text(playStr) | bold | color(isPlaying ? Theme::Active : Theme::Inactive);
    auto bpmEl  = text(bpmStr);
    auto barEl  = text(barStr);
    auto beatEl = text(beatStr) | color(isPlaying ? Theme::Active : Theme::Inactive);
    auto keyEl  = text(keyStr);
    auto progEl = text(progStr);

    if (wide) {
        // Single row, tight spacing with separators
        return hbox({
            playEl | size(WIDTH, EQUAL, 8),
            text(" │ ") | color(Theme::Inactive),
            bpmEl  | size(WIDTH, EQUAL, 8),
            text(" │ ") | color(Theme::Inactive),
            barEl  | size(WIDTH, EQUAL, 7),
            text(" │ ") | color(Theme::Inactive),
            beatEl | size(WIDTH, EQUAL, 8),
            text(" │ ") | color(Theme::Inactive),
            keyEl,
            text(" │ ") | color(Theme::Inactive),
            progEl | flex,
        });
    } else {
        // Two rows for narrow terminals
        auto row1 = hbox({
            playEl | size(WIDTH, EQUAL, 8),
            text(" ") | color(Theme::Inactive),
            bpmEl  | size(WIDTH, EQUAL, 8),
            text(" ") | color(Theme::Inactive),
            barEl  | size(WIDTH, EQUAL, 7),
            text(" ") | color(Theme::Inactive),
            beatEl,
        });
        auto row2 = hbox({
            keyEl,
            text("  ") | color(Theme::Inactive),
            progEl | flex,
        });
        return vbox({row1, row2});
    }
}

