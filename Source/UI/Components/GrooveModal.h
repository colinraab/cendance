#pragma once

#include <cstdint>
#include <ftxui/dom/elements.hpp>
#include <string>

enum class GrooveModalFocus : uint8_t {
    Swing = 0,
    Velocity,
    Timing,
};

ftxui::Element GrooveModal(float swingAmount,
                           float velocityHumanize,
                           float timingJitter,
                           GrooveModalFocus focus,
                           const std::string& statusMessage,
                           bool statusIsError);
