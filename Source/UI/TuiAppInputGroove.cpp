#include "TuiApp.h"
#include "TuiAppInput.h"
#include <ftxui/component/event.hpp>
#include <cctype>
using namespace tui_input;

bool TuiApp::handleGrooveModalInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeGrooveModal();
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    closeGrooveModal();
    return true;
  }

  if (event == Event::Character("\t")) {
    cycleGrooveModalFocus(1);
    return true;
  }

  if (event == Event::ArrowUp) {
    cycleGrooveModalFocus(-1);
    return true;
  }

  if (event == Event::ArrowDown) {
    cycleGrooveModalFocus(1);
    return true;
  }

  if (event == Event::ArrowLeft) {
    nudgeGrooveModalValue(-1);
    return true;
  }

  if (event == Event::ArrowRight) {
    nudgeGrooveModalValue(1);
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1) {
      const char c = chars[0];

      if (c == '+' || c == '=') {
        nudgeGrooveModalValue(1);
        return true;
      }

      if (c == '-' || c == '_') {
        nudgeGrooveModalValue(-1);
        return true;
      }

      // Quick presets: 0=reset, 5=half, 9=max
      if (c == '0' || c == '5' || c == '9') {
        float val = 0.0f;
        if (c == '5') val = 0.5f;
        else if (c == '9') val = 1.0f;

        if (grooveModalFocus == GrooveModalFocus::Swing)
          appState.setSwingAmount(val);
        else if (grooveModalFocus == GrooveModalFocus::Velocity)
          appState.setVelocityHumanize(val);
        else
          appState.setTimingJitter(val);

        grooveModalStatus = "Set to " + std::to_string(static_cast<int>(val * 100)) + "%";
        grooveModalStatusIsError = false;
        return true;
      }
    }
  }

  return true;
}
