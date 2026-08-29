#include "TuiApp.h"
#include "../App/AlgorithmCatalog.h"
#include "../App/AlgorithmPresetRegistry.h"
#include "../App/SynthCatalog.h"
#include "../App/EffectPresetCatalog.h"
#include "../App/GenreCatalog.h"
#include "../App/ProjectKey.h"
#include "../Audio/Harmony/ChordProgression.h"
#include <string>
#include <cstdint>
#include <numeric>
#include <limits>
#include <sstream>

namespace {

constexpr size_t kMaxNumberSelectionDigits = 6;
constexpr size_t kMaxKeySelectionChars = 24;
constexpr int kMasterTrackIndex = 4;

bool parseDisplayId(const std::string &input, uint16_t &output) {
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

std::string slotLabel(const uint8_t slotIndex) {
  return "Slot " + std::to_string(static_cast<int>(slotIndex) + 1);
}

bool parseEffectSelectionInputImpl(const std::string &input,
                                   EffectSelectionInput &output) {
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

} // namespace

void TuiApp::openNumberSelection(NumberSelectionDomain domain) {
  numberSelectionOpen = true;
  numberSelectionDomain = domain;
  numberSelectionTrack = selectedTrack;
  clearNumberSelectionInput();
}

void TuiApp::closeNumberSelection() {
  numberSelectionOpen = false;
  numberSelectionInput.clear();
  numberSelectionPreview.clear();
  numberSelectionStatus.clear();
  numberSelectionValid = true;
}

void TuiApp::clearNumberSelectionInput() {
  numberSelectionInput.clear();
  numberSelectionPreview.clear();
  numberSelectionValid = true;

  const uint16_t implementedCount = getNumberSelectionImplementedCount();
  if (implementedCount == 0) {
    numberSelectionValid = false;
    numberSelectionStatus = "No options implemented.";
    return;
  }

  if (numberSelectionDomain == NumberSelectionDomain::EffectPreset) {
    numberSelectionStatus =
        "Enter [slot][category][preset] (e.g. 101) or [slot]- to clear.";
  } else if (numberSelectionDomain == NumberSelectionDomain::Genre) {
    numberSelectionStatus = "Enter genre ID 0-" +
                            std::to_string(GenreCatalog::kGenreCount) +
                            " (0 = free randomization).";
  } else {
    numberSelectionStatus = "Enter ID 1-" + std::to_string(implementedCount);
  }
}

uint16_t TuiApp::getNumberSelectionImplementedCount() const {
  switch (numberSelectionDomain) {
  case NumberSelectionDomain::Algorithm: {
    const uint16_t builtinCount = AlgorithmCatalog::getAlgorithmCountForTrack(
        static_cast<uint8_t>(numberSelectionTrack));
    const uint16_t customCount = globalAlgorithmPresetRegistry()
        .getCustomAlgorithmCountForTrack(static_cast<uint8_t>(numberSelectionTrack));
    return builtinCount + customCount;
  }
  case NumberSelectionDomain::Sound:
    return SynthCatalog::getPresetCountForTrack(
        static_cast<uint8_t>(numberSelectionTrack));
  case NumberSelectionDomain::ChordProgression:
    return static_cast<uint16_t>(ChordProgression::getNumProgressions());
  case NumberSelectionDomain::EffectPreset: {
    return EffectPresetCatalog::getCategoryMappedPresetCount();
  }
  case NumberSelectionDomain::Genre:
    return static_cast<uint16_t>(GenreCatalog::kGenreCount) + 1u; // 0–8: 0 = no genre
  }
  return 0;
}

void TuiApp::refreshNumberSelectionPreview() {
  const uint16_t implementedCount = getNumberSelectionImplementedCount();
  if (implementedCount == 0) {
    numberSelectionValid = false;
    numberSelectionPreview = "None";
    numberSelectionStatus = "No options implemented.";
    return;
  }

  if (numberSelectionInput.empty()) {
    numberSelectionValid = true;
    numberSelectionPreview.clear();
    if (numberSelectionDomain == NumberSelectionDomain::EffectPreset) {
      numberSelectionStatus =
          "Enter [slot][category][preset] (e.g. 101) or [slot]- to clear.";
    } else if (numberSelectionDomain == NumberSelectionDomain::Genre) {
      numberSelectionStatus = "Enter genre ID 0-" +
                              std::to_string(GenreCatalog::kGenreCount) +
                              " (0 = free randomization).";
    } else {
      numberSelectionStatus = "Enter ID 1-" + std::to_string(implementedCount);
    }
    return;
  }

  if (numberSelectionDomain == NumberSelectionDomain::EffectPreset) {
    EffectSelectionInput parsed;
    if (!parseEffectSelectionInputImpl(numberSelectionInput, parsed)) {
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Use [slot][category][preset] or [slot]-.";
      return;
    }

    const std::string slotName = slotLabel(parsed.slotIndex);
    switch (parsed.kind) {
    case EffectSelectionKind::SlotOnly:
      numberSelectionValid = true;
      numberSelectionPreview = slotName;
      numberSelectionStatus = "Enter category digit (0-9) or '-' to clear.";
      return;
    case EffectSelectionKind::CategoryOnly: {
      const std::string categoryName = std::string(
          EffectPresetCatalog::getCategoryName(parsed.categoryDigit));
      numberSelectionPreview = slotName + " -> " + categoryName;
      if (EffectPresetCatalog::isCategoryImplemented(parsed.categoryDigit)) {
        numberSelectionValid = true;
        numberSelectionStatus = "Enter preset number in category.";
      } else {
        numberSelectionValid = false;
        numberSelectionStatus =
            categoryName + " has no presets implemented yet.";
      }
      return;
    }
    case EffectSelectionKind::ClearSlot:
      numberSelectionValid = true;
      numberSelectionPreview = slotName + " -> Clear";
      numberSelectionStatus = "Press Enter to clear slot.";
      return;
    case EffectSelectionKind::Preset: {
      if (EffectPresetCatalog::isValidCategoryPresetDisplayId(
              parsed.categoryDigit, parsed.presetDisplayId)) {
        numberSelectionValid = true;
        numberSelectionPreview =
            slotName + " -> " +
            std::string(EffectPresetCatalog::getCategoryPresetNameByDisplayId(
                parsed.categoryDigit, parsed.presetDisplayId));
        numberSelectionStatus = "Press Enter to assign preset.";
      } else {
        numberSelectionValid = false;
        numberSelectionPreview = "Invalid";
        const std::string categoryName = std::string(
            EffectPresetCatalog::getCategoryName(parsed.categoryDigit));
        if (!EffectPresetCatalog::isCategoryImplemented(parsed.categoryDigit)) {
          numberSelectionStatus =
              categoryName + " has no presets implemented yet.";
        } else {
          numberSelectionStatus = "Preset out of range for " + categoryName +
                                  ". Press Enter to reset.";
        }
      }
      return;
    }
    case EffectSelectionKind::Invalid:
      break;
    }

    numberSelectionValid = false;
    numberSelectionPreview = "Invalid";
    numberSelectionStatus = "Use [slot][category][preset] or [slot]-.";
    return;
  }

  uint16_t displayId = 0;
  if (!parseDisplayId(numberSelectionInput, displayId)) {
    numberSelectionValid = false;
    numberSelectionPreview = "Invalid";
    numberSelectionStatus = "Invalid ID. Press Enter to reset.";
    return;
  }

  switch (numberSelectionDomain) {
  case NumberSelectionDomain::Algorithm: {
    const uint8_t track = static_cast<uint8_t>(numberSelectionTrack);
    if (displayId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase) {
      // Custom algorithm lookup
      const auto* preset = globalAlgorithmPresetRegistry().findByRuntimeId(track, displayId);
      if (preset != nullptr) {
        numberSelectionValid = true;
        numberSelectionPreview = preset->name + " (custom)";
        numberSelectionStatus = "Press Enter to apply.";
      } else {
        numberSelectionValid = false;
        numberSelectionPreview = "Invalid";
        numberSelectionStatus = "Custom algorithm not found.";
      }
    } else if (AlgorithmCatalog::isValidDisplayIdForTrack(track, displayId)) {
      numberSelectionValid = true;
      numberSelectionPreview = std::string(
          AlgorithmCatalog::getAlgorithmNameByDisplayId(track, displayId));
      numberSelectionStatus = "Press Enter to apply.";
    } else {
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Press Enter to reset.";
    }
    break;
  }
  case NumberSelectionDomain::Sound: {
    const uint8_t track = static_cast<uint8_t>(numberSelectionTrack);
    if (SynthCatalog::isValidDisplayIdForTrack(track, displayId)) {
      numberSelectionValid = true;
      numberSelectionPreview =
          std::string(SynthCatalog::getPresetNameByDisplayId(track, displayId));
      numberSelectionStatus = "Press Enter to apply.";
    } else {
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Press Enter to reset.";
    }
    break;
  }
  case NumberSelectionDomain::ChordProgression: {
    if (ChordProgression::isValidDisplayId(displayId)) {
      numberSelectionValid = true;
      numberSelectionPreview =
          std::string(ChordProgression::getNameByDisplayId(displayId));
      numberSelectionStatus = "Press Enter to apply.";
    } else {
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Press Enter to reset.";
    }
    break;
  }
  case NumberSelectionDomain::Genre: {
    if (displayId == 0) {
      numberSelectionValid = true;
      numberSelectionPreview = "Free Randomization";
      numberSelectionStatus = "Normal startup randomization | Enter to select";
    } else if (GenreCatalog::isValidGenreId(displayId)) {
      numberSelectionValid = true;
      numberSelectionPreview =
          std::string(GenreCatalog::getGenreName(displayId - 1));
      float minB = GenreCatalog::getGenreMinBpm(displayId - 1);
      float maxB = GenreCatalog::getGenreMaxBpm(displayId - 1);
      numberSelectionStatus = "BPM " + std::to_string(static_cast<int>(minB)) +
          "-" + std::to_string(static_cast<int>(maxB)) + " | Enter to select";
    } else {
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Genre out of range (0-8). 0=no genre, Press Enter to reset.";
    }
    break;
  }
  case NumberSelectionDomain::EffectPreset:
    break;
  }
}

bool TuiApp::submitNumberSelection() {
  if (!numberSelectionOpen) {
    return false;
  }

  if (!numberSelectionValid || numberSelectionInput.empty()) {
    clearNumberSelectionInput();
    numberSelectionValid = false;
    numberSelectionPreview = "Invalid";
    numberSelectionStatus = "Invalid ID. Enter a new number.";
    return false;
  }

  Command cmd{Command::Type::SetAlgorithm, static_cast<uint8_t>(selectedTrack),
              0, 0.0f};
  switch (numberSelectionDomain) {
  case NumberSelectionDomain::Algorithm: {
    uint16_t displayId = 0;
    if (!parseDisplayId(numberSelectionInput, displayId) || displayId == 0) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Enter a new number.";
      return false;
    }
    if (selectedTrack == kMasterTrackIndex) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Master supports FX-only selection.";
      return false;
    }
    cmd.type = Command::Type::SetAlgorithm;
    cmd.trackIndex = static_cast<uint8_t>(selectedTrack);
    // Accept both built-in display IDs (1-20 → 0-19) and custom runtime IDs (2048+)
    if (displayId >= AlgorithmPresetRegistry::kCustomAlgorithmIdBase) {
      cmd.paramId = displayId; // Custom algorithm: use runtime ID directly
    } else {
      cmd.paramId = AlgorithmCatalog::displayIdToAlgorithmId(displayId);
    }
    break;
  }
  case NumberSelectionDomain::Sound: {
    uint16_t displayId = 0;
    if (!parseDisplayId(numberSelectionInput, displayId) || displayId == 0) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Enter a new number.";
      return false;
    }
    if (selectedTrack == kMasterTrackIndex) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Master supports FX-only selection.";
      return false;
    }
    cmd.type = Command::Type::SetSynthPreset;
    cmd.trackIndex = static_cast<uint8_t>(selectedTrack);
    cmd.paramId = SynthCatalog::displayIdToPresetId(displayId);
    break;
  }
  case NumberSelectionDomain::ChordProgression: {
    uint16_t displayId = 0;
    if (!parseDisplayId(numberSelectionInput, displayId) || displayId == 0) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid ID. Enter a new number.";
      return false;
    }
    cmd.type = Command::Type::SetChordProg;
    cmd.trackIndex = static_cast<uint8_t>(selectedTrack);
    cmd.paramId = static_cast<uint16_t>(
        ChordProgression::displayIdToProgressionIndex(displayId));
    break;
  }
  case NumberSelectionDomain::Genre: {
    uint16_t displayId = 0;
    if (!parseDisplayId(numberSelectionInput, displayId)) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Genre ID out of range (0-8). Enter a new number.";
      return false;
    }
    if (displayId == 0) {
      // 0 = no genre: trigger normal startup-style randomization (unfiltered).
      numberSelectionStatus = "Free randomization applied.";
      numberSelectionPreview = "Free Randomization";
      cmd.type = Command::Type::RandomizeForGenre;
      cmd.trackIndex = 0;
      cmd.paramId = 0;
      break;
    }
    if (!GenreCatalog::isValidGenreId(displayId)) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Genre ID out of range (1-8). Enter a new number.";
      return false;
    }
    cmd.type = Command::Type::RandomizeForGenre;
    cmd.trackIndex = 0;
    cmd.paramId = displayId; // 1-based genre ID
    break;
  }
  case NumberSelectionDomain::EffectPreset: {
    EffectSelectionInput parsed;
    if (!parseEffectSelectionInputImpl(numberSelectionInput, parsed)) {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid FX selection. Enter a new number.";
      return false;
    }

    uint16_t presetId = 0;
    uint8_t slotIndex = parsed.slotIndex;
    if (parsed.kind == EffectSelectionKind::ClearSlot) {
      presetId = 0;
    } else if (parsed.kind == EffectSelectionKind::Preset &&
               EffectPresetCatalog::isValidCategoryPresetDisplayId(
                   parsed.categoryDigit, parsed.presetDisplayId)) {
      presetId = EffectPresetCatalog::categoryPresetDisplayIdToPresetId(
          parsed.categoryDigit, parsed.presetDisplayId);
    } else {
      clearNumberSelectionInput();
      numberSelectionValid = false;
      numberSelectionPreview = "Invalid";
      numberSelectionStatus = "Invalid FX selection. Enter a new number.";
      return false;
    }
    cmd.type = (selectedTrack == kMasterTrackIndex)
                   ? Command::Type::SetMasterEffectPreset
                   : Command::Type::SetTrackEffectPreset;
    cmd.trackIndex = static_cast<uint8_t>(selectedTrack);
    cmd.paramId = Command::encodeEffectSlotPreset(slotIndex, presetId);
    break;
  }
  }

  // Build undo command and description
  std::string desc;
  Command undoCmd = cmd;
  if (cmd.type == Command::Type::SetAlgorithm) {
    uint8_t prev = appState.tracks[cmd.trackIndex].algorithmId.load(std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd.trackIndex + 1) + " Algorithm \u2192 " + std::to_string(cmd.paramId + 1);
    undoCmd = Command{Command::Type::SetAlgorithm, cmd.trackIndex, prev, 0.0f};
  } else if (cmd.type == Command::Type::SetSynthPreset) {
    uint8_t prev = appState.tracks[cmd.trackIndex].synthPreset.load(std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd.trackIndex + 1) + " Sound \u2192 " + std::to_string(cmd.paramId + 1);
    undoCmd = Command{Command::Type::SetSynthPreset, cmd.trackIndex, prev, 0.0f};
  } else if (cmd.type == Command::Type::SetChordProg) {
    uint8_t prev = appState.chordProgression.load(std::memory_order_relaxed);
    desc = "Chord Prog \u2192 " + std::to_string(cmd.paramId);
    undoCmd = Command{Command::Type::SetChordProg, 0, prev, 0.0f};
  } else if (cmd.type == Command::Type::SetTrackEffectPreset) {
    uint8_t slot = Command::decodeEffectSlotIndex(cmd.paramId);
    uint16_t prev = appState.tracks[cmd.trackIndex].getEffectPresetSlot(slot);
    desc = "Track " + std::to_string(cmd.trackIndex + 1) + " FX" + std::to_string(slot + 1) + " \u2192 " + std::to_string(Command::decodeEffectPresetId(cmd.paramId));
    undoCmd = Command{Command::Type::SetTrackEffectPreset, cmd.trackIndex, Command::encodeEffectSlotPreset(slot, prev), 0.0f};
  } else if (cmd.type == Command::Type::SetMasterEffectPreset) {
    uint8_t slot = Command::decodeEffectSlotIndex(cmd.paramId);
    uint16_t prev = appState.master.getEffectPresetSlot(slot);
    desc = "Master FX" + std::to_string(slot + 1) + " \u2192 " + std::to_string(Command::decodeEffectPresetId(cmd.paramId));
    undoCmd = Command{Command::Type::SetMasterEffectPreset, 0, Command::encodeEffectSlotPreset(slot, prev), 0.0f};
  } else {
    desc = "Action";
  }

  if (!dispatchAndLog(cmd, desc, undoCmd)) {
    numberSelectionValid = false;
    numberSelectionStatus = "Command queue full. Try again.";
    return false;
  }

  closeNumberSelection();
  return true;
}

void TuiApp::openKeySelection() {
  keySelectionOpen = true;
  keySelectionInput.clear();
  keySelectionPreview = ProjectKey::format(
      appState.projectKeyRoot.load(std::memory_order_relaxed),
      appState.projectKeyMode.load(std::memory_order_relaxed));
  keySelectionValid = true;
  keySelectionStatus = "Enter key like A, a, A#, bb, Db major, c#min";
}

void TuiApp::closeKeySelection() {
  keySelectionOpen = false;
  keySelectionInput.clear();
  keySelectionPreview.clear();
  keySelectionStatus.clear();
  keySelectionValid = true;
}

void TuiApp::refreshKeySelectionPreview() {
  if (keySelectionInput.empty()) {
    keySelectionValid = true;
    keySelectionPreview = ProjectKey::format(
        appState.projectKeyRoot.load(std::memory_order_relaxed),
        appState.projectKeyMode.load(std::memory_order_relaxed));
    keySelectionStatus = "Enter key like A, a, A#, bb, Db major, c#min";
    return;
  }

  ProjectKey::ParsedValue parsed;
  if (!ProjectKey::parse(keySelectionInput, parsed)) {
    keySelectionValid = false;
    keySelectionPreview = "Invalid";
    keySelectionStatus =
        "Invalid key format. Examples: A, a, A#, bb, Db major, c#min";
    return;
  }

  keySelectionValid = true;
  keySelectionPreview = ProjectKey::format(parsed.root, parsed.mode);
  keySelectionStatus = "Press Enter to apply.";
}

bool TuiApp::submitKeySelection() {
  if (!keySelectionOpen) {
    return false;
  }

  ProjectKey::ParsedValue parsed;
  if (!ProjectKey::parse(keySelectionInput, parsed)) {
    keySelectionValid = false;
    keySelectionPreview = "Invalid";
    keySelectionStatus = "Invalid key format. Enter a new key.";
    return false;
  }

  const uint16_t payload = Command::encodeProjectKey(parsed.root, parsed.mode);
  const Command command{Command::Type::SetProjectKey, 0, payload, 0.0f};
  uint8_t prevRoot = appState.projectKeyRoot.load(std::memory_order_relaxed);
  uint8_t prevMode = appState.projectKeyMode.load(std::memory_order_relaxed);
  Command undoCmd{Command::Type::SetProjectKey, 0, Command::encodeProjectKey(prevRoot, prevMode), 0.0f};
  std::string desc = "Set Key \u2192 " + ProjectKey::format(parsed.root, parsed.mode);
  if (!dispatchAndLog(command, desc, undoCmd)) {
    keySelectionValid = false;
    keySelectionStatus = "Command queue full. Try again.";
    return false;
  }

  closeKeySelection();
  return true;
}
