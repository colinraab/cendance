#include "TuiApp.h"
#include "TuiAppInput.h"

#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <cctype>
#include <algorithm>

using namespace tui_input;

bool TuiApp::handleOnboardingTipsInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape || event == Event::Return ||
      event == Event::Character("\n") || event == Event::Character(" ")) {
    onboardingTipsOpen = false;
    return true;
  }

  return true;
}

bool TuiApp::handleAgentInput(const ftxui::Event& event, const MeterData& currentMeters) {
  using namespace ftxui;

  if (event == Event::Escape) {
    agentInputActive = false;
    agentInput.clear();
    agentHistoryIndex = -1;
    agentStatus = "ready";
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    const std::string submitted = tui_input::trimCopy(agentInput);
    agentInputActive = false;
    agentInput.clear();
    agentHistoryIndex = -1;
    if (submitted.empty()) {
      agentStatus = "ready";
      return true;
    }
    agentInputHistory.push_back(submitted);
    while (agentInputHistory.size() > 50) {
      agentInputHistory.pop_front();
    }
    const auto response = executeAgentCommand(submitted, currentMeters);
    agentStatus = response.ok ? response.message : ("error: " + response.message);
    setStatusMessage(agentStatus, !response.ok);
    return true;
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (!agentInput.empty()) {
      agentInput.pop_back();
    }
    return true;
  }

  if (event == Event::ArrowUp) {
    if (!agentInputHistory.empty()) {
      if (agentHistoryIndex < 0) {
        agentHistoryIndex = static_cast<int>(agentInputHistory.size()) - 1;
      } else {
        agentHistoryIndex = std::max(0, agentHistoryIndex - 1);
      }
      agentInput = agentInputHistory[static_cast<size_t>(agentHistoryIndex)];
    }
    return true;
  }

  if (event == Event::ArrowDown) {
    if (agentHistoryIndex >= 0 &&
        agentHistoryIndex + 1 < static_cast<int>(agentInputHistory.size())) {
      ++agentHistoryIndex;
      agentInput = agentInputHistory[static_cast<size_t>(agentHistoryIndex)];
    } else {
      agentHistoryIndex = -1;
      agentInput.clear();
    }
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1 &&
        std::isprint(static_cast<unsigned char>(chars[0])) &&
        agentInput.size() < tui_input::kMaxAgentInputChars) {
      agentInput.push_back(chars[0]);
    }
    return true;
  }

  return true;
}

bool TuiApp::handleNumberSelectionInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeNumberSelection();
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    submitNumberSelection();
    return true;
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (!numberSelectionInput.empty()) {
      numberSelectionInput.pop_back();
    }
    refreshNumberSelectionPreview();
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1) {
      const char c = chars[0];
      if (std::isdigit(static_cast<unsigned char>(c)) &&
          numberSelectionInput.size() < tui_input::kMaxNumberSelectionDigits) {
        numberSelectionInput.push_back(c);
      } else if (numberSelectionDomain == NumberSelectionDomain::EffectPreset &&
                 c == '-' && numberSelectionInput.size() == 1) {
        numberSelectionInput.push_back(c);
      }
      refreshNumberSelectionPreview();
    }
    return true;
  }

  return true;
}

bool TuiApp::handleKeySelectionInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeKeySelection();
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    submitKeySelection();
    return true;
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (!keySelectionInput.empty()) {
      keySelectionInput.pop_back();
    }
    refreshKeySelectionPreview();
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1 &&
        std::isprint(static_cast<unsigned char>(chars[0]))) {
      if (keySelectionInput.size() < tui_input::kMaxKeySelectionChars) {
        keySelectionInput.push_back(chars[0]);
      }
    }
    refreshKeySelectionPreview();
    return true;
  }

  return true;
}
