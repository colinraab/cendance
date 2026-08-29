#include "TuiApp.h"
#include "TuiAppInput.h"
#include "TuiAppProject.h"
#include <ftxui/component/event.hpp>
#include <cctype>
using namespace tui_input;

bool TuiApp::handleProjectPathModalInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeProjectPathModal();
    return true;
  }

  if (projectPathModalMode == ProjectPathModalMode::Save) {
    if (event == Event::Character("\t")) {
      saveProjectFieldFocus =
          (saveProjectFieldFocus == SaveProjectFieldFocus::Directory)
              ? SaveProjectFieldFocus::Name
              : SaveProjectFieldFocus::Directory;
      return true;
    }
    if (event == Event::ArrowUp) {
      saveProjectFieldFocus = SaveProjectFieldFocus::Name;
      return true;
    }
    if (event == Event::ArrowDown) {
      saveProjectFieldFocus = SaveProjectFieldFocus::Directory;
      return true;
    }
  } else {
    if (event == Event::ArrowUp) {
      selectRecentProject(-1);
      return true;
    }

    if (event == Event::ArrowDown) {
      selectRecentProject(1);
      return true;
    }
  }

  if (event == Event::Backspace || event == Event::Character("\x7f")) {
    if (projectPathModalMode == ProjectPathModalMode::Save) {
      std::string &target =
          (saveProjectFieldFocus == SaveProjectFieldFocus::Directory)
              ? projectDirectoryInput
              : projectNameInput;
      if (!target.empty()) {
        target.pop_back();
      }
    } else if (!projectPathInput.empty()) {
      projectPathInput.pop_back();
    }
    projectPathStatus.clear();
    projectPathStatusIsError = false;
    return true;
  }

  if (event == Event::Return || event == Event::Character("\n")) {
    if (projectPathModalMode == ProjectPathModalMode::Save) {
      if (saveProjectFieldFocus == SaveProjectFieldFocus::Directory) {
        saveProjectFieldFocus = SaveProjectFieldFocus::Name;
        projectPathStatus =
            "Project name is required. Type a name, then press Enter.";
        projectPathStatusIsError = true;
        return true;
      }

      const std::string name =
          stripCendanceExtension(trimCopy(projectNameInput));
      if (name.empty()) {
        saveProjectFieldFocus = SaveProjectFieldFocus::Name;
        projectPathStatus =
            "Project name is required. Type a name, then press Enter.";
        projectPathStatusIsError = true;
        return true;
      }
    }

    submitProjectPathModal();
    return true;
  }

  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1 &&
        std::isprint(static_cast<unsigned char>(chars[0]))) {
      if (projectPathModalMode == ProjectPathModalMode::Save) {
        std::string &target =
            (saveProjectFieldFocus == SaveProjectFieldFocus::Directory)
                ? projectDirectoryInput
                : projectNameInput;
        target.push_back(chars[0]);
      } else {
        projectPathInput.push_back(chars[0]);
      }
      projectPathStatus.clear();
      projectPathStatusIsError = false;
    }
    return true;
  }

  return true;
}
