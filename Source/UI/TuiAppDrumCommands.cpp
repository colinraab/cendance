#include "TuiApp.h"
#include "AgentProtocolServer.h"
#include "../App/AgentCommand.h"
#include "../App/ContributionPackage.h"
#include "../App/DrumSampleCatalog.h"
#include "../Mcp/P2PToolHandler.h"
#include "../Security/SecurityManager.h"

#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <deque>
#include <array>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <filesystem>

namespace {

std::string trimCopy(const std::string &text) {
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

} // namespace

bool TuiApp::dispatchAndLog(const Command &cmd, const std::string &description,
                            const Command &undoCmd) {
  if (cmdQueue.push(cmd)) {
    if (!description.empty()) {
      undoStack.push_back({description, undoCmd});
      while (undoStack.size() > kMaxUndoActions) {
        undoStack.pop_front();
      }
    }
    return true;
  } else {
    setStatusMessage("Command queue full. Try again.", true);
    return false;
  }
}

void TuiApp::performUndo() {
  if (undoStack.empty()) {
    setStatusMessage("Nothing to undo.", true);
    return;
  }
  const auto &action = undoStack.back();
  if (cmdQueue.push(action.undoCommand)) {
    setStatusMessage("Undid: " + action.description, false);
    undoStack.pop_back();
  } else {
    setStatusMessage("Command queue full. Try again.", true);
  }
}

AgentCommand::Response
TuiApp::executeAgentCommand(const std::string &input,
                            const MeterData &currentMeters) {
  AgentCommand::ExecutionContext context{
      appState,
      &cmdQueue,
      currentMeters,
      agentMeterHistory,
      [this](const Command &command, const std::string &description,
             const Command &undoCommand) {
        return dispatchAndLog(command, description, undoCommand);
      },
      contributionLibrary,
      // P2P bridge: routes p2p.* commands to P2PClient via P2PToolHandler
      [this](const std::string &toolName,
             const std::string &argsJson) -> std::string {
        if (!p2pClient || !presetSerializer)
          return "{\"ok\":false,\"message\":\"P2P not available\"}";
        static SecurityManager sm;
        static bool smInit = false;
        if (!smInit) { sm.initialize(); smInit = true; }
        P2PToolHandler handler(appState, sm, *presetSerializer, *p2pClient);
        return handler.handle(juce::String(toolName), juce::String(argsJson)).toStdString();
      },
      // Recording bridge: routes record.* commands to JuceRuntime
      [this](const std::string &action,
             const std::string &argsJson) -> std::string {
        if (!recordFn)
          return "{\"ok\":false,\"message\":\"Recording not available\"}";
        return recordFn(action, argsJson);
      },
      // Streaming bridge: routes stream.* commands to JuceRuntime
      [this](const std::string &action,
             const std::string &argsJson) -> std::string {
        if (!streamFn)
          return "{\"ok\":false,\"message\":\"Streaming not available\"}";
        return streamFn(action, argsJson);
      }};
  return AgentCommand::execute(input, context);
}

std::string TuiApp::enqueueAgentProtocolCommand(const std::string &input) {
  auto request = std::make_shared<PendingAgentProtocolRequest>();
  request->input = input;
  {
    std::lock_guard<std::mutex> lock(pendingAgentRequestsMutex);
    pendingAgentRequests.push_back(request);
  }

  std::unique_lock<std::mutex> requestLock(request->mutex);
  if (!request->cv.wait_for(requestLock, std::chrono::seconds(10),
                            [&request]() { return request->complete; })) {
    return "{\"ok\":false,\"message\":\"Agent command timed out.\"}";
  }
  return request->output;
}

void TuiApp::processAgentProtocolRequests(const MeterData &currentMeters) {
  std::deque<std::shared_ptr<PendingAgentProtocolRequest>> requests;
  {
    std::lock_guard<std::mutex> lock(pendingAgentRequestsMutex);
    requests.swap(pendingAgentRequests);
  }

  for (const auto &request : requests) {
    const auto response = executeAgentCommand(request->input, currentMeters);
    {
      std::lock_guard<std::mutex> requestLock(request->mutex);
      request->output = response.json;
      request->complete = true;
    }
    request->cv.notify_one();
  }
}

void TuiApp::refreshDrumSampleEntries() {
  if (drumSampleLibrary == nullptr) {
    drumSampleEntries.clear();
    selectedDrumSampleIndex = 0;
    return;
  }

  drumSampleEntries = drumSampleLibrary->listSamples();
  if (drumSampleEntries.empty()) {
    selectedDrumSampleIndex = 0;
  } else {
    selectedDrumSampleIndex =
        std::clamp(selectedDrumSampleIndex, 0,
                   static_cast<int>(drumSampleEntries.size() - 1));
  }
}

std::string TuiApp::getDrumSlotSampleLabel(uint8_t slotIndex) const {
  if (slotIndex >= AppState::TrackState::DrumSampleSlotCount) {
    return "-";
  }

  const uint16_t sampleId =
      appState.tracks[0].getDrumSampleSlotSampleId(slotIndex);
  if (sampleId == 0) {
    return "(synth)";
  }

  if (drumSampleLibrary == nullptr) {
    return "ID " + std::to_string(sampleId);
  }

  const std::string name = drumSampleLibrary->getSampleName(sampleId);
  if (!name.empty()) {
    return "#" + std::to_string(sampleId) + " " + name;
  }

  return "#" + std::to_string(sampleId) + " (missing)";
}

void TuiApp::openDrumSampleModal() {
  if (drumSampleLibrary == nullptr) {
    setStatusMessage("Drum sample library is unavailable.", true);
    return;
  }

  drumSampleModalOpen = true;
  drumSampleModalFocus = DrumSampleModalFocus::Path;
  drumSamplePathInput.clear();
  drumSampleStatus =
      "Enter path then Enter to import+assign. Tab switches focus.";
  drumSampleStatusIsError = false;
  selectedDrumSlot = std::clamp<uint8_t>(
      selectedDrumSlot, 0,
      static_cast<uint8_t>(AppState::TrackState::DrumSampleSlotCount - 1));
  refreshDrumSampleEntries();
}

void TuiApp::closeDrumSampleModal() {
  drumSampleModalOpen = false;
  drumSamplePathInput.clear();
  drumSampleStatus.clear();
  drumSampleStatusIsError = false;
}

void TuiApp::refreshDownloadedSampleEntries() {
  downloadedSampleEntries.clear();
  if (p2pClient == nullptr) {
    selectedDownloadedSampleIndex = 0;
    return;
  }
  for (const auto &entry : p2pClient->registry().allEntries()) {
    if (entry.content_type == ContentType::Sample) {
      downloadedSampleEntries.push_back(entry);
    }
  }
  if (contributionLibrary != nullptr) {
    for (const auto &package : contributionLibrary->installedPackages()) {
      for (const auto &item : package.samplePacks) {
        P2PDownloadEntry entry;
        entry.preset_id = package.packageId + ":" + item.itemId;
        entry.sender_id = package.authorAgent;
        entry.verified = package.hashVerified;
        entry.content_type = ContentType::Sample;
        entry.display_name = item.name;
        entry.format = item.format;
        entry.sample_rate = item.sampleRate;
        entry.channels = item.channels;
        entry.duration = item.duration;
        entry.sha256 = item.sha256;
        entry.local_path =
            (std::filesystem::path(contributionLibrary->getPayloadDirectory()) /
             package.packageId / item.filePath)
                .string();
        downloadedSampleEntries.push_back(entry);
      }
    }
  }
  if (downloadedSampleEntries.empty()) {
    selectedDownloadedSampleIndex = 0;
  } else {
    selectedDownloadedSampleIndex =
        std::clamp(selectedDownloadedSampleIndex, 0,
                   static_cast<int>(downloadedSampleEntries.size() - 1));
  }
}

void TuiApp::openSoundFileBrowser() {
  if (p2pClient == nullptr) {
    setStatusMessage("P2P sample browser is unavailable.", true);
    return;
  }
  soundFileBrowserOpen = true;
  soundFileBrowserStatus =
      "Enter imports selected sample to the drum slot. Escape closes.";
  soundFileBrowserStatusIsError = false;
  refreshDownloadedSampleEntries();
}

void TuiApp::closeSoundFileBrowser() {
  soundFileBrowserOpen = false;
  soundFileBrowserStatus.clear();
  soundFileBrowserStatusIsError = false;
}

bool TuiApp::importSelectedDownloadedSample() {
  if (selectedTrack != 0) {
    soundFileBrowserStatus = "Downloaded sample import is available on Drums only.";
    soundFileBrowserStatusIsError = true;
    return false;
  }
  if (drumSampleLibrary == nullptr || downloadedSampleEntries.empty()) {
    soundFileBrowserStatus = "No downloaded samples available.";
    soundFileBrowserStatusIsError = true;
    return false;
  }
  const auto &entry = downloadedSampleEntries[static_cast<size_t>(selectedDownloadedSampleIndex)];
  uint16_t sampleId = 0;
  std::string error;
  if (!drumSampleLibrary->importSampleFromPath(entry.local_path, sampleId, error)) {
    soundFileBrowserStatus = "Import failed: " + error;
    soundFileBrowserStatusIsError = true;
    return false;
  }
  const uint16_t previous = appState.tracks[0].getDrumSampleSlotSampleId(selectedDrumSlot);
  Command cmd{Command::Type::SetDrumSampleAssignment, 0,
              Command::encodeDrumSlotSampleId(selectedDrumSlot, sampleId), 0.0f};
  Command undo = (previous == 0)
      ? Command{Command::Type::ClearDrumSampleAssignment, 0, selectedDrumSlot, 0.0f}
      : Command{Command::Type::SetDrumSampleAssignment, 0,
                Command::encodeDrumSlotSampleId(selectedDrumSlot, previous), 0.0f};
  if (!dispatchAndLog(cmd, "Import downloaded sample", undo)) {
    soundFileBrowserStatus = "Command queue full. Try again.";
    soundFileBrowserStatusIsError = true;
    return false;
  }
  refreshDrumSampleEntries();
  soundFileBrowserStatus = "Imported downloaded sample.";
  soundFileBrowserStatusIsError = false;
  return true;
}

bool TuiApp::setDrumSampleParam(Command::Type type, float value) {
  const Command cmd{type, 0, selectedDrumSlot, value};
  float prev = 0.0f;
  std::string paramName;
  switch (type) {
    case Command::Type::SetDrumSampleVolume:
      prev = appState.tracks[0].getDrumSampleSlotVolume(selectedDrumSlot);
      paramName = "Volume";
      break;
    case Command::Type::SetDrumSampleTune:
      prev = appState.tracks[0].getDrumSampleSlotTuneSemitones(selectedDrumSlot);
      paramName = "Tune";
      break;
    case Command::Type::SetDrumSampleStartOffset:
      prev = appState.tracks[0].getDrumSampleSlotStartOffset(selectedDrumSlot);
      paramName = "Start";
      break;
    case Command::Type::SetDrumSampleDecay:
      prev = appState.tracks[0].getDrumSampleSlotDecay(selectedDrumSlot);
      paramName = "Decay";
      break;
    case Command::Type::SetDrumSampleVelocitySensitivity:
      prev = appState.tracks[0].getDrumSampleSlotVelocitySensitivity(selectedDrumSlot);
      paramName = "Velocity";
      break;
    default:
      return false;
  }
  Command undoCmd{type, 0, selectedDrumSlot, prev};
  std::string desc = "Drum Slot " + std::to_string(selectedDrumSlot + 1) + " " + paramName;
  if (!dispatchAndLog(cmd, desc, undoCmd)) {
    drumSampleStatus = "Command queue full. Try again.";
    drumSampleStatusIsError = true;
    return false;
  }
  return true;
}

void TuiApp::nudgeDrumSampleParam(Command::Type type, float delta) {
  float current = 0.0f;
  float minValue = 0.0f;
  float maxValue = 1.0f;

  switch (type) {
  case Command::Type::SetDrumSampleVolume:
    current = appState.tracks[0].getDrumSampleSlotVolume(selectedDrumSlot);
    minValue = 0.0f;
    maxValue = 2.0f;
    break;
  case Command::Type::SetDrumSampleTune:
    current =
        appState.tracks[0].getDrumSampleSlotTuneSemitones(selectedDrumSlot);
    minValue = -24.0f;
    maxValue = 24.0f;
    break;
  case Command::Type::SetDrumSampleStartOffset:
    current = appState.tracks[0].getDrumSampleSlotStartOffset(selectedDrumSlot);
    minValue = 0.0f;
    maxValue = 0.95f;
    break;
  case Command::Type::SetDrumSampleDecay:
    current = appState.tracks[0].getDrumSampleSlotDecay(selectedDrumSlot);
    minValue = 0.0f;
    maxValue = 1.0f;
    break;
  case Command::Type::SetDrumSampleVelocitySensitivity:
    current = appState.tracks[0].getDrumSampleSlotVelocitySensitivity(
        selectedDrumSlot);
    minValue = 0.0f;
    maxValue = 1.0f;
    break;
  default:
    return;
  }

  const float next = std::clamp(current + delta, minValue, maxValue);
  if (setDrumSampleParam(type, next)) {
    drumSampleStatus = "Updated slot parameter.";
    drumSampleStatusIsError = false;
  }
}

bool TuiApp::assignSelectedDrumSample() {
  if (drumSampleEntries.empty()) {
    drumSampleStatus = "No imported samples yet.";
    drumSampleStatusIsError = true;
    return false;
  }

  selectedDrumSampleIndex =
      std::clamp(selectedDrumSampleIndex, 0,
                 static_cast<int>(drumSampleEntries.size() - 1));
  const auto &selected =
      drumSampleEntries[static_cast<size_t>(selectedDrumSampleIndex)];
  const Command cmd{
      Command::Type::SetDrumSampleAssignment,
      0,
      Command::encodeDrumSlotSampleId(selectedDrumSlot, selected.id),
      0.0f,
  };

  uint16_t prevSampleId = appState.tracks[0].getDrumSampleSlotSampleId(selectedDrumSlot);
  Command undoCmd = (prevSampleId == 0)
    ? Command{Command::Type::ClearDrumSampleAssignment, 0, selectedDrumSlot, 0.0f}
    : Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(selectedDrumSlot, prevSampleId), 0.0f};
  std::string desc = "Drum Slot " + std::to_string(selectedDrumSlot + 1) + " → Sample " + std::to_string(selected.id);
  if (!dispatchAndLog(cmd, desc, undoCmd)) {
    drumSampleStatus = "Command queue full. Try again.";
    drumSampleStatusIsError = true;
    return false;
  }

  drumSampleStatus = "Assigned slot " + std::to_string(selectedDrumSlot + 1) +
                     " to sample #" + std::to_string(selected.id) + ".";
  drumSampleStatusIsError = false;
  return true;
}

bool TuiApp::clearSelectedDrumSample() {
  const Command cmd{Command::Type::ClearDrumSampleAssignment, 0, selectedDrumSlot, 0.0f};
  uint16_t prevSampleId = appState.tracks[0].getDrumSampleSlotSampleId(selectedDrumSlot);
  Command undoCmd = (prevSampleId == 0)
    ? Command{Command::Type::ClearDrumSampleAssignment, 0, selectedDrumSlot, 0.0f}
    : Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(selectedDrumSlot, prevSampleId), 0.0f};
  std::string desc = "Clear Drum Slot " + std::to_string(selectedDrumSlot + 1);
  if (!dispatchAndLog(cmd, desc, undoCmd)) {
    drumSampleStatus = "Command queue full. Try again.";
    drumSampleStatusIsError = true;
    return false;
  }
  drumSampleStatus = "Cleared slot " + std::to_string(selectedDrumSlot + 1) + ".";
  drumSampleStatusIsError = false;
  return true;
}

bool TuiApp::importAndAssignDrumSample() {
  if (drumSampleLibrary == nullptr) {
    drumSampleStatus = "Drum sample library unavailable.";
    drumSampleStatusIsError = true;
    return false;
  }

  const std::string trimmed = trimCopy(drumSamplePathInput);
  if (trimmed.empty()) {
    drumSampleStatus = "Sample path is required.";
    drumSampleStatusIsError = true;
    return false;
  }

  uint16_t sampleId = 0;
  std::string error;
  if (!drumSampleLibrary->importSampleFromPath(trimmed, sampleId, error)) {
    drumSampleStatus = "Import failed: " + error;
    drumSampleStatusIsError = true;
    return false;
  }

  refreshDrumSampleEntries();

  const Command cmd{
      Command::Type::SetDrumSampleAssignment,
      0,
      Command::encodeDrumSlotSampleId(selectedDrumSlot, sampleId),
      0.0f,
  };

  uint16_t prevSampleId = appState.tracks[0].getDrumSampleSlotSampleId(selectedDrumSlot);
  Command undoCmd = (prevSampleId == 0)
    ? Command{Command::Type::ClearDrumSampleAssignment, 0, selectedDrumSlot, 0.0f}
    : Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(selectedDrumSlot, prevSampleId), 0.0f};
  std::string desc = "Import+Assign Drum Slot " + std::to_string(selectedDrumSlot + 1);
  if (!dispatchAndLog(cmd, desc, undoCmd)) {
    drumSampleStatus = "Imported but assignment queue is full.";
    drumSampleStatusIsError = true;
    return false;
  }

  drumSamplePathInput.clear();
  drumSampleStatus = "Imported and assigned sample #" + std::to_string(sampleId) +
                     " to slot " + std::to_string(selectedDrumSlot + 1) + ".";
  drumSampleStatusIsError = false;
  return true;
}

void TuiApp::openAlgorithmEditor() {
  algorithmEditorOpen = true;
  algorithmEditorTrack = static_cast<uint8_t>(selectedTrack);
  algorithmEditorStepCount = 16;
  algorithmEditorSelectedStep = 0;
  algorithmEditorFocus = 0;
  algorithmEditorStatus.clear();
  algorithmEditorStatusIsError = false;

  // Initialize draft
  algorithmEditorDraft = CustomAlgorithmPreset();
  algorithmEditorDraft.trackIndex = algorithmEditorTrack;
  algorithmEditorDraft.stepCount = algorithmEditorStepCount;
  algorithmEditorDraft.name = "Custom " + std::string(AlgorithmCatalog::getAlgorithmName(algorithmEditorTrack, 0));
  algorithmEditorDraft.id = sanitizeAlgorithmId(algorithmEditorDraft.name);
  algorithmEditorDraft.version = "1.0";
  algorithmEditorDraft.rhythmicPattern.resize(algorithmEditorStepCount, 0);
  algorithmEditorDraft.melodicPattern.resize(algorithmEditorStepCount, 0);
  algorithmEditorDraft.densityCurve.resize(algorithmEditorStepCount, 1.0f);
  algorithmEditorDraft.complexityCurve.resize(algorithmEditorStepCount, 1.0f);

  refreshCachedCustomAlgorithms();
}

void TuiApp::closeAlgorithmEditor() {
  algorithmEditorOpen = false;
  algorithmEditorStatus.clear();
  algorithmEditorStatusIsError = false;
}

void TuiApp::refreshCachedCustomAlgorithms() {
  cachedCustomAlgorithms = globalAlgorithmPresetRegistry().listForTrack(algorithmEditorTrack);
  // Notify audio engine to rebuild its custom algorithm instances
  Command cmd{Command::Type::RebuildCustomAlgorithms, 0, 0, 0.0f};
  cmdQueue.push(cmd);
}

bool TuiApp::saveAlgorithmEditorDraft() {
  // Validate
  std::string error;
  if (!validate(algorithmEditorDraft, error)) {
    algorithmEditorStatus = error;
    algorithmEditorStatusIsError = true;
    return false;
  }

  // Set step count from editor
  algorithmEditorDraft.stepCount = algorithmEditorStepCount;
  algorithmEditorDraft.rhythmicPattern.resize(algorithmEditorStepCount, 0);
  algorithmEditorDraft.melodicPattern.resize(algorithmEditorStepCount, 0);
  algorithmEditorDraft.densityCurve.resize(algorithmEditorStepCount, 1.0f);
  algorithmEditorDraft.complexityCurve.resize(algorithmEditorStepCount, 1.0f);

  // Save to registry
  if (!globalAlgorithmPresetRegistry().savePreset(algorithmEditorDraft, error)) {
    algorithmEditorStatus = error;
    algorithmEditorStatusIsError = true;
    return false;
  }

  // Rebuild audio engine's custom instances
  // (This is done via the registry's rebuild signal - for now, we rely on
  //  the audio engine polling or being notified through the command queue)
  refreshCachedCustomAlgorithms();

  algorithmEditorStatus = "Saved custom algorithm '" + algorithmEditorDraft.name + "'.";
  algorithmEditorStatusIsError = false;
  return true;
}

bool TuiApp::deleteSelectedCustomAlgorithm() {
  if (selectedCustomAlgorithmIndex < 0 || selectedCustomAlgorithmIndex >= static_cast<int>(cachedCustomAlgorithms.size()))
    return false;

  const auto& preset = cachedCustomAlgorithms[selectedCustomAlgorithmIndex];
  auto runtimeId = globalAlgorithmPresetRegistry().runtimeIdForPresetId(algorithmEditorTrack, preset.id);
  if (!runtimeId.has_value())
    return false;

  std::string error;
  if (!globalAlgorithmPresetRegistry().deletePreset(runtimeId.value(), error)) {
    algorithmEditorStatus = error;
    algorithmEditorStatusIsError = true;
    return false;
  }

  refreshCachedCustomAlgorithms();
  algorithmEditorStatus = "Deleted custom algorithm '" + preset.name + "'.";
  algorithmEditorStatusIsError = false;
  return true;
}
