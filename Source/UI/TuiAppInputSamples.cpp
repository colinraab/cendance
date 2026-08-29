#include "TuiApp.h"
#include "TuiAppInput.h"
#include <ftxui/component/event.hpp>
#include <cctype>
#include <algorithm>
using namespace tui_input;

bool TuiApp::handleDrumSampleModalInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeDrumSampleModal();
    return true;
  }

  if (event == Event::Character("\t")) {
    drumSampleModalFocus =
        (drumSampleModalFocus == DrumSampleModalFocus::Path)
            ? DrumSampleModalFocus::List
            : DrumSampleModalFocus::Path;
    return true;
  }

  if (event == Event::ArrowUp &&
      drumSampleModalFocus == DrumSampleModalFocus::List &&
      !drumSampleEntries.empty()) {
    selectedDrumSampleIndex = std::max(0, selectedDrumSampleIndex - 1);
    return true;
  }

  if (event == Event::ArrowDown &&
      drumSampleModalFocus == DrumSampleModalFocus::List &&
      !drumSampleEntries.empty()) {
    selectedDrumSampleIndex =
        std::min(static_cast<int>(drumSampleEntries.size() - 1),
                 selectedDrumSampleIndex + 1);
    return true;
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (drumSampleModalFocus == DrumSampleModalFocus::Path &&
        !drumSamplePathInput.empty()) {
      drumSamplePathInput.pop_back();
    }
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    if (drumSampleModalFocus == DrumSampleModalFocus::Path) {
      importAndAssignDrumSample();
    } else {
      assignSelectedDrumSample();
    }
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1) {
      const char c = chars[0];

      if (drumSampleModalFocus == DrumSampleModalFocus::Path) {
        if (std::isprint(static_cast<unsigned char>(c))) {
          drumSamplePathInput.push_back(c);
        }
        return true;
      }

      if (c >= '1' && c <= '4') {
        selectedDrumSlot = static_cast<uint8_t>(c - '1');
        return true;
      }

      switch (c) {
      case 'a':
      case 'A':
        assignSelectedDrumSample();
        return true;
      case 'c':
      case 'C':
        clearSelectedDrumSample();
        return true;
      case 'r':
      case 'R': {
        if (drumSampleLibrary == nullptr) {
          drumSampleStatus = "Drum sample library unavailable.";
          drumSampleStatusIsError = true;
        } else {
          std::string error;
          if (!drumSampleLibrary->rescanGlobalDirectory(error)) {
            drumSampleStatus = "Rescan failed: " + error;
            drumSampleStatusIsError = true;
          } else {
            refreshDrumSampleEntries();
            drumSampleStatus = "Rescanned drum sample library.";
            drumSampleStatusIsError = false;
          }
        }
        return true;
      }
      case 'q':
      case 'Q':
        nudgeDrumSampleParam(Command::Type::SetDrumSampleVolume, 0.05f);
        return true;
      case 'f':
      case 'F':
        nudgeDrumSampleParam(Command::Type::SetDrumSampleVolume, -0.05f);
        return true;
      default:
        break;
      }

      if (c == 'w' || c == 'W') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleTune, 1.0f);
        return true;
      }
      if (c == 's' || c == 'S') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleTune, -1.0f);
        return true;
      }
      if (c == 'e' || c == 'E') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleStartOffset,
                             0.01f);
        return true;
      }
      if (c == 'd' || c == 'D') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleStartOffset,
                             -0.01f);
        return true;
      }
      if (c == 't' || c == 'T') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleDecay, 0.05f);
        return true;
      }
      if (c == 'g' || c == 'G') {
        nudgeDrumSampleParam(Command::Type::SetDrumSampleDecay, -0.05f);
        return true;
      }
      if (c == 'y' || c == 'Y') {
        nudgeDrumSampleParam(
            Command::Type::SetDrumSampleVelocitySensitivity, 0.05f);
        return true;
      }
      if (c == 'h' || c == 'H') {
        nudgeDrumSampleParam(
            Command::Type::SetDrumSampleVelocitySensitivity, -0.05f);
        return true;
      }
    }
  }

  return true;
}

bool TuiApp::handleSoundFileBrowserInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeSoundFileBrowser();
    return true;
  }
  if (event == Event::ArrowUp && !downloadedSampleEntries.empty()) {
    selectedDownloadedSampleIndex = std::max(0, selectedDownloadedSampleIndex - 1);
    return true;
  }
  if (event == Event::ArrowDown && !downloadedSampleEntries.empty()) {
    selectedDownloadedSampleIndex =
        std::min(static_cast<int>(downloadedSampleEntries.size() - 1),
                 selectedDownloadedSampleIndex + 1);
    return true;
  }
  if (event == Event::Return || event == Event::Character("\n")) {
    importSelectedDownloadedSample();
    return true;
  }
  if (event.is_character()) {
    const char c = event.character()[0];
    if (c >= '1' && c <= '4') {
      selectedDrumSlot = static_cast<uint8_t>(c - '1');
      return true;
    }
    if (c == 'r' || c == 'R') {
      refreshDownloadedSampleEntries();
      soundFileBrowserStatus = "Refreshed downloaded sample list.";
      soundFileBrowserStatusIsError = false;
      return true;
    }
  }
  return true;
}
