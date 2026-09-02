#pragma once

#include "../App/AppState.h"
#include "../App/AgentCommand.h"
#include "../App/AlgorithmPresetRegistry.h"
#include "../App/CommandQueue.h"
#include "../App/ContributionPackage.h"
#include "../App/DrumSampleLibrary.h"
#include "../App/MeterQueue.h"
#include "../App/ProjectIO.h"
#include "../Network/P2PClient.h"
#include "../Network/PeerDiscovery.h"
#include "../Security/PresetSerializer.h"
#include "Components/ArrangementModal.h"
#include "Components/NumberSelectionModal.h"
#include "Components/ProjectPathModal.h"
#include "Components/GrooveModal.h"

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <array>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class AgentProtocolServer;

struct UndoAction {
  std::string description;
  Command undoCommand;
};

class TuiApp {
public:
  TuiApp(AppState &appState, CommandQueue &cmdQueue, MeterQueue &meterQueue,
         DrumSampleLibrary *drumSampleLibrary = nullptr,
         ContributionPackage::Library *contributionLibrary = nullptr,
         P2PClient *p2pClient = nullptr,
         PresetSerializer *presetSerializer = nullptr,
         PeerDiscovery *peerDiscovery = nullptr,
         const std::string &initialProjectPath = "",
         const std::string &startupStatusMessage = "",
         bool startupStatusIsError = false,
         int agentPort = 0,
         std::function<std::string(const std::string&, const std::string&)> recordFn = nullptr,
         std::function<std::string(const std::string&, const std::string&)> streamFn = nullptr);
  ~TuiApp();

  void run();
  const std::string &getActiveProjectPath() const;

private:
  void openNumberSelection(NumberSelectionDomain domain);
  void closeNumberSelection();
  void clearNumberSelectionInput();
  void refreshNumberSelectionPreview();
  bool submitNumberSelection();
  uint16_t getNumberSelectionImplementedCount() const;

  void openKeySelection();
  void closeKeySelection();
  void refreshKeySelectionPreview();
  bool submitKeySelection();

  void openArrangementModal();
  void closeArrangementModal();
  void cycleArrangementModalFocus(int delta);
  void nudgeArrangementModalFocusedValue(int delta);
  bool submitArrangementModal();

  // Groove modal
  void openGrooveModal();
  void closeGrooveModal();
  void cycleGrooveModalFocus(int delta);
  void nudgeGrooveModalValue(int delta);

  void loadRecentProjects();
  void persistRecentProjects();
  void openProjectPathModal(ProjectPathModalMode mode);
  void closeProjectPathModal();
  void selectRecentProject(int delta);
  bool submitProjectPathModal();
  void setStatusMessage(const std::string &message, bool isError);
  void showSpotEffectPopup(const Command &cmd);
  bool quickSaveActiveProject();
  bool saveProjectToPath(const std::string &path);
  bool loadProjectFromPath(const std::string &path);

  // Command Dispatch & Logging
  bool dispatchAndLog(const Command &cmd, const std::string &description,
                      const Command &undoCmd);
  void performUndo();
  AgentCommand::Response executeAgentCommand(const std::string &input,
                                             const MeterData &currentMeters);
  std::string enqueueAgentProtocolCommand(const std::string &input);
  void processAgentProtocolRequests(const MeterData &currentMeters);

  void openDrumSampleModal();
  void closeDrumSampleModal();
  void refreshDrumSampleEntries();
  bool importAndAssignDrumSample();
  bool assignSelectedDrumSample();
  bool clearSelectedDrumSample();
  void openSoundFileBrowser();
  void closeSoundFileBrowser();
  void refreshDownloadedSampleEntries();
  bool importSelectedDownloadedSample();

  // Algorithm Editor
  void openAlgorithmEditor();
  void closeAlgorithmEditor();
  void refreshCachedCustomAlgorithms();
  bool saveAlgorithmEditorDraft();
  bool deleteSelectedCustomAlgorithm();

  bool setDrumSampleParam(Command::Type type, float value);
  void nudgeDrumSampleParam(Command::Type type, float delta);
  std::string getDrumSlotSampleLabel(uint8_t slotIndex) const;
  std::vector<std::string> buildMidiViewRows(size_t width, int height,
                                             uint8_t trackIndex);

  // Extracted from run(): UI rendering and input handling
  ftxui::Element buildUI(const MeterData& currentMeters);
  bool handleEventInput(const ftxui::Event& event,
                        ftxui::ScreenInteractive& screen,
                        const MeterData& currentMeters);

  // Modal input handlers (extracted from handleEventInput)
  bool handleOnboardingTipsInput(const ftxui::Event& event);
  bool handleAgentInput(const ftxui::Event& event,
                        const MeterData& currentMeters);
  bool handleAlgorithmEditorInput(const ftxui::Event& event);
  bool handleDrumSampleModalInput(const ftxui::Event& event);
  bool handleSoundFileBrowserInput(const ftxui::Event& event);
  bool handleProjectPathModalInput(const ftxui::Event& event);
  bool handleArrangementModalInput(const ftxui::Event& event);
  bool handleGrooveModalInput(const ftxui::Event& event);
  bool handleNumberSelectionInput(const ftxui::Event& event);
  bool handleKeySelectionInput(const ftxui::Event& event);
  bool handleMainInput(const ftxui::Event& event,
                       ftxui::ScreenInteractive& screen);

  AppState &appState;
  CommandQueue &cmdQueue;
  MeterQueue &meterQueue;
  DrumSampleLibrary *drumSampleLibrary = nullptr;
  ContributionPackage::Library *contributionLibrary = nullptr;
  P2PClient *p2pClient = nullptr;
  PresetSerializer *presetSerializer = nullptr;
  PeerDiscovery *peerDiscovery = nullptr;
  int agentPort = 0;
  std::unique_ptr<AgentProtocolServer> agentServer;

  struct PendingAgentProtocolRequest {
    std::string input;
    std::string output;
    bool complete = false;
    std::mutex mutex;
    std::condition_variable cv;
  };

  std::mutex pendingAgentRequestsMutex;
  std::deque<std::shared_ptr<PendingAgentProtocolRequest>>
      pendingAgentRequests;
  std::vector<MeterData> agentMeterHistory;
  bool agentInputActive = false;
  std::string agentInput;
  std::string agentStatus = "ready";
  std::deque<std::string> agentInputHistory;
  int agentHistoryIndex = -1;

  std::deque<std::array<uint64_t, 2>> trackMidiHistory[4];

  int selectedTrack = 0;
  bool showHelp = false;
  int helpScrollOffset = 0;

  bool numberSelectionOpen = false;
  NumberSelectionDomain numberSelectionDomain =
      NumberSelectionDomain::Algorithm;
  int numberSelectionTrack = 0;
  std::string numberSelectionInput;
  std::string numberSelectionPreview;
  std::string numberSelectionStatus;
  bool numberSelectionValid = true;

  bool keySelectionOpen = false;
  std::string keySelectionInput;
  std::string keySelectionPreview;
  std::string keySelectionStatus;
  bool keySelectionValid = true;

  bool arrangementModalOpen = false;
  ArrangementModalFocus arrangementModalFocus =
      ArrangementModalFocus::Preset;
  uint8_t arrangementModalSectionCount = 4;
  uint8_t arrangementModalCurrentSection = 0;
  std::array<uint8_t, AppState::kArrangementMaxSections>
      arrangementModalSectionLengths{{4, 4, 4, 4, 4, 4, 4, 4}};
  std::array<uint8_t, AppState::kArrangementMaxSections>
      arrangementModalSectionProgressions{{
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
          AppState::kArrangementProgressionFollowGlobal,
      }};
  std::array<uint8_t, AppState::kArrangementMaxSections>
      arrangementModalSectionTrackMasks{{
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
          AppState::kArrangementTrackMaskAll,
      }};
  bool arrangementModalSectionParametersEnabled = false;
  uint8_t arrangementModalParameterTrack = 0;
  uint8_t arrangementModalParameterIndex = 0;
  uint8_t arrangementModalPresetIndex = 0;
  std::array<std::array<std::array<float, AppState::kArrangementTrackParameterCount>, AppState::kTrackCount>, AppState::kArrangementMaxSections>
      arrangementModalSectionTrackParameters{};
  std::string arrangementModalStatus;
  bool arrangementModalStatusIsError = false;

  // Groove modal
  bool grooveModalOpen = false;
  GrooveModalFocus grooveModalFocus = GrooveModalFocus::Swing;
  std::string grooveModalStatus;
  bool grooveModalStatusIsError = false;

  bool projectPathModalOpen = false;
  ProjectPathModalMode projectPathModalMode = ProjectPathModalMode::Save;
  std::string projectPathInput;
  std::string projectDirectoryInput;
  std::string projectNameInput;
  SaveProjectFieldFocus saveProjectFieldFocus = SaveProjectFieldFocus::Name;
  std::string projectPathStatus;
  bool projectPathStatusIsError = false;

  // Undo System
  std::deque<UndoAction> undoStack;
  static constexpr size_t kMaxUndoActions = 50;
  std::vector<std::string> recentProjectPaths;
  int selectedRecentProjectIndex = 0;
  std::string activeProjectPath;
  std::string uiStatusMessage;
  bool uiStatusIsError = false;
  std::string spotEffectPopupMessage;

  enum class DrumSampleModalFocus {
    Path,
    List,
  };

  bool drumSampleModalOpen = false;
  DrumSampleModalFocus drumSampleModalFocus = DrumSampleModalFocus::Path;
  std::string drumSamplePathInput;
  std::string drumSampleStatus;
  bool drumSampleStatusIsError = false;
  uint8_t selectedDrumSlot = 0;
  std::vector<DrumSampleLibrary::SampleRecord> drumSampleEntries;
  int selectedDrumSampleIndex = 0;

  bool onboardingTipsOpen = false;

  bool soundFileBrowserOpen = false;
  int selectedDownloadedSampleIndex = 0;
  std::string soundFileBrowserStatus;
  bool soundFileBrowserStatusIsError = false;
  std::vector<P2PDownloadEntry> downloadedSampleEntries;

  // Algorithm Editor Modal
  bool algorithmEditorOpen = false;
  uint8_t algorithmEditorTrack = 0;
  CustomAlgorithmPreset algorithmEditorDraft;
  uint8_t algorithmEditorStepCount = 16;
  uint8_t algorithmEditorSelectedStep = 0;
  uint8_t algorithmEditorFocus = 0;
  std::string algorithmEditorStatus;
  bool algorithmEditorStatusIsError = false;
  std::vector<CustomAlgorithmPreset> cachedCustomAlgorithms;
  int selectedCustomAlgorithmIndex = 0;

  // Recording callback (routes to JuceRuntime via Main)
  std::function<std::string(const std::string&, const std::string&)> recordFn;

  // Streaming callback (routes to JuceRuntime via Main)
  std::function<std::string(const std::string&, const std::string&)> streamFn;
};
