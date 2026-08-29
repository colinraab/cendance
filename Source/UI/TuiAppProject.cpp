#include "TuiApp.h"
#include "TuiAppProject.h"
#include "../App/ProjectIO.h"
#include "../App/ProjectIOLoad.h"
#include "../App/ProjectKey.h"
#include "Components/ProjectPathModal.h"
#include <filesystem>
#include <string>
#include <utility>

namespace {

std::string trimCopy(const std::string &text) {
  const auto begin = text.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }

  const auto end = text.find_last_not_of(" \t\r\n");
  return text.substr(begin, (end - begin) + 1);
}

} // namespace

std::string stripCendanceExtension(const std::string &name) {
  const std::string suffix = ".cendance";
  if (name.size() >= suffix.size() &&
      name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
    return name.substr(0, name.size() - suffix.size());
  }
  return name;
}

void TuiApp::loadRecentProjects() {
  std::string error;
  recentProjectPaths = ProjectIO::loadRecentProjectPaths(error);
  if (!error.empty()) {
    setStatusMessage("Recent project history unavailable.", true);
  }

  if (!activeProjectPath.empty()) {
    ProjectIO::touchRecentProjectPath(recentProjectPaths, activeProjectPath);
  }
}

void TuiApp::persistRecentProjects() {
  std::string error;
  if (!ProjectIO::saveRecentProjectPaths(recentProjectPaths, error)) {
    setStatusMessage("Failed to persist recent projects.", true);
  }
}

void TuiApp::setStatusMessage(const std::string &message, bool isError) {
  uiStatusMessage = message;
  uiStatusIsError = isError;
}

bool TuiApp::quickSaveActiveProject() {
  if (activeProjectPath.empty()) {
    openProjectPathModal(ProjectPathModalMode::Save);
    projectPathStatus =
        "No active project path yet. Enter directory/name, then press Enter.";
    projectPathStatusIsError = false;
    return false;
  }

  return saveProjectToPath(activeProjectPath);
}

void TuiApp::openProjectPathModal(ProjectPathModalMode mode) {
  projectPathModalOpen = true;
  projectPathModalMode = mode;
  if (mode == ProjectPathModalMode::Save) {
    const std::filesystem::path activePath(activeProjectPath);
    projectDirectoryInput = activeProjectPath.empty()
                                ? ProjectIO::getDefaultProjectsDirectory()
                                : activePath.parent_path().string();
    projectNameInput =
        activeProjectPath.empty()
            ? "untitled"
            : stripCendanceExtension(activePath.filename().string());
    saveProjectFieldFocus = SaveProjectFieldFocus::Name;
  } else {
    projectPathInput = activeProjectPath;
  }
  projectPathStatusIsError = false;
  projectPathStatus =
      (mode == ProjectPathModalMode::Save)
          ? "Edit directory/name. Extension .cendance is added automatically."
          : "Enter a path or select a recent project.";

  if (mode == ProjectPathModalMode::Load && !recentProjectPaths.empty()) {
    selectedRecentProjectIndex = 0;
    if (projectPathInput.empty()) {
      projectPathInput = recentProjectPaths[0];
    }
  }
}

void TuiApp::closeProjectPathModal() {
  projectPathModalOpen = false;
  projectPathInput.clear();
  projectDirectoryInput.clear();
  projectNameInput.clear();
  projectPathStatus.clear();
  projectPathStatusIsError = false;
}

void TuiApp::selectRecentProject(int delta) {
  if (projectPathModalMode != ProjectPathModalMode::Load ||
      recentProjectPaths.empty()) {
    return;
  }

  selectedRecentProjectIndex += delta;
  if (selectedRecentProjectIndex < 0) {
    selectedRecentProjectIndex =
        static_cast<int>(recentProjectPaths.size()) - 1;
  } else if (selectedRecentProjectIndex >=
             static_cast<int>(recentProjectPaths.size())) {
    selectedRecentProjectIndex = 0;
  }

  projectPathInput =
      recentProjectPaths[static_cast<size_t>(selectedRecentProjectIndex)];
}

bool TuiApp::saveProjectToPath(const std::string &path) {
  ProjectIO::ProjectSnapshot snapshot = ProjectIO::snapshotFromState(appState);
  snapshot.projectName = std::filesystem::path(path).stem().string();

  std::string error;
  if (!ProjectIO::saveProjectFile(snapshot, path, error)) {
    setStatusMessage("Save failed: " + error, true);
    return false;
  }

  activeProjectPath = path;
  ProjectIO::touchRecentProjectPath(recentProjectPaths, path);
  persistRecentProjects();
  setStatusMessage("Saved project: " + path, false);
  return true;
}

bool TuiApp::loadProjectFromPath(const std::string &path) {
  ProjectIO::ProjectSnapshot snapshot;
  std::string error;
  if (!ProjectIO::loadProjectFile(path, snapshot, error)) {
    setStatusMessage("Load failed: " + error, true);
    return false;
  }

  if (!ProjectIO::applySnapshotToCommandQueue(snapshot, appState, cmdQueue,
                                              error, true)) {
    setStatusMessage("Load apply failed: " + error, true);
    return false;
  }

  activeProjectPath = path;
  ProjectIO::touchRecentProjectPath(recentProjectPaths, path);
  persistRecentProjects();

  int missingSamples = 0;
  if (drumSampleLibrary != nullptr) {
    for (uint8_t slot = 0; slot < AppState::TrackState::DrumSampleSlotCount;
         ++slot) {
      const uint16_t sampleId =
          snapshot.tracks[0].drumSampleSlots[slot].sampleId;
      if (sampleId != 0 && !drumSampleLibrary->hasSample(sampleId)) {
        ++missingSamples;
      }
    }
  }

  if (missingSamples > 0) {
    setStatusMessage("Loaded project: " + path + " (" +
                         std::to_string(missingSamples) +
                         " drum samples missing; synth fallback active)",
                     true);
  } else {
    setStatusMessage("Loaded project: " + path + " (stopped)", false);
  }
  return true;
}

bool TuiApp::submitProjectPathModal() {
  std::string normalizedPath;
  std::string error;

  bool success = false;
  if (projectPathModalMode == ProjectPathModalMode::Save) {
    const std::string directory = trimCopy(projectDirectoryInput).empty()
                                      ? ProjectIO::getDefaultProjectsDirectory()
                                      : trimCopy(projectDirectoryInput);
    std::string name = stripCendanceExtension(trimCopy(projectNameInput));

    if (name.empty()) {
      projectPathStatus = "Project name is required.";
      projectPathStatusIsError = true;
      return false;
    }
    if (name.find('/') != std::string::npos ||
        name.find('\\') != std::string::npos) {
      projectPathStatus = "Project name cannot contain path separators.";
      projectPathStatusIsError = true;
      return false;
    }

    const std::filesystem::path rawPath =
        std::filesystem::path(directory) / name;
    if (!ProjectIO::normalizeProjectPath(rawPath.string(), normalizedPath,
                                         error, true)) {
      projectPathStatus = error;
      projectPathStatusIsError = true;
      return false;
    }

    success = saveProjectToPath(normalizedPath);
  } else {
    if (!ProjectIO::normalizeProjectPath(projectPathInput, normalizedPath,
                                         error, true)) {
      projectPathStatus = error;
      projectPathStatusIsError = true;
      return false;
    }

    success = loadProjectFromPath(normalizedPath);
  }

  if (success) {
    closeProjectPathModal();
  } else {
    projectPathStatus = uiStatusMessage;
    projectPathStatusIsError = true;
  }

  return success;
}
