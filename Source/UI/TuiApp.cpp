#include "TuiApp.h"
#include "AgentProtocolServer.h"
#include "TuiAppProject.h"
#include "../App/AgentCommand.h"
#include "../App/AlgorithmCatalog.h"
#include "../App/DrumSampleCatalog.h"
#include "../App/EffectPresetCatalog.h"
#include "../App/GenreCatalog.h"
#include "../App/KeyMapping.h"
#include "../App/ProjectIO.h"
#include "../App/ProjectKey.h"
#include "../App/SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"
#include "Components/AgentInputBar.h"
#include "Components/ArrangementModal.h"
#include "Components/KeyEntryModal.h"
#include "Components/NumberSelectionModal.h"
#include "Components/ProjectPathModal.h"
#include "Components/SpectrumView.h"
#include "Components/TrackPanel.h"
#include "Components/TransportBar.h"
#include "Themes/Colors.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/terminal.hpp>
#include <iomanip>
#include <limits>
#include <sstream>
#include <thread>
#include <utility>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#if defined(_WIN32)
#include <io.h>
#define CENDANCE_ISATTY _isatty
#define CENDANCE_FILENO _fileno
#else
#include <unistd.h>
#define CENDANCE_ISATTY isatty
#define CENDANCE_FILENO fileno
#endif

namespace {

constexpr size_t kMaxNumberSelectionDigits = 6;
constexpr size_t kMaxKeySelectionChars = 24;
constexpr size_t kMaxAgentInputChars = 240;
constexpr int kUiTrackCount = 5;
constexpr int kMasterTrackIndex = 4;
constexpr int kArrangementModalFieldCount = 7;

} // namespace

namespace {

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

const char *verticalBlockGlyph(int eighths) {
  static constexpr std::array<const char *, 9> kGlyphs = {
      " ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
  return kGlyphs[static_cast<size_t>(std::clamp(eighths, 0, 8))];
}

float sampleSpectrumBar(const MeterData &meterData, size_t bar,
                        size_t barCount) {
  if (barCount <= 1) {
    return meterData.spectrumBins[0];
  }

  const float sourcePosition =
      (static_cast<float>(bar) * static_cast<float>(kSpectrumBinCount - 1)) /
      static_cast<float>(barCount - 1);
  const size_t left = static_cast<size_t>(std::floor(sourcePosition));
  const size_t right = std::min(left + 1, kSpectrumBinCount - 1);
  const float blend = sourcePosition - static_cast<float>(left);

  const float leftValue =
      std::clamp(meterData.spectrumBins[left], 0.0f, 1.0f);
  const float rightValue =
      std::clamp(meterData.spectrumBins[right], 0.0f, 1.0f);
  const float interpolated = leftValue + ((rightValue - leftValue) * blend);

  if (interpolated <= 0.015f) {
    return 0.0f;
  }

  // UI-only shaping: keeps quieter spectra alive while preserving hot peaks.
  return std::pow(std::clamp(interpolated, 0.0f, 1.0f), 0.78f);
}

std::vector<std::string> buildCompactSpectrumRows(const MeterData &meterData,
                                                  size_t width, int height) {
  if (width == 0 || height <= 0) {
    return {};
  }

  std::vector<std::string> rows(static_cast<size_t>(height),
                                std::string{});
  for (auto &row : rows) {
    row.reserve(width * 3);
  }

  const size_t barCount = std::max<size_t>(1, (width + 1) / 2);
  static std::vector<int> peakEighths;
  static uint32_t lastAnalyzerFrame = 0;
  if (peakEighths.size() != barCount) {
    peakEighths.assign(barCount, 0);
  }

  const uint32_t frameDelta =
      (lastAnalyzerFrame == 0 || meterData.analyzerFrame <= lastAnalyzerFrame)
          ? 1
          : meterData.analyzerFrame - lastAnalyzerFrame;
  lastAnalyzerFrame = meterData.analyzerFrame;
  const int peakDecay = std::clamp(static_cast<int>(frameDelta) * 2, 1, 12);

  if (!meterData.analyzerValid) {
    for (auto &peak : peakEighths) {
      peak = std::max(0, peak - peakDecay);
    }
  }

  std::vector<int> barEighths(barCount, 0);
  if (meterData.analyzerValid) {
    for (size_t bar = 0; bar < barCount; ++bar) {
      const float value = sampleSpectrumBar(meterData, bar, barCount);
      barEighths[bar] = std::clamp(
          static_cast<int>(std::lround(value * static_cast<float>(height * 8))),
          0, height * 8);
      peakEighths[bar] = std::max(barEighths[bar],
                                  std::max(0, peakEighths[bar] - peakDecay));
    }
  }

  for (int row = 0; row < height; ++row) {
    auto &line = rows[static_cast<size_t>(row)];
    const int cellFromBottom = (height - 1) - row;
    int displayWidth = 0;
    for (size_t bar = 0; bar < barCount; ++bar) {
      if (displayWidth >= static_cast<int>(width)) {
        break;
      }

      const int cellBaseEighths = cellFromBottom * 8;
      const int cellEighths = std::clamp(barEighths[bar] - (cellFromBottom * 8),
                                         0, 8);
      const bool drawPeak =
          cellEighths == 0 && peakEighths[bar] > cellBaseEighths &&
          peakEighths[bar] <= cellBaseEighths + 8;
      line += drawPeak ? "▔" : verticalBlockGlyph(cellEighths);
      ++displayWidth;
      if (displayWidth < static_cast<int>(width) && bar + 1 < barCount) {
        line += " ";
        ++displayWidth;
      }
    }
    while (displayWidth < static_cast<int>(width)) {
      const size_t bar = barCount - 1;
      const int cellBaseEighths = cellFromBottom * 8;
      const int cellEighths = std::clamp(barEighths[bar] - cellBaseEighths,
                                         0, 8);
      const bool drawPeak =
          cellEighths == 0 && peakEighths[bar] > cellBaseEighths &&
          peakEighths[bar] <= cellBaseEighths + 8;
      line += drawPeak ? "▔" : verticalBlockGlyph(cellEighths);
      ++displayWidth;
    }
  }

  return rows;
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

bool parseDisplayId(const std::string &input, uint16_t &output) {
  if (input.empty()) {
    return false;
  }

  uint32_t value = 0;
  for (const char ch : input) {
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }

    value = value * 10u + static_cast<uint32_t>(ch - '0');
    if (value > std::numeric_limits<uint16_t>::max()) {
      return false;
    }
  }

  output = static_cast<uint16_t>(value);
  return true;
}

enum class EffectSelectionKind : uint8_t {
  Invalid,
  SlotOnly,
  CategoryOnly,
  ClearSlot,
  Preset,
};

struct EffectSelectionInput {
  EffectSelectionKind kind = EffectSelectionKind::Invalid;
  uint8_t slotIndex = 0;
  uint8_t categoryDigit = 0;
  uint16_t presetDisplayId = 0;
};

std::string slotLabel(const uint8_t slotIndex) {
  return "Slot " + std::to_string(static_cast<int>(slotIndex) + 1);
}

bool parseEffectSelectionInputImpl(const std::string &input,
                                   EffectSelectionInput &output) {
  output = EffectSelectionInput{};

  if (input.empty()) {
    return false;
  }

  const char slotChar = input[0];
  if (slotChar < '1' || slotChar > '3') {
    return false;
  }

  output.slotIndex = static_cast<uint8_t>(slotChar - '1');
  if (input.size() == 1) {
    output.kind = EffectSelectionKind::SlotOnly;
    return true;
  }

  const char categoryChar = input[1];
  if (categoryChar == '-') {
    if (input.size() != 2) {
      return false;
    }

    output.kind = EffectSelectionKind::ClearSlot;
    return true;
  }

  if (!std::isdigit(static_cast<unsigned char>(categoryChar))) {
    return false;
  }

  output.categoryDigit = static_cast<uint8_t>(categoryChar - '0');
  if (input.size() == 2) {
    output.kind = EffectSelectionKind::CategoryOnly;
    return true;
  }

  uint32_t presetValue = 0;
  for (size_t i = 2; i < input.size(); ++i) {
    const char ch = input[i];
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }

    presetValue = presetValue * 10u + static_cast<uint32_t>(ch - '0');
    if (presetValue > std::numeric_limits<uint16_t>::max()) {
      return false;
    }
  }

  if (presetValue == 0) {
    return false;
  }

  output.kind = EffectSelectionKind::Preset;
  output.presetDisplayId = static_cast<uint16_t>(presetValue);
  return true;
}

ftxui::ScreenInteractive makeScreen() {
  const char *modeEnv = std::getenv("CENDANCE_TUI_MODE");
  if (modeEnv != nullptr) {
    const std::string mode(modeEnv);
    if (mode == "terminal" || mode == "terminal-output") {
      return ftxui::ScreenInteractive::TerminalOutput();
    }
    if (mode == "fit") {
      return ftxui::ScreenInteractive::FitComponent();
    }
  }

  const bool stdinTty = CENDANCE_ISATTY(CENDANCE_FILENO(stdin)) != 0;
  const bool stdoutTty = CENDANCE_ISATTY(CENDANCE_FILENO(stdout)) != 0;

  if (!stdinTty || !stdoutTty) {
    return ftxui::ScreenInteractive::TerminalOutput();
  }

  return ftxui::ScreenInteractive::Fullscreen();
}

} // namespace

std::vector<std::string> TuiApp::buildMidiViewRows(size_t width, int height,
                                                   uint8_t trackIndex) {
  if (width == 0 || height <= 0 || trackIndex >= 4) {
    return std::vector<std::string>(height, std::string(width, ' '));
  }
  std::vector<std::string> rows(static_cast<size_t>(height),
                                std::string(width, ' '));
  auto &history = trackMidiHistory[trackIndex];

  int minNote = 127;
  int maxNote = 0;
  bool hasNotes = false;

  for (const auto &maskPair : history) {
    for (int b = 0; b < 2; ++b) {
      uint64_t mask = maskPair[b];
      while (mask) {
        int bit = __builtin_ctzll(mask);
        int note = b * 64 + bit;
        minNote = std::min(minNote, note);
        maxNote = std::max(maxNote, note);
        hasNotes = true;
        mask &= mask - 1;
      }
    }
  }

  if (!hasNotes) {
    minNote = 60;
    maxNote = 60 + height - 1;
  } else if (maxNote - minNote < height - 1) {
    int center = (minNote + maxNote) / 2;
    minNote = center - height / 2;
    maxNote = minNote + height - 1;
  }

  const float noteRange = std::max(1.0f, static_cast<float>(maxNote - minNote));
  const size_t histSize = history.size();

  for (size_t x = 0; x < width && x < histSize; ++x) {
    const auto &maskPair = history[x];
    size_t drawX = width - 1 - x;
    for (int b = 0; b < 2; ++b) {
      uint64_t mask = maskPair[b];
      while (mask) {
        int bit = __builtin_ctzll(mask);
        int note = b * 64 + bit;

        if (note >= minNote && note <= maxNote) {
          float t = static_cast<float>(note - minNote) / noteRange;
          int row = std::clamp(
              static_cast<int>((1.0f - t) * static_cast<float>(height - 1) +
                               0.5f),
              0, height - 1);
          rows[static_cast<size_t>(row)][drawX] = '=';
        }
        mask &= mask - 1;
      }
    }
  }
  return rows;
}

TuiApp::TuiApp(AppState &ast, CommandQueue &cq, MeterQueue &mq,
               DrumSampleLibrary *drumSampleLibrary,
               ContributionPackage::Library *contributionLibrary,
               P2PClient *p2pClient,
               PresetSerializer *presetSerializer,
               PeerDiscovery *peerDiscovery,
               const std::string &initialProjectPath,
               const std::string &startupStatusMessage,
               bool startupStatusIsError,
               int agentPort,
               std::function<std::string(const std::string&, const std::string&)> recordFn,
               std::function<std::string(const std::string&, const std::string&)> streamFn)
    : appState(ast), cmdQueue(cq), meterQueue(mq),
      drumSampleLibrary(drumSampleLibrary),
      contributionLibrary(contributionLibrary),
      p2pClient(p2pClient),
      presetSerializer(presetSerializer),
      peerDiscovery(peerDiscovery),
      agentPort(agentPort),
      activeProjectPath(initialProjectPath),
      recordFn(std::move(recordFn)),
      streamFn(std::move(streamFn)) {
  loadRecentProjects();
  refreshDrumSampleEntries();
  if (!startupStatusMessage.empty()) {
    setStatusMessage(startupStatusMessage, startupStatusIsError);
  }
}

TuiApp::~TuiApp() = default;

const std::string &TuiApp::getActiveProjectPath() const {
  return activeProjectPath;
}



void TuiApp::run() {
  using namespace ftxui;
  auto screen = makeScreen();

  MeterData currentMeters;

  if (agentPort > 0 && agentServer == nullptr) {
    agentServer = std::make_unique<AgentProtocolServer>(
        agentPort,
        [this](const std::string &input) { return enqueueAgentProtocolCommand(input); });
    std::string serverError;
    if (agentServer->start(serverError)) {
      agentStatus = "agent port " + std::to_string(agentPort);
      setStatusMessage("Agent protocol listening on 127.0.0.1:" +
                           std::to_string(agentPort),
                       false);
    } else {
      agentStatus = "agent server error";
      setStatusMessage(serverError, true);
      agentServer.reset();
    }
  }

  // ── Renderer delegates to extracted method ──────────────────────────
  auto renderer = Renderer([this, &currentMeters]() -> Element {
    return buildUI(currentMeters);
  });

  // ── Input delegates to extracted method ─────────────────────────────
  auto mainComponent = CatchEvent(renderer, [this, &screen, &currentMeters](Event event) {
    return handleEventInput(event, screen, currentMeters);
  });
  Loop loop(&screen, mainComponent);
  while (!loop.HasQuitted()) {
    bool hasNewMeters = meterQueue.popLatest(currentMeters);
    if (hasNewMeters || currentMeters.isPlaying) {
      const int terminalWidth = std::max(80, ftxui::Terminal::Size().dimx);
      const int analyzerInnerWidth = std::max(14, terminalWidth - 6);
      for (int t = 0; t < 4; ++t) {
        trackMidiHistory[t].push_front(
            {currentMeters.activeNotes[t][0], currentMeters.activeNotes[t][1]});
        while (trackMidiHistory[t].size() >
               static_cast<size_t>(analyzerInnerWidth)) {
          trackMidiHistory[t].pop_back();
        }
      }
    }
    if (hasNewMeters) {
      agentMeterHistory.push_back(currentMeters);
      constexpr size_t kMaxAgentMeterHistory = 1800;
      if (agentMeterHistory.size() > kMaxAgentMeterHistory) {
        agentMeterHistory.erase(
            agentMeterHistory.begin(),
            agentMeterHistory.begin() +
                static_cast<std::ptrdiff_t>(agentMeterHistory.size() -
                                            kMaxAgentMeterHistory));
      }
    }
    processAgentProtocolRequests(currentMeters);
    screen.PostEvent(Event::Custom);
    loop.RunOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30fps
  }

  if (agentServer != nullptr) {
    agentServer->stop();
    agentServer.reset();
  }
}
