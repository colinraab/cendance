#include "GrooveModal.h"
#include "ParameterBar.h"
#include "../Themes/Colors.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace {

std::string percentLabel(float value) {
    std::ostringstream out;
    out << static_cast<int>(std::round(std::clamp(value, 0.0f, 1.0f) * 100.0f)) << "%";
    return out.str();
}

const char* focusLabel(GrooveModalFocus focus) {
    switch (focus) {
    case GrooveModalFocus::Swing: return "Swing";
    case GrooveModalFocus::Velocity: return "Velocity";
    case GrooveModalFocus::Timing: return "Timing";
    }
    return "Groove";
}

} // namespace

ftxui::Element GrooveModal(float swingAmount,
                           float velocityHumanize,
                           float timingJitter,
                           GrooveModalFocus focus,
                           const std::string& statusMessage,
                           bool statusIsError) {
    using namespace ftxui;

    constexpr int barWidth = 20;

    auto makeRow = [&](const std::string& label,
                       float value,
                       GrooveModalFocus thisFocus) -> Element {
        const bool isFocused = (focus == thisFocus);
        auto row = hbox({
            text(isFocused ? "> " : "  "),
            ParameterBar(label, value, barWidth),
            filler(),
            text(percentLabel(value)),
        });

        return row | color(isFocused ? Theme::Highlight : Theme::Foreground);
    };

    const std::string status = statusMessage.empty()
        ? "Left/Right adjust  Tab/Up/Down focus  0/5/9 quick set"
        : statusMessage;

    auto content = vbox({
        text("Editing: " + std::string(focusLabel(focus))) | bold | color(Theme::Active),
        separator(),
        makeRow("Swing", swingAmount, GrooveModalFocus::Swing),
        makeRow("Vel", velocityHumanize, GrooveModalFocus::Velocity),
        makeRow("Jitr", timingJitter, GrooveModalFocus::Timing),
        separator(),
        text(status) | color(statusIsError ? Theme::Error : Theme::Foreground),
        text("Enter=Done  Esc=Close") | color(Theme::Inactive),
    });

    return window(text(" Groove ") | bold | center,
                  content | size(WIDTH, EQUAL, 48)) |
           clear_under | center;
}
