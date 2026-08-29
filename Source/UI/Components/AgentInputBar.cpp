#include "AgentInputBar.h"

#include "../Themes/Colors.h"

#include <ftxui/dom/elements.hpp>

ftxui::Element AgentInputBar(const std::string& text, bool active) {
    using namespace ftxui;
    return hbox({
        ftxui::text(" " + text) |
            color(active ? Theme::Active : Theme::Inactive),
        filler(),
    });
}
