#include "TuiAppInput.h"

#include <cctype>
#include <cstdint>
#include <limits>
#include <string>

namespace tui_input {

std::string trimCopy(const std::string& text) {
  const auto begin = text.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }

  const auto end = text.find_last_not_of(" \t\r\n");
  return text.substr(begin, (end - begin) + 1);
}

bool isMasterTrackOnlyIgnoredCommand(Command::Type type) {
  switch (type) {
  case Command::Type::SetAlgorithm:
  case Command::Type::StepAlgorithm:
  case Command::Type::SetDensity:
  case Command::Type::SetComplexity:
  case Command::Type::SetSynthPreset:
  case Command::Type::StepSynthPreset:
  case Command::Type::SetTone:
  case Command::Type::SetMotion:
  case Command::Type::ToggleTrackMute:
  case Command::Type::SetTrackEffectPreset:
    return true;
  default:
    return false;
  }
}

bool parseDisplayId(const std::string& input, uint16_t& output) {
  if (input.empty()) {
    return false;
  }

  uint32_t value = 0;
  for (const char ch : input) {
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }

    value = value * 10u + static_cast<uint32_t>(ch - '0');
    if (value > std::numeric_limits<uint16_t>::max()) {
      return false;
    }
  }

  output = static_cast<uint16_t>(value);
  return true;
}

std::string slotLabel(const uint8_t slotIndex) {
  return "Slot " + std::to_string(static_cast<int>(slotIndex) + 1);
}

bool parseEffectSelectionInput(const std::string& input,
                               EffectSelectionInput& output) {
  output = EffectSelectionInput{};

  if (input.empty()) {
    return false;
  }

  const char slotChar = input[0];
  if (slotChar < '1' || slotChar > '3') {
    return false;
  }

  output.slotIndex = static_cast<uint8_t>(slotChar - '1');
  if (input.size() == 1) {
    output.kind = EffectSelectionKind::SlotOnly;
    return true;
  }

  const char categoryChar = input[1];
  if (categoryChar == '-') {
    if (input.size() != 2) {
      return false;
    }

    output.kind = EffectSelectionKind::ClearSlot;
    return true;
  }

  if (!std::isdigit(static_cast<unsigned char>(categoryChar))) {
    return false;
  }

  output.categoryDigit = static_cast<uint8_t>(categoryChar - '0');
  if (input.size() == 2) {
    output.kind = EffectSelectionKind::CategoryOnly;
    return true;
  }

  uint32_t presetValue = 0;
  for (size_t i = 2; i < input.size(); ++i) {
    const char ch = input[i];
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }

    presetValue = presetValue * 10u + static_cast<uint32_t>(ch - '0');
    if (presetValue > std::numeric_limits<uint16_t>::max()) {
      return false;
    }
  }

  if (presetValue == 0) {
    return false;
  }

  output.kind = EffectSelectionKind::Preset;
  output.presetDisplayId = static_cast<uint16_t>(presetValue);
  return true;
}

} // namespace tui_input
