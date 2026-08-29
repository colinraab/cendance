#include "TuiApp.h"
#include "TuiAppInput.h"
#include "../Audio/Harmony/ChordProgression.h"
#include <ftxui/component/event.hpp>
#include <cctype>
#include <algorithm>
using namespace tui_input;

bool TuiApp::handleArrangementModalInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeArrangementModal();
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    submitArrangementModal();
    return true;
  }

  if (event == Event::Character("\t")) {
    cycleArrangementModalFocus(1);
    return true;
  }

  if (event == Event::ArrowUp) {
    cycleArrangementModalFocus(-1);
    return true;
  }

  if (event == Event::ArrowDown) {
    cycleArrangementModalFocus(1);
    return true;
  }

  if (event == Event::ArrowLeft) {
    nudgeArrangementModalFocusedValue(-1);
    return true;
  }

  if (event == Event::ArrowRight) {
    nudgeArrangementModalFocusedValue(1);
    return true;
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (arrangementModalFocus == ArrangementModalFocus::ProgressionSource) {
      arrangementModalSectionProgressions[arrangementModalCurrentSection] =
          AppState::kArrangementProgressionFollowGlobal;
      arrangementModalStatus =
          "Section progression follows global progression.";
      arrangementModalStatusIsError = false;
    } else if (arrangementModalFocus == ArrangementModalFocus::TrackMask) {
      arrangementModalSectionTrackMasks[arrangementModalCurrentSection] =
          AppState::kArrangementTrackMaskAll;
      arrangementModalStatus = "Track mask reset to all tracks.";
      arrangementModalStatusIsError = false;
    }
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1) {
      const char c = chars[0];

      if (c == '+' || c == '=') {
        nudgeArrangementModalFocusedValue(1);
        return true;
      }

      if (c == '-' || c == '_') {
        nudgeArrangementModalFocusedValue(-1);
        return true;
      }

      if (c == 't' || c == 'T') {
        arrangementModalParameterTrack = static_cast<uint8_t>((arrangementModalParameterTrack + 1u) % AppState::kTrackCount);
        arrangementModalStatus = "Section slider track changed.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (c == 'p' || c == 'P') {
        arrangementModalParameterIndex = static_cast<uint8_t>((arrangementModalParameterIndex + 1u) % AppState::kArrangementTrackParameterCount);
        arrangementModalStatus = "Section slider parameter changed.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (c == 'x' || c == 'X') {
        arrangementModalSectionParametersEnabled = !arrangementModalSectionParametersEnabled;
        arrangementModalStatus = arrangementModalSectionParametersEnabled
            ? "Section slider automation enabled."
            : "Section slider automation disabled.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (c == 'c' || c == 'C') {
        const uint8_t section = arrangementModalCurrentSection;
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
          arrangementModalSectionTrackParameters[section][track][0] =
              appState.tracks[track].density.load(std::memory_order_relaxed);
          arrangementModalSectionTrackParameters[section][track][1] =
              appState.tracks[track].complexity.load(std::memory_order_relaxed);
          arrangementModalSectionTrackParameters[section][track][2] =
              appState.tracks[track].tone.load(std::memory_order_relaxed);
          arrangementModalSectionTrackParameters[section][track][3] =
              appState.tracks[track].motion.load(std::memory_order_relaxed);
        }
        arrangementModalSectionParametersEnabled = true;
        arrangementModalStatus = "Captured current sliders for this section.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (arrangementModalFocus ==
              ArrangementModalFocus::ProgressionSource &&
          (c == 'g' || c == 'G')) {
        arrangementModalSectionProgressions
            [arrangementModalCurrentSection] =
                AppState::kArrangementProgressionFollowGlobal;
        arrangementModalStatus =
            "Section progression follows global progression.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (arrangementModalFocus == ArrangementModalFocus::TrackMask) {
        if (c >= '1' && c <= '4') {
          const uint8_t bit =
              static_cast<uint8_t>(1u << static_cast<uint8_t>(c - '1'));
          arrangementModalSectionTrackMasks
              [arrangementModalCurrentSection] = static_cast<uint8_t>(
                  arrangementModalSectionTrackMasks
                      [arrangementModalCurrentSection] ^
                  bit);
          arrangementModalStatus = "Toggled track mask bit.";
          arrangementModalStatusIsError = false;
          return true;
        }
        if (c == 'a' || c == 'A') {
          arrangementModalSectionTrackMasks
              [arrangementModalCurrentSection] =
                  AppState::kArrangementTrackMaskAll;
          arrangementModalStatus = "Track mask set to all tracks.";
          arrangementModalStatusIsError = false;
          return true;
        }
        if (c == '0') {
          arrangementModalSectionTrackMasks
              [arrangementModalCurrentSection] = 0;
          arrangementModalStatus = "Track mask cleared.";
          arrangementModalStatusIsError = false;
          return true;
        }
      }

      if (arrangementModalFocus == ArrangementModalFocus::SectionCount &&
          c >= '1' && c <= '8') {
        arrangementModalSectionCount = static_cast<uint8_t>(c - '0');
        if (arrangementModalCurrentSection >=
            arrangementModalSectionCount) {
          arrangementModalCurrentSection =
              static_cast<uint8_t>(arrangementModalSectionCount - 1);
        }
        arrangementModalStatus = "Updated section count.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (arrangementModalFocus == ArrangementModalFocus::CurrentSection &&
          c >= '1' && c <= '8') {
        const uint8_t nextSection = static_cast<uint8_t>(c - '1');
        arrangementModalCurrentSection =
            static_cast<uint8_t>(std::min<uint8_t>(
                nextSection,
                static_cast<uint8_t>(arrangementModalSectionCount - 1)));
        arrangementModalStatus = "Selected section changed.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (arrangementModalFocus == ArrangementModalFocus::SectionLength &&
          c >= '1' && c <= '9') {
        arrangementModalSectionLengths[arrangementModalCurrentSection] =
            static_cast<uint8_t>(c - '0');
        arrangementModalStatus = "Section length changed.";
        arrangementModalStatusIsError = false;
        return true;
      }

      if (arrangementModalFocus ==
              ArrangementModalFocus::ProgressionSource &&
          std::isdigit(static_cast<unsigned char>(c)) && c != '0') {
        const int progressionCount = ChordProgression::getNumProgressions();
        if (progressionCount <= 0) {
          arrangementModalSectionProgressions
              [arrangementModalCurrentSection] =
                  AppState::kArrangementProgressionFollowGlobal;
          arrangementModalStatus =
              "No progression overrides are available.";
          arrangementModalStatusIsError = true;
          return true;
        }

        const uint8_t displayId = static_cast<uint8_t>(c - '0');
        const uint8_t clampedDisplayId =
            static_cast<uint8_t>(std::min<uint8_t>(
                displayId, static_cast<uint8_t>(progressionCount)));
        arrangementModalSectionProgressions
            [arrangementModalCurrentSection] =
                static_cast<uint8_t>(clampedDisplayId - 1);
        arrangementModalStatus = "Section progression source changed.";
        arrangementModalStatusIsError = false;
        return true;
      }
    }
  }

  return true;
}
