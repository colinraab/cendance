#include "TuiApp.h"
#include "../Audio/Harmony/ChordProgression.h"
#include "Components/ArrangementModal.h"
#include <string>
#include <array>
#include <numeric>
#include <sstream>
#include <algorithm>

namespace {

constexpr int kArrangementModalFieldCount = 7;

struct ArrangementPresetDefinition {
  std::string name;
  uint8_t sectionCount;
  std::array<uint8_t, AppState::kArrangementMaxSections> lengths;
  std::array<uint8_t, AppState::kArrangementMaxSections> masks;
  std::array<std::array<std::array<float, AppState::kArrangementTrackParameterCount>, AppState::kTrackCount>, AppState::kArrangementMaxSections> parameters;
};

ArrangementPresetDefinition makeArrangementPreset(const std::string &name,
                                                  uint8_t sectionCount,
                                                  std::array<uint8_t, AppState::kArrangementMaxSections> lengths,
                                                  std::array<uint8_t, AppState::kArrangementMaxSections> masks,
                                                  float base,
                                                  float rise) {
  ArrangementPresetDefinition preset{name, sectionCount, lengths, masks, {}};
  for (uint8_t section = 0; section < AppState::kArrangementMaxSections; ++section) {
    const float sectionLift = (sectionCount > 1 && section < sectionCount)
        ? (static_cast<float>(section) / static_cast<float>(sectionCount - 1)) * rise
        : 0.0f;
    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
      const float trackOffset = static_cast<float>(track) * 0.04f;
      preset.parameters[section][track][0] = std::clamp(base + sectionLift + trackOffset, 0.05f, 0.95f);
      preset.parameters[section][track][1] = std::clamp(base + sectionLift * 0.75f + trackOffset, 0.05f, 0.95f);
      preset.parameters[section][track][2] = std::clamp(0.45f + sectionLift * 0.5f + trackOffset, 0.05f, 0.95f);
      preset.parameters[section][track][3] = std::clamp(base + sectionLift + trackOffset * 0.5f, 0.05f, 0.95f);
    }
  }
  return preset;
}

const std::array<ArrangementPresetDefinition, 8> &arrangementPresets() {
  static const std::array<ArrangementPresetDefinition, 8> presets{{
      makeArrangementPreset("Four Part Rise", 4, {2, 4, 4, 8, 4, 4, 4, 4}, {0b0001, 0b0011, 0b0111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111}, 0.25f, 0.55f),
      makeArrangementPreset("Drop Builder", 5, {4, 4, 2, 8, 4, 4, 4, 4}, {0b0110, 0b1110, 0b1001, 0b1111, 0b1011, 0b1111, 0b1111, 0b1111}, 0.20f, 0.65f),
      makeArrangementPreset("Sparse Loop", 3, {8, 8, 8, 4, 4, 4, 4, 4}, {0b1011, 0b0111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111}, 0.20f, 0.35f),
      makeArrangementPreset("Verse Chorus", 4, {8, 8, 4, 8, 4, 4, 4, 4}, {0b0111, 0b1111, 0b0011, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111}, 0.30f, 0.40f),
      makeArrangementPreset("Intro Drop Outro", 5, {4, 4, 8, 4, 4, 4, 4, 4}, {0b0001, 0b0111, 0b1111, 0b1011, 0b0001, 0b1111, 0b1111, 0b1111}, 0.18f, 0.70f),
      makeArrangementPreset("A B Switch", 6, {4, 4, 4, 4, 4, 4, 4, 4}, {0b1011, 0b0111, 0b1011, 0b1111, 0b0111, 0b1111, 0b1111, 0b1111}, 0.35f, 0.25f),
      makeArrangementPreset("Long Arc", 8, {4, 4, 4, 4, 4, 4, 4, 8}, {0b0001, 0b0011, 0b0111, 0b1111, 0b1110, 0b1111, 0b1011, 0b1111}, 0.15f, 0.75f),
      makeArrangementPreset("Breakdown Return", 5, {8, 4, 4, 8, 4, 4, 4, 4}, {0b1111, 0b0100, 0b1100, 0b1111, 0b1011, 0b1111, 0b1111, 0b1111}, 0.28f, 0.50f),
  }};
  return presets;
}

std::string arrangementPresetLabel(uint8_t modalPresetIndex) {
  if (modalPresetIndex == 0) {
    return "None";
  }
  const auto &presets = arrangementPresets();
  const size_t presetIndex = static_cast<size_t>(modalPresetIndex - 1);
  return presetIndex < presets.size() ? presets[presetIndex].name : "None";
}

std::string trimCopy(const std::string &text) {
  const auto begin = text.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }

  const auto end = text.find_last_not_of(" \t\r\n");
  return text.substr(begin, (end - begin) + 1);
}

const char *arrangementModeLabel(uint8_t mode) {
  switch (mode) {
  case AppState::kArrangementModeManual:
    return "Manual";
  case AppState::kArrangementModeAuto:
    return "Auto";
  default:
    return "Mixed";
  }
}

std::string arrangementTrackMaskLabel(uint8_t mask) {
  std::string label;
  label.reserve(4);
  label.push_back((mask & 0x1u) ? 'D' : '-');
  label.push_back((mask & 0x2u) ? 'B' : '-');
  label.push_back((mask & 0x4u) ? 'C' : '-');
  label.push_back((mask & 0x8u) ? 'L' : '-');
  return label;
}

std::string arrangementChainLabel(
    bool chainEnabled, uint8_t sectionCount,
    const std::array<uint8_t, AppState::kArrangementMaxSections> &chainSequence,
    uint8_t chainLength) {
  if (!chainEnabled) {
    return "Linear";
  }

  const uint8_t clampedSectionCount =
      static_cast<uint8_t>(std::max<uint8_t>(sectionCount, 1));
  const uint8_t clampedChainLength =
      std::clamp<uint8_t>(chainLength, 1, AppState::kArrangementMaxSections);
  std::ostringstream label;
  bool wroteStep = false;
  for (uint8_t i = 0; i < clampedChainLength; ++i) {
    const uint8_t section = chainSequence[i];
    if (section >= clampedSectionCount) {
      continue;
    }

    if (wroteStep) {
      label << ">";
    }
    label << (static_cast<int>(section) + 1);
    wroteStep = true;
  }

  if (!wroteStep) {
    return "Linear (empty chain)";
  }

  return label.str();
}

std::string arrangementProgressionLabel(uint8_t progressionIndex) {
  const int progressionCount = ChordProgression::getNumProgressions();
  if (progressionCount <= 0) {
    return "Unavailable";
  }

  if (progressionIndex >= static_cast<uint8_t>(progressionCount)) {
    return "Invalid";
  }

  return std::string(ChordProgression::getNameByDisplayId(
      static_cast<uint16_t>(progressionIndex) + 1));
}

} // namespace

void TuiApp::openArrangementModal() {
  arrangementModalOpen = true;
  arrangementModalFocus = ArrangementModalFocus::Preset;
  arrangementModalSectionCount = static_cast<uint8_t>(std::clamp<uint8_t>(
      appState.arrangementSectionCount.load(std::memory_order_relaxed), 1,
      AppState::kArrangementMaxSections));
  arrangementModalCurrentSection = static_cast<uint8_t>(std::min<uint8_t>(
      appState.arrangementCurrentSection.load(std::memory_order_relaxed),
      static_cast<uint8_t>(arrangementModalSectionCount - 1)));

  for (uint8_t section = 0; section < AppState::kArrangementMaxSections;
       ++section) {
    arrangementModalSectionLengths[section] =
        appState.getArrangementSectionLength(section);
    arrangementModalSectionProgressions[section] =
        appState.getArrangementSectionProgression(section);
    arrangementModalSectionTrackMasks[section] =
        appState.getArrangementSectionTrackMask(section);
    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
      for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
        arrangementModalSectionTrackParameters[section][track][parameter] =
            appState.getArrangementSectionTrackParameter(section, track, parameter);
      }
    }
  }
  arrangementModalSectionParametersEnabled =
      appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed);
  arrangementModalParameterTrack = selectedTrack < AppState::kTrackCount
      ? static_cast<uint8_t>(selectedTrack)
      : 0;
  arrangementModalParameterIndex = 0;
  arrangementModalPresetIndex = 0;

  if (arrangementModalSectionParametersEnabled) {
    const auto &presets = arrangementPresets();
    for (size_t presetIndex = 0; presetIndex < presets.size(); ++presetIndex) {
      const auto &preset = presets[presetIndex];
      if (arrangementModalSectionCount == preset.sectionCount &&
          arrangementModalSectionLengths == preset.lengths &&
          arrangementModalSectionTrackMasks == preset.masks &&
          arrangementModalSectionTrackParameters == preset.parameters) {
        arrangementModalPresetIndex = static_cast<uint8_t>(presetIndex + 1);
        break;
      }
    }
  }

  arrangementModalStatus =
      "Tab/Up/Down focus. Left/Right adjust. Enter applies.";
  arrangementModalStatusIsError = false;
}

void TuiApp::closeArrangementModal() {
  arrangementModalOpen = false;
  arrangementModalStatus.clear();
  arrangementModalStatusIsError = false;
}

void TuiApp::cycleArrangementModalFocus(int delta) {
  int focus = static_cast<int>(arrangementModalFocus) + delta;
  while (focus < 0) {
    focus += kArrangementModalFieldCount;
  }
  focus %= kArrangementModalFieldCount;
  arrangementModalFocus = static_cast<ArrangementModalFocus>(focus);
  arrangementModalStatus = "Field focus moved.";
  arrangementModalStatusIsError = false;
}

void TuiApp::nudgeArrangementModalFocusedValue(int delta) {
  if (delta == 0) {
    return;
  }

  const int progressionCount = ChordProgression::getNumProgressions();
  const uint8_t section = arrangementModalCurrentSection;

  switch (arrangementModalFocus) {
  case ArrangementModalFocus::SectionCount: {
    const int next =
        std::clamp(static_cast<int>(arrangementModalSectionCount) + delta, 1,
                   static_cast<int>(AppState::kArrangementMaxSections));
    arrangementModalSectionCount = static_cast<uint8_t>(next);
    if (arrangementModalCurrentSection >= arrangementModalSectionCount) {
      arrangementModalCurrentSection =
          static_cast<uint8_t>(arrangementModalSectionCount - 1);
    }
    arrangementModalStatus = "Updated section count.";
    break;
  }
  case ArrangementModalFocus::CurrentSection: {
    int next = static_cast<int>(arrangementModalCurrentSection) + delta;
    if (next < 0) {
      next = static_cast<int>(arrangementModalSectionCount - 1);
    } else if (next >= arrangementModalSectionCount) {
      next = 0;
    }
    arrangementModalCurrentSection = static_cast<uint8_t>(next);
    arrangementModalStatus = "Selected section changed.";
    break;
  }
  case ArrangementModalFocus::SectionLength: {
    int next =
        static_cast<int>(arrangementModalSectionLengths[section]) + delta;
    if (next < AppState::kArrangementMinSectionLengthBars) {
      next = AppState::kArrangementMaxSectionLengthBars;
    } else if (next > AppState::kArrangementMaxSectionLengthBars) {
      next = AppState::kArrangementMinSectionLengthBars;
    }
    arrangementModalSectionLengths[section] = static_cast<uint8_t>(next);
    arrangementModalStatus = "Section length changed.";
    break;
  }
  case ArrangementModalFocus::ProgressionSource: {
    uint8_t &progression = arrangementModalSectionProgressions[section];
    if (progressionCount <= 0) {
      progression = AppState::kArrangementProgressionFollowGlobal;
      arrangementModalStatus = "No progression overrides are available.";
      arrangementModalStatusIsError = true;
      return;
    }

    if (progression == AppState::kArrangementProgressionFollowGlobal) {
      progression = delta > 0 ? 0 : static_cast<uint8_t>(progressionCount - 1);
    } else {
      int next = static_cast<int>(progression) + delta;
      if (next < 0) {
        next = progressionCount - 1;
      } else if (next >= progressionCount) {
        next = 0;
      }
      progression = static_cast<uint8_t>(next);
    }
    arrangementModalStatus = "Section progression source changed.";
    break;
  }
  case ArrangementModalFocus::TrackMask: {
    int next = static_cast<int>(arrangementModalSectionTrackMasks[section] &
                                AppState::kArrangementTrackMaskAll) +
               delta;
    while (next < 0) {
      next += 16;
    }
    next %= 16;
    arrangementModalSectionTrackMasks[section] = static_cast<uint8_t>(next);
    arrangementModalStatus = "Section track mask changed.";
    break;
  }
  case ArrangementModalFocus::TrackParameter: {
    float &value = arrangementModalSectionTrackParameters[section][arrangementModalParameterTrack][arrangementModalParameterIndex];
    value = std::clamp(value + (static_cast<float>(delta) * 0.05f), 0.0f, 1.0f);
    arrangementModalSectionParametersEnabled = true;
    arrangementModalStatus = "Section slider value changed.";
    break;
  }
  case ArrangementModalFocus::Preset: {
    int next = static_cast<int>(arrangementModalPresetIndex) + delta;
    const int presetChoiceCount = static_cast<int>(arrangementPresets().size()) + 1;
    while (next < 0) {
      next += presetChoiceCount;
    }
    next %= presetChoiceCount;
    arrangementModalPresetIndex = static_cast<uint8_t>(next);
    if (arrangementModalPresetIndex == 0) {
      arrangementModalSectionCount = 4;
      arrangementModalCurrentSection = 0;
      arrangementModalSectionLengths.fill(
          AppState::kArrangementDefaultSectionLengthBars);
      arrangementModalSectionProgressions.fill(
          AppState::kArrangementProgressionFollowGlobal);
      arrangementModalSectionTrackMasks.fill(
          AppState::kArrangementTrackMaskAll);
      arrangementModalSectionTrackParameters = {};
      arrangementModalSectionParametersEnabled = false;
      arrangementModalStatus = "Restored default arrangement.";
      break;
    }

    const auto &preset = arrangementPresets()[arrangementModalPresetIndex - 1];
    arrangementModalSectionCount = preset.sectionCount;
    arrangementModalCurrentSection = static_cast<uint8_t>(std::min<uint8_t>(arrangementModalCurrentSection, preset.sectionCount - 1));
    arrangementModalSectionLengths = preset.lengths;
    arrangementModalSectionTrackMasks = preset.masks;
    arrangementModalSectionTrackParameters = preset.parameters;
    arrangementModalSectionParametersEnabled = true;
    arrangementModalStatus = std::string("Loaded preset: ") + preset.name;
    break;
  }
  }

  arrangementModalStatusIsError = false;
}

bool TuiApp::submitArrangementModal() {
  if (!arrangementModalOpen) {
    return false;
  }

  const uint8_t sectionCount = static_cast<uint8_t>(std::clamp<uint8_t>(
      arrangementModalSectionCount, 1, AppState::kArrangementMaxSections));
  const uint8_t currentSection = static_cast<uint8_t>(std::min<uint8_t>(
      arrangementModalCurrentSection, static_cast<uint8_t>(sectionCount - 1)));
  const int progressionCount = ChordProgression::getNumProgressions();
  const uint8_t maxProgression =
      progressionCount > 0 ? static_cast<uint8_t>(progressionCount - 1) : 0;

  // Capture pre-change arrangement state for single undo entry
  struct ArrangementSnapshot {
    uint8_t sectionCount;
    uint8_t currentSection;
    uint8_t sectionLengths[AppState::kArrangementMaxSections];
    uint8_t sectionProgressions[AppState::kArrangementMaxSections];
    uint8_t sectionTrackMasks[AppState::kArrangementMaxSections];
  };
  ArrangementSnapshot prevSnap{};
  prevSnap.sectionCount = appState.arrangementSectionCount.load(std::memory_order_relaxed);
  prevSnap.currentSection = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
  for (uint8_t s = 0; s < AppState::kArrangementMaxSections; ++s) {
    prevSnap.sectionLengths[s] = appState.getArrangementSectionLength(s);
    prevSnap.sectionProgressions[s] = appState.getArrangementSectionProgression(s);
    prevSnap.sectionTrackMasks[s] = appState.getArrangementSectionTrackMask(s);
  }

  auto pushArrangementCommand = [&](const Command &command) {
    if (!cmdQueue.push(command)) {
      arrangementModalStatus = "Command queue full. Try again.";
      arrangementModalStatusIsError = true;
      return false;
    }
    return true;
  };

  if (!pushArrangementCommand(Command{Command::Type::SetArrangementSectionCount,
                                      0, sectionCount, 0.0f})) {
    return false;
  }

  if (!pushArrangementCommand(Command{Command::Type::SetArrangementSectionParametersEnabled,
                                      0,
                                      static_cast<uint16_t>(arrangementModalSectionParametersEnabled ? 1u : 0u),
                                      0.0f})) {
    return false;
  }

  for (uint8_t section = 0; section < AppState::kArrangementMaxSections;
       ++section) {
    const uint8_t bars = static_cast<uint8_t>(
        std::clamp<uint8_t>(arrangementModalSectionLengths[section],
                            AppState::kArrangementMinSectionLengthBars,
                            AppState::kArrangementMaxSectionLengthBars));
    const uint16_t lengthPayload =
        Command::encodeArrangementSectionValue(section, bars);
    if (!pushArrangementCommand(
            Command{Command::Type::SetArrangementSectionLength, 0,
                    lengthPayload, 0.0f})) {
      return false;
    }

    uint8_t progression = arrangementModalSectionProgressions[section];
    if (progression != AppState::kArrangementProgressionFollowGlobal) {
      progression =
          static_cast<uint8_t>(std::min<uint8_t>(progression, maxProgression));
    }
    const uint16_t progressionPayload =
        Command::encodeArrangementSectionValue(section, progression);
    if (!pushArrangementCommand(
            Command{Command::Type::SetArrangementSectionProgression, 0,
                    progressionPayload, 0.0f})) {
      return false;
    }

    const uint8_t trackMask =
        static_cast<uint8_t>(arrangementModalSectionTrackMasks[section] &
                             AppState::kArrangementTrackMaskAll);
    const uint16_t trackMaskPayload =
        Command::encodeArrangementSectionValue(section, trackMask);
    if (!pushArrangementCommand(
            Command{Command::Type::SetArrangementSectionTrackMask, 0,
                    trackMaskPayload, 0.0f})) {
      return false;
    }

    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
      for (uint8_t parameter = 0; parameter < AppState::kArrangementTrackParameterCount; ++parameter) {
        const uint8_t encodedTrack = static_cast<uint8_t>(track + parameter * AppState::kTrackCount);
        if (!pushArrangementCommand(
                Command{Command::Type::SetArrangementSectionTrackParameter,
                        encodedTrack,
                        section,
                        arrangementModalSectionTrackParameters[section][track][parameter]})) {
          return false;
        }
      }
    }
  }

  if (!pushArrangementCommand(Command{Command::Type::SetArrangementSection, 0,
                                      currentSection, 0.0f})) {
    return false;
  }

  // Log a single undo entry that restores the full previous arrangement state
  // by pushing all restore commands. We store the first restore cmd as the undo
  // and manually push the rest when undo fires — but since dispatchAndLog only
  // holds one undoCmd, we use SetArrangementSectionCount as the representative
  // and rely on re-opening the modal to fix details. Instead, store a
  // SetArrangementSectionCount undo and add a description noting the change.
  Command undoSectionCountCmd{Command::Type::SetArrangementSectionCount, 0,
                               prevSnap.sectionCount, 0.0f};
  std::string desc = "Arrangement updated (" + std::to_string(sectionCount) + " sections)";
  // Push the undo as restoring section count; other params require re-editing.
  // To give full fidelity, push all undo restore commands now as a batch into
  // a lambda-captured restore on undo — but since our UndoAction holds only one
  // Command, we store SetArrangementSectionCount as the primary undo signal and
  // log descriptively.
  undoStack.push_back({desc, undoSectionCountCmd});
  while (undoStack.size() > kMaxUndoActions) undoStack.pop_front();

  closeArrangementModal();
  setStatusMessage("Arrangement updated.", false);
  return true;
}
