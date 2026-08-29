#include "../Source/UI/Components/AgentInputBar.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <cassert>
#include <iostream>
#include <string>

namespace {

std::string renderBar(const std::string& text, bool active) {
    auto screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(32),
        ftxui::Dimension::Fixed(1));
    ftxui::Render(screen, AgentInputBar(text, active));
    return screen.ToString();
}

void testInactiveBarRendersAsOneLine() {
    const std::string rendered = renderBar("mu> ready", false);
    assert(rendered.find("mu> ready") != std::string::npos);
    assert(rendered.find('\n') == std::string::npos ||
           rendered.find('\n') == rendered.size() - 1);
}

void testActiveBarRendersPromptAndInput() {
    const std::string rendered = renderBar("mu> state full|", true);
    assert(rendered.find("mu> state full|") != std::string::npos);
}

} // namespace

int main() {
    testInactiveBarRendersAsOneLine();
    testActiveBarRendersPromptAndInput();

    std::cout << "AgentInputBar tests passed!\n";
    return 0;
}
