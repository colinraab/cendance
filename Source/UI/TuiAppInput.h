#pragma once

#include "../App/CommandQueue.h"
#include <cstdint>
#include <string>

namespace tui_input {

inline constexpr int kUiTrackCount = 5;
inline constexpr int kMasterTrackIndex = 4;
inline constexpr size_t kMaxAgentInputChars = 240;
inline constexpr size_t kMaxNumberSelectionDigits = 6;
inline constexpr size_t kMaxKeySelectionChars = 24;

std::string trimCopy(const std::string& text);

bool isMasterTrackOnlyIgnoredCommand(Command::Type type);

bool parseDisplayId(const std::string& input, uint16_t& output);

enum class EffectSelectionKind : uint8_t {
  Invalid,
  SlotOnly,
  CategoryOnly,
  ClearSlot,
  Preset,
};

struct EffectSelectionInput {
  EffectSelectionKind kind = EffectSelectionKind::Invalid;
  uint8_t slotIndex = 0;
  uint8_t categoryDigit = 0;
  uint16_t presetDisplayId = 0;
};

std::string slotLabel(const uint8_t slotIndex);

bool parseEffectSelectionInput(const std::string& input,
                               EffectSelectionInput& output);

} // namespace tui_input
