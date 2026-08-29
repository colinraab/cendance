#pragma once
#include <ftxui/dom/elements.hpp>
#include <cstdint>

ftxui::Element TransportBar(float bpm,
							bool isPlaying,
							uint16_t barNumber,
							uint32_t beatPosition,
							uint8_t progressionId,
							uint8_t projectKeyRoot,
							uint8_t projectKeyMode,
							int terminalWidth);

