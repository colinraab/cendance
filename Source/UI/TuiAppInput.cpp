#include "TuiApp.h"
#include "TuiAppInput.h"
#include "TuiAppProject.h"
#include "../App/KeyMapping.h"
#include "../App/SpotEffectCatalog.h"
#include "Components/NumberSelectionModal.h"
#include "Components/KeyEntryModal.h"
#include "Components/ProjectPathModal.h"
#include "Components/ArrangementModal.h"

#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <cctype>
#include <string>

using namespace tui_input;

bool TuiApp::handleEventInput(const ftxui::Event& event,
                              ftxui::ScreenInteractive& screen,
                              const MeterData& currentMeters) {
  using namespace ftxui;
  (void)currentMeters;

  if (onboardingTipsOpen) return handleOnboardingTipsInput(event);
  if (agentInputActive) return handleAgentInput(event, currentMeters);
  if (algorithmEditorOpen) return handleAlgorithmEditorInput(event);
  if (drumSampleModalOpen) return handleDrumSampleModalInput(event);
  if (soundFileBrowserOpen) return handleSoundFileBrowserInput(event);
  if (projectPathModalOpen) return handleProjectPathModalInput(event);
  if (arrangementModalOpen) return handleArrangementModalInput(event);
  if (grooveModalOpen) return handleGrooveModalInput(event);
  if (numberSelectionOpen) return handleNumberSelectionInput(event);
  if (keySelectionOpen) return handleKeySelectionInput(event);
  return handleMainInput(event, screen);
}

bool TuiApp::handleMainInput(const ftxui::Event& event,
                             ftxui::ScreenInteractive& screen) {
  using namespace ftxui;

if (event == Event::Character(":")) {
  agentInputActive = true;
  agentInput.clear();
  agentHistoryIndex = -1;
  agentStatus = "command mode";
  return true;
}

if (event == Event::Escape) {
  if (showHelp) {
    showHelp = false;
    return true;
  }
  screen.Exit();
  return true;
}
if (event == Event::Character("z") || event == Event::Character("Z") ||
    event == Event::Character("\x1a") ||
    event == Event::Character("\x1f")) {
  performUndo();
  return true;
}
if (event == Event::Character("\x13")) {
  quickSaveActiveProject();
  return true;
}
if (event.character() == "?" || event.character() == "h") {
  showHelp = !showHelp;
  if (!showHelp) {
    helpScrollOffset = 0;
  }
  return true;
}

if (event == Event::Tab) {
  selectedTrack = (selectedTrack + 1) % (kMasterTrackIndex + 1);
  return true;
}
if (event == Event::TabReverse) {
  selectedTrack =
      (selectedTrack + kMasterTrackIndex) % (kMasterTrackIndex + 1);
  return true;
}

// Scroll help popup with PageUp/PageDown/Arrows
if (showHelp) {
  if (event == Event::PageDown) {
    helpScrollOffset += 8;
    return true;
  } else if (event == Event::PageUp) {
    helpScrollOffset -= 8;
    return true;
  } else if (event == Event::ArrowDown) {
    helpScrollOffset += 1;
    return true;
  } else if (event == Event::ArrowUp) {
    helpScrollOffset -= 1;
    return true;
  }
}
if (event.character() == "1") {
  selectedTrack = 0;
  return true;
}
if (event.character() == "2") {
  selectedTrack = 1;
  return true;
}
if (event.character() == "3") {
  selectedTrack = 2;
  return true;
}
if (event.character() == "4") {
  selectedTrack = 3;
  return true;
}
if (event.character() == "5") {
  selectedTrack = 4;
  return true;
}

if (event.is_character()) {
  const char c = event.character()[0];
  if (c == 'a' || c == 'A') {
    if (selectedTrack != kMasterTrackIndex) {
      openNumberSelection(NumberSelectionDomain::Algorithm);
    }
    return true;
  }
  if (c == 's' || c == 'S') {
    if (selectedTrack != kMasterTrackIndex) {
      openNumberSelection(NumberSelectionDomain::Sound);
    }
    return true;
  }
  if (c == 'c' || c == 'C') {
    openNumberSelection(NumberSelectionDomain::ChordProgression);
    return true;
  }
  if (c == 'g' || c == 'G') {
    openNumberSelection(NumberSelectionDomain::Genre);
    return true;
  }
  if (c == 'q' || c == 'Q') {
    openGrooveModal();
    return true;
  }
  if (c == 'k' || c == 'K') {
    openKeySelection();
    return true;
  }
  if (c == 'f' || c == 'F') {
    openNumberSelection(NumberSelectionDomain::EffectPreset);
    return true;
  }
  if (c == 'r' || c == 'R') {
    openArrangementModal();
    return true;
  }
  if (c == 'w') {
    openProjectPathModal(ProjectPathModalMode::Save);
    return true;
  }
  if (c == 'W') {
    quickSaveActiveProject();
    return true;
  }
  if (c == 'l' || c == 'L') {
    openProjectPathModal(ProjectPathModalMode::Load);
    return true;
  }
  if (c == 'i' || c == 'I') {
    if (selectedTrack == 0) {
      openDrumSampleModal();
    } else {
      setStatusMessage("Select Drums track to open sample manager.", true);
    }
    return true;
  }
  if (c == 'd' || c == 'D') {
    openSoundFileBrowser();
    return true;
  }
  if (c == 'u' || c == 'U') {
    openAlgorithmEditor();
    return true;
  }
}

auto cmd = mapKeyToCommand(event, selectedTrack);
if (cmd) {
  if (selectedTrack == kMasterTrackIndex &&
      isMasterTrackOnlyIgnoredCommand(cmd->type)) {
    return true;
  }

  std::string desc;
  Command undoCmd =
      *cmd; // fallback, effectively no-op undo if not handled below
  bool canUndo = false;

  if (cmd->type == Command::Type::SetTempo) {
    float current = appState.bpm.load(std::memory_order_relaxed);
    desc = "Tempo " + std::string(cmd->value > 0 ? "+" : "") +
           std::to_string(static_cast<int>(cmd->value));
    undoCmd = Command{Command::Type::SetTempoAbsolute, 0, 0, current};
    canUndo = true;
  } else if (cmd->type == Command::Type::SetDensity &&
             cmd->trackIndex < 4) {
    float current = appState.tracks[cmd->trackIndex].density.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Density " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetDensityAbsolute, cmd->trackIndex, 0,
                      current};
    canUndo = true;
  } else if (cmd->type == Command::Type::SetComplexity &&
             cmd->trackIndex < 4) {
    float current = appState.tracks[cmd->trackIndex].complexity.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Complexity " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetComplexityAbsolute, cmd->trackIndex,
                      0, current};
    canUndo = true;
  } else if (cmd->type == Command::Type::SetTone && cmd->trackIndex < 4) {
    float current = appState.tracks[cmd->trackIndex].tone.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Tone " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetToneAbsolute, cmd->trackIndex, 0,
                      current};
    canUndo = true;
  } else if (cmd->type == Command::Type::SetMotion && cmd->trackIndex < 4) {
    float current = appState.tracks[cmd->trackIndex].motion.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Motion " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetMotionAbsolute, cmd->trackIndex, 0,
                      current};
    canUndo = true;
  } else if (cmd->type == Command::Type::SetTrackGain &&
             cmd->trackIndex <= kMasterTrackIndex) {
    float current = (cmd->trackIndex == kMasterTrackIndex)
        ? appState.master.gain.load(std::memory_order_relaxed)
        : appState.tracks[cmd->trackIndex].gain.load(std::memory_order_relaxed);
    desc = (cmd->trackIndex == kMasterTrackIndex
                ? std::string("Master")
                : "Track " + std::to_string(cmd->trackIndex + 1)) + " Gain " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetTrackGainAbsolute, cmd->trackIndex,
                      0, current};
    canUndo = true;
  } else if (cmd->type == Command::Type::ToggleTrackMute &&
             cmd->trackIndex < 4) {
    bool muted = appState.tracks[cmd->trackIndex].muted.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) +
           (muted ? " Unmute" : " Mute");
    undoCmd =
        Command{Command::Type::ToggleTrackMute, cmd->trackIndex, 0, 0.0f};
    canUndo = true;
  } else if (cmd->type == Command::Type::StepAlgorithm &&
             cmd->trackIndex < 4) {
    uint8_t current = appState.tracks[cmd->trackIndex].algorithmId.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Algorithm " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetAlgorithm, cmd->trackIndex, current,
                      0.0f};
    canUndo = true;
  } else if (cmd->type == Command::Type::StepSynthPreset &&
             cmd->trackIndex < 4) {
    uint8_t current = appState.tracks[cmd->trackIndex].synthPreset.load(
        std::memory_order_relaxed);
    desc = "Track " + std::to_string(cmd->trackIndex + 1) + " Sound " +
           std::string(cmd->value > 0 ? "+" : "-");
    undoCmd = Command{Command::Type::SetSynthPreset, cmd->trackIndex,
                      current, 0.0f};
    canUndo = true;
  } else if (cmd->type == Command::Type::StepArrangementSection) {
    uint8_t current =
        appState.arrangementCurrentSection.load(std::memory_order_relaxed);
    desc = "Arrangement Section " + std::string(cmd->value > 0 ? "+" : "-");
    undoCmd =
        Command{Command::Type::SetArrangementSection, 0, current, 0.0f};
    canUndo = true;
  } else if (cmd->type == Command::Type::StepArrangementMode) {
    uint8_t current =
        appState.arrangementMode.load(std::memory_order_relaxed);
    desc = "Arrangement Mode Cycled";
    undoCmd = Command{Command::Type::SetArrangementMode, 0, current, 0.0f};
    canUndo = true;
  }

  if (canUndo) {
    dispatchAndLog(*cmd, desc, undoCmd);
  } else {
    showSpotEffectPopup(*cmd);
    cmdQueue.push(*cmd);
  }
  return true;
}
return false;

}

void TuiApp::showSpotEffectPopup(const Command &cmd) {
  if (cmd.type != Command::Type::SpotEffectOn &&
      cmd.type != Command::Type::SpotEffectOff &&
      cmd.type != Command::Type::SpotEffectToggle) {
    return;
  }

  if (!SpotEffectCatalog::isValidSpotEffectId(cmd.paramId)) {
    return;
  }

  const auto effectId = static_cast<Command::SpotEffectId>(cmd.paramId);
  const uint8_t effectBit = SpotEffectCatalog::getBitMask(effectId);
  const uint8_t currentMask =
      appState.activeSpotEffects.load(std::memory_order_relaxed);
  uint8_t nextMask = currentMask;

  if (cmd.type == Command::Type::SpotEffectToggle) {
    nextMask = static_cast<uint8_t>(currentMask ^ effectBit);
  } else if (cmd.type == Command::Type::SpotEffectOn) {
    nextMask = static_cast<uint8_t>(currentMask | effectBit);
  } else {
    nextMask = static_cast<uint8_t>(currentMask & ~effectBit);
  }

  nextMask = static_cast<uint8_t>(nextMask & SpotEffectCatalog::getSupportedBitmask());
  if (nextMask == 0) {
    spotEffectPopupMessage.clear();
    return;
  }

  std::string message;
  for (const auto &definition : SpotEffectCatalog::kSpotEffects) {
    if ((nextMask & definition.bitMask) == 0) {
      continue;
    }
    if (!message.empty()) {
      message += " + ";
    }
    message += definition.name;
  }
  spotEffectPopupMessage = message + " ON";
}

//==============================================================================
// Groove modal methods
//==============================================================================

void TuiApp::openGrooveModal() {
  grooveModalOpen = true;
  grooveModalFocus = GrooveModalFocus::Swing;
  grooveModalStatus.clear();
  grooveModalStatusIsError = false;
}

void TuiApp::closeGrooveModal() {
  grooveModalOpen = false;
  grooveModalStatus.clear();
}

void TuiApp::cycleGrooveModalFocus(int delta) {
  int current = static_cast<int>(grooveModalFocus);
  current += delta;
  if (current < 0) current = 2;
  if (current > 2) current = 0;
  grooveModalFocus = static_cast<GrooveModalFocus>(current);
}

void TuiApp::nudgeGrooveModalValue(int delta) {
  const float step = 0.05f; // 5% per nudge
  float current = 0.0f;

  if (grooveModalFocus == GrooveModalFocus::Swing)
    current = appState.getSwingAmount();
  else if (grooveModalFocus == GrooveModalFocus::Velocity)
    current = appState.getVelocityHumanize();
  else
    current = appState.getTimingJitter();

  float newValue = std::clamp(current + delta * step, 0.0f, 1.0f);

  if (grooveModalFocus == GrooveModalFocus::Swing)
    appState.setSwingAmount(newValue);
  else if (grooveModalFocus == GrooveModalFocus::Velocity)
    appState.setVelocityHumanize(newValue);
  else
    appState.setTimingJitter(newValue);

  grooveModalStatus = "Set to " + std::to_string(static_cast<int>(newValue * 100)) + "%";
  grooveModalStatusIsError = false;
}
