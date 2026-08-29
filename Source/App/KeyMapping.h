#pragma once

#include "CommandQueue.h"
#include <ftxui/component/event.hpp>
#include <optional>

// Maps raw keypresses to Command objects. Pure function, no state.
// selectedTrack is 0-3 based on UI selection.
std::optional<Command> mapKeyToCommand(const ftxui::Event& event, uint8_t selectedTrack);
