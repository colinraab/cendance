#include "TuiApp.h"
#include "Components/TransportBar.h"
#include "Components/TrackPanel.h"
#include "Components/AgentInputBar.h"
#include "Components/ArrangementModal.h"
#include "Components/NumberSelectionModal.h"
#include "Components/KeyEntryModal.h"
#include "Components/ProjectPathModal.h"
#include "Components/SpectrumView.h"
#include "Themes/Colors.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/screen/terminal.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace {
constexpr int kUiTrackCount = 5;
constexpr int kMasterTrackIndex = 4;
} // namespace

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



ftxui::Element TuiApp::buildUI(const MeterData& currentMeters) {
  using namespace ftxui;

  const bool isPlaying = appState.playing.load(std::memory_order_relaxed);

  // ── Terminal dimensions & layout mode ─────────────────────────────────────
  const int terminalWidth  = std::max(40, Terminal::Size().dimx);
  const int terminalHeight = std::max(10, Terminal::Size().dimy);

  TrackLayoutMode layoutMode;
  if (terminalWidth >= 140) {
    layoutMode = TrackLayoutMode::Full;
  } else if (terminalWidth >= 80) {
    layoutMode = TrackLayoutMode::Compact;
  } else {
    layoutMode = TrackLayoutMode::Focused;
  }

  // Fixed-row estimate (per layout mode) for all non-analyzer content:
  //   outer border(2) + transport(1-2) + project(1) + sep(1) + tracks(~14) +
  //   sep(1) + arrangement(3-5) + sep(1) + status(1) + log border+content(5)
  // The analyzer is only shown when it can be at least 8 rows tall.
  // It is placed LAST in the vbox so FTXUI clips it before status/log.
  constexpr int kAnalyzerMin  = 8;
  const int kFixedRowsEst = (terminalWidth >= 100)
      ? 32   // wide: 1-row transport, 1-row arrangement
      : 35;  // narrow: 2-row transport, 3-row arrangement grid
  const bool showAnalyzer = (terminalHeight >= kFixedRowsEst + kAnalyzerMin + 2);
  const int  analyzerHeight = showAnalyzer
      ? std::clamp(terminalHeight - kFixedRowsEst - 2, kAnalyzerMin, 14)
      : 0;


  // ── Transport bar ─────────────────────────────────────────────────────────
  auto topBar = TransportBar(
      appState.bpm.load(), isPlaying, currentMeters.barNumber,
      currentMeters.beatPosition, appState.chordProgression.load(),
      appState.projectKeyRoot.load(std::memory_order_relaxed),
      appState.projectKeyMode.load(std::memory_order_relaxed),
      terminalWidth);

  // ── Project line (truncated path on narrow terminals) ──────────────────────
  std::string projectDisplayPath = activeProjectPath.empty() ? "(none)" : activeProjectPath;
  if (terminalWidth < 100 && !activeProjectPath.empty()) {
    // Show only the filename portion
    const auto slash = activeProjectPath.rfind('/');
    const auto bslash = activeProjectPath.rfind('\\');
    size_t sep = std::string::npos;
    if (slash != std::string::npos) sep = slash;
    if (bslash != std::string::npos && (sep == std::string::npos || bslash > sep)) sep = bslash;
    if (sep != std::string::npos) {
      projectDisplayPath = "..." + activeProjectPath.substr(sep);
    }
  }
  auto projectLine =
      hbox({text(" Project: ") | bold | color(Theme::Highlight),
            text(projectDisplayPath) |
                color(activeProjectPath.empty() ? Theme::Inactive
                                                : Theme::Foreground)});

  // ── Helper lambda to build slot names for a given track ────────────────────
  auto getSlotNames = [&](int i) -> std::array<std::string, 3> {
    std::array<std::string, 3> names;
    for (uint8_t slot = 0; slot < 3; ++slot) {
      const uint16_t presetId =
          (i == kMasterTrackIndex)
              ? appState.master.getEffectPresetSlot(slot)
              : appState.tracks[i].getEffectPresetSlot(slot);
      names[slot] = std::string(EffectPresetCatalog::getPresetName(presetId));
    }
    return names;
  };

  // ── Track panels ──────────────────────────────────────────────────────────
  // Track panel widths are kept explicit so compact-mode content does not
  // skew the hbox distribution near the mode boundary.
  const int panelOuterWidth = (layoutMode == TrackLayoutMode::Focused)
      ? std::max(10, terminalWidth - 4)
      : std::max(10, (terminalWidth - 6) / kUiTrackCount);
  const int panelInnerWidth = std::max(8, panelOuterWidth - 2);

  Elements trackPanels;

  if (layoutMode == TrackLayoutMode::Focused) {
    // Focused mode: show selected track + mini strip of all 5
    const int selIdx = selectedTrack;
    auto slots = getSlotNames(selIdx);
    trackPanels.push_back(TrackPanel(
        selIdx, true,
        (selIdx == kMasterTrackIndex) ? 0 : currentMeters.activeAlgorithm[selIdx],
        (selIdx == kMasterTrackIndex) ? 0 : appState.tracks[selIdx].synthPreset.load(std::memory_order_relaxed),
        slots[0], slots[1], slots[2],
        (selIdx == kMasterTrackIndex) ? 0.0f : appState.tracks[selIdx].density.load(),
        (selIdx == kMasterTrackIndex) ? 0.0f : appState.tracks[selIdx].complexity.load(),
        (selIdx == kMasterTrackIndex) ? 0.0f : appState.tracks[selIdx].tone.load(std::memory_order_relaxed),
        (selIdx == kMasterTrackIndex) ? 0.0f : appState.tracks[selIdx].motion.load(std::memory_order_relaxed),
        (selIdx == kMasterTrackIndex) ? false : appState.tracks[selIdx].muted.load(std::memory_order_relaxed),
        (selIdx == kMasterTrackIndex) ? appState.master.gain.load(std::memory_order_relaxed) : appState.tracks[selIdx].gain.load(std::memory_order_relaxed),
        (selIdx == kMasterTrackIndex) ? currentMeters.masterLevel : currentMeters.trackLevels[selIdx],
        TrackLayoutMode::Full, panelInnerWidth));

  } else {
    // Full or Compact: render all 5 tracks side by side
    for (int i = 0; i < kUiTrackCount; ++i) {
      auto slots = getSlotNames(i);
      auto panel = TrackPanel(
          i, (i == selectedTrack),
          (i == kMasterTrackIndex) ? 0 : currentMeters.activeAlgorithm[i],
          (i == kMasterTrackIndex) ? 0 : appState.tracks[i].synthPreset.load(std::memory_order_relaxed),
          slots[0], slots[1], slots[2],
          (i == kMasterTrackIndex) ? 0.0f : appState.tracks[i].density.load(),
          (i == kMasterTrackIndex) ? 0.0f : appState.tracks[i].complexity.load(),
          (i == kMasterTrackIndex) ? 0.0f : appState.tracks[i].tone.load(std::memory_order_relaxed),
          (i == kMasterTrackIndex) ? 0.0f : appState.tracks[i].motion.load(std::memory_order_relaxed),
          (i == kMasterTrackIndex) ? false : appState.tracks[i].muted.load(std::memory_order_relaxed),
          (i == kMasterTrackIndex) ? appState.master.gain.load(std::memory_order_relaxed) : appState.tracks[i].gain.load(std::memory_order_relaxed),
          (i == kMasterTrackIndex) ? currentMeters.masterLevel : currentMeters.trackLevels[i],
          layoutMode, panelInnerWidth);
      trackPanels.push_back(panel | size(WIDTH, EQUAL, panelOuterWidth));

      if (i < (kUiTrackCount - 1))
        trackPanels.push_back(separator());
    }
  }

  // Build mini strip (always shown in Focused mode so all 5 tracks are visible)
  Element trackSection;
  if (layoutMode == TrackLayoutMode::Focused) {
    std::vector<MiniTrackInfo> miniInfos;
    constexpr char kLetters[] = {'D', 'B', 'C', 'L', 'M'};
    for (int i = 0; i < kUiTrackCount; ++i) {
      miniInfos.push_back(MiniTrackInfo{
          i,
          (i == selectedTrack),
          (i == kMasterTrackIndex) ? false : appState.tracks[i].muted.load(std::memory_order_relaxed),
          kLetters[i]});
    }
    trackSection = vbox({
        window(text(" Tracks ") | color(Theme::Inactive),
               MiniTrackStrip(miniInfos)),
        hbox(std::move(trackPanels)) | flex,
    });
  } else {
    trackSection = hbox(std::move(trackPanels));
  }

  // ── Arrangement panel (dedicated bordered window) ──────────────────────────
  const uint8_t sectionCount = static_cast<uint8_t>(std::max<uint8_t>(
      appState.arrangementSectionCount.load(std::memory_order_relaxed), 1));
  const uint8_t currentSection = static_cast<uint8_t>(std::min<uint8_t>(
      appState.arrangementCurrentSection.load(std::memory_order_relaxed),
      static_cast<uint8_t>(sectionCount - 1)));
  const uint8_t mode =
      appState.arrangementMode.load(std::memory_order_relaxed);
  const uint8_t sectionLength =
      appState.getArrangementSectionLength(currentSection);
  const uint8_t sectionProgression =
      appState.getArrangementSectionProgression(currentSection);
  const uint8_t sectionTrackMask =
      appState.getArrangementSectionTrackMask(currentSection);
  const bool chainEnabled =
      appState.arrangementChainEnabled.load(std::memory_order_relaxed);
  const uint8_t chainLength = appState.getArrangementChainLength();
  std::array<uint8_t, AppState::kArrangementMaxSections> chainSequence{};
  for (uint8_t step = 0; step < AppState::kArrangementMaxSections; ++step) {
    chainSequence[step] = appState.getArrangementChainStep(step);
  }
  const std::string chainStatus = arrangementChainLabel(
      chainEnabled, sectionCount, chainSequence, chainLength);

  const std::string progLabel =
      (sectionProgression == AppState::kArrangementProgressionFollowGlobal)
          ? "Global"
          : std::to_string(static_cast<int>(sectionProgression) + 1);

  // Build arrangement content as labeled cells
  auto arrCell = [](const std::string& label, const std::string& val) -> Element {
    return hbox({
        text(label) | color(Theme::Inactive),
        text(val)   | bold,
    });
  };

  Element arrangementContent;
  if (terminalWidth >= 100) {
    // Wide: single row hbox of cells separated by " │ "
    arrangementContent = hbox({
        arrCell("Mode: ",   arrangementModeLabel(mode)),
        text("  │  ") | color(Theme::TrackBorder),
        arrCell("Sect: ",   std::to_string(static_cast<int>(currentSection) + 1) + "/" + std::to_string(static_cast<int>(sectionCount))),
        text("  │  ") | color(Theme::TrackBorder),
        arrCell("Len: ",    std::to_string(static_cast<int>(sectionLength)) + " bars"),
        text("  │  ") | color(Theme::TrackBorder),
        arrCell("Prog: ",   progLabel),
        text("  │  ") | color(Theme::TrackBorder),
        arrCell("Tracks: ", arrangementTrackMaskLabel(sectionTrackMask)),
        text("  │  ") | color(Theme::TrackBorder),
        arrCell("Chain: ",  chainStatus),
        filler(),
    });
  } else {
    // Narrow: 2-column grid
    arrangementContent = vbox({
        hbox({
            arrCell("Mode: ",   arrangementModeLabel(mode))  | size(WIDTH, EQUAL, terminalWidth / 2 - 2),
            arrCell("Sect: ",   std::to_string(static_cast<int>(currentSection) + 1) + "/" + std::to_string(static_cast<int>(sectionCount))),
        }),
        hbox({
            arrCell("Len: ",    std::to_string(static_cast<int>(sectionLength)) + " bars") | size(WIDTH, EQUAL, terminalWidth / 2 - 2),
            arrCell("Prog: ",   progLabel),
        }),
        hbox({
            arrCell("Tracks: ", arrangementTrackMaskLabel(sectionTrackMask)) | size(WIDTH, EQUAL, terminalWidth / 2 - 2),
            arrCell("Chain: ",  chainStatus),
        }),
    });
  }
  auto arrangementPanel = window(
      text(" Arrangement ") | color(Theme::Highlight),
      arrangementContent);

  // ── MIDI / Spectrum analyzer ───────────────────────────────────────────────
  const int analyzerInnerWidth = std::max(14, terminalWidth - 4);
  const size_t compactAnalyzerWidth = static_cast<size_t>(analyzerInnerWidth);

  Element analyzerSection;
  if (showAnalyzer) {
    ftxui::Element analyzerPanel;
    if (selectedTrack == kMasterTrackIndex) {
      const auto compactSpectrumRows = buildCompactSpectrumRows(
          currentMeters, compactAnalyzerWidth, analyzerHeight);
      Elements spectrumRows;
      for (int row = 0; row < analyzerHeight; ++row) {
        auto spectrumRowColor = Theme::Highlight;
        if (row >= (analyzerHeight * 2) / 3) {
          spectrumRowColor = Theme::MeterLow;
        } else if (row >= analyzerHeight / 3) {
          spectrumRowColor = Theme::MeterMid;
        } else {
          spectrumRowColor = Theme::MeterHigh;
        }
        spectrumRows.push_back(
            text(compactSpectrumRows[static_cast<size_t>(row)]) |
            color(spectrumRowColor));
      }
      analyzerPanel = window(text(" Spectrum ") | color(Theme::Inactive),
                             vbox(std::move(spectrumRows)));
    } else {
      const auto midiRows =
          buildMidiViewRows(compactAnalyzerWidth, analyzerHeight,
                            static_cast<uint8_t>(selectedTrack));
      Elements midiElements;
      ftxui::Color noteColor = (selectedTrack < 4)
          ? Theme::NoteColors[selectedTrack]
          : Theme::Highlight;
      for (int row = 0; row < analyzerHeight; ++row) {
        midiElements.push_back(text(midiRows[static_cast<size_t>(row)]) |
                               color(noteColor));
      }
      analyzerPanel = window(text(" MIDI Notes ") | color(noteColor),
                             vbox(std::move(midiElements)));
    }
    analyzerSection = analyzerPanel;
  } else {
    analyzerSection = text("") | size(HEIGHT, EQUAL, 0);
  }

  // ── Status line ───────────────────────────────────────────────────────────
  auto statusLine =
      text(uiStatusMessage.empty() ? "Ready" : uiStatusMessage) |
      color(uiStatusIsError ? Theme::Error : Theme::Active);

  Elements statusRows;
  statusRows.push_back(statusLine);
  if (currentMeters.performanceProfileValid) {
    std::ostringstream perf;
    perf << std::fixed << std::setprecision(2) << "Perf "
         << "avg " << currentMeters.callbackMsAvg << "ms"
         << " peak " << currentMeters.callbackMsPeak << "ms"
         << " util " << currentMeters.callbackUtilizationAvg << "%/"
         << currentMeters.callbackUtilizationPeak << "%"
         << " cmd " << currentMeters.commandsMsAvg << "ms"
         << " gen " << currentMeters.generationMsAvg << "ms"
         << " fx " << currentMeters.trackFxMsAvg << "ms"
         << " master " << currentMeters.masterFxMsAvg << "ms"
         << " meter " << currentMeters.meteringMsAvg << "ms"
         << " window " << currentMeters.profileWindowCallbacks;
    statusRows.push_back(text(perf.str()) | color(Theme::Inactive));
  }

  // ── Action log (min 3 rows, compress from top) ────────────────────────────
  constexpr size_t maxLogRows = 3;

  size_t logStart = (undoStack.size() >= maxLogRows) ? undoStack.size() - maxLogRows : 0;
  Elements logRows;
  for (size_t i = logStart; i < undoStack.size(); ++i) {
    logRows.push_back(text(" " + undoStack[i].description) |
                      color(Theme::Inactive));
  }
  if (logRows.empty()) {
    logRows.push_back(text(" (No recent actions)") | color(Theme::Inactive));
  }
  while (logRows.size() < maxLogRows) {
    logRows.push_back(text(""));
  }
  auto logPanel = window(text(" Action Log ") | color(Theme::Inactive),
                         vbox(std::move(logRows)));

  const std::string agentText =
      agentInputActive ? ("mu> " + agentInput + "|")
                       : ("mu> " + (agentStatus.empty() ? "ready"
                                                        : agentStatus));
  auto agentInputBar = AgentInputBar(agentText, agentInputActive);

  // ── Assemble main window ──────────────────────────────────────────────────
  // NOTE: analyzer is last in the body so FTXUI clips/hides it before status,
  // log, and the fixed agent command footer.
  Elements mainContent;
  mainContent.push_back(topBar);
  mainContent.push_back(projectLine);
  mainContent.push_back(separator());
  mainContent.push_back(trackSection);
  mainContent.push_back(separator());
  mainContent.push_back(arrangementPanel);
  mainContent.push_back(separator());
  mainContent.push_back(vbox(std::move(statusRows)));
  mainContent.push_back(logPanel);
  if (showAnalyzer) {
    mainContent.push_back(analyzerSection | size(HEIGHT, EQUAL, analyzerHeight + 2));
  }

  auto mainWindow =
      window(text(" cendance  [Press ? for Help] ") | bold,
             vbox({
                 vbox(std::move(mainContent)) | flex,
                 separator(),
                 agentInputBar,
             }));

  ftxui::Element composed = mainWindow;
  if (showHelp) {
    const int helpMaxHeight = std::max(8, terminalHeight - 4);

    // All help lines as raw strings
    static constexpr auto kHelpLines = std::to_array<const char*>({
      " Transport / Session: ",
      "   <Space>    : Play / Pause",
      "   <H>        : Stop + Reset to Bar 1 Beat 1",
      "   <+> / <->  : Tempo Up / Down",
      "   <M>        : Toggle Metronome",
      "",
      " Musical Setup: ",
      "   <C>        : Number Select Chord Progression",
      "   <K>        : Set Project Key (text input)",
      "   <G>        : Number Select Genre",
      "",
      " Track Selection: ",
      "   <1,2,3,4,5>: Select Track (Drums, Bass, Chords, Lead, Master)",
      "   <Tab> / <Shift+Tab>: Next / Previous Track",
      "",
      " Track Content: ",
      "   <N>        : Toggle Mute for Selected Track",
      "   <A>        : Number Select Algorithm (Selected Track)",
      "   <S>        : Number Select Sound (Selected Track)",
      "   <O> / <P>  : Prev / Next Algorithm",
      "   <9> / <0>  : Prev / Next Sound Preset (Drums/Bass/Chords/Lead)",
      "   <F>        : Number Select Insert FX Preset (Selected Track)",
      "",
      " Mix / Performance Controls: ",
      "   <]> / <[>  : Gain Up / Down",
      "   <Left>     : Decrease Density",
      "   <Right>    : Increase Density",
      "   <Up>       : Increase Complexity",
      "   <Down>     : Decrease Complexity",
      "   <,> / <.>  : Tone Down / Up (Drums/Bass/Chords/Lead)",
      "   <;> / <'>  : Movement Down / Up (Drums/Bass/Chords/Lead)",
      "",
      " Arrangement / Groove: ",
      "   <R>        : Arrangement Editor",
      "   <j> / <u>  : Arrangement Section Prev / Next",
      "   <V>        : Arrangement Mode Cycle (Manual/Auto/Mixed)",
      "   <Q>        : Groove / Swing Editor",
      "",
      " Spot FX: ",
      "   <B>        : Toggle Spot Tape Brake",
      "   <X>        : Toggle Spot Stutter",
      "",
      " Projects / Files: ",
      "   <W>        : Quick Save to Active Project Path",
      "   <Ctrl+S>   : Quick Save",
      "   <w>        : Save Project As (.cendance)",
      "   <L>        : Load Project (.cendance)",
      "   <I>        : Drum Sample Modal (Drums track only)",
      "   <D>        : Sound File Browser",
      "",
      " Utility / Help: ",
      "   <Z>        : Undo last action",
      "   <Esc>      : Close popup / Quit app",
      "   <Up/Down> / <PageUp/Dn>: Scroll help popup",
      "",
      " Number Selector: ",
      "   Type ID (1-based), preview updates live, Enter to apply",
      "   FX format: [slot][category][preset], e.g. 101",
      "   FX clear: [slot]-, e.g. 1-  (10/20/30 are invalid)",
      "   Spot FX presets are trigger-only (not slot assignable)",
      "   Invalid ID + Enter keeps popup open and resets input",
      "   Master track supports FX-only assignment",
    });

    // Title = 1 row, top/bottom border = 2 rows → content area = helpMaxHeight - 3
    const int contentRows = helpMaxHeight - 3;
    const int totalContent = static_cast<int>(kHelpLines.size()) + 1; // +1 for footer
    const int maxScroll = std::max(0, totalContent - contentRows);
    helpScrollOffset = std::clamp(helpScrollOffset, 0, maxScroll);

    const int startRow = helpScrollOffset;
    const int endRow = std::min(startRow + contentRows, static_cast<int>(kHelpLines.size()));

    Elements helpBodyElements;
    for (int i = startRow; i < endRow; ++i) {
      helpBodyElements.push_back(text(kHelpLines[i]));
    }
    // Footer line
    if (maxScroll > 0) {
      std::string footer = " Up/Down or PageUp/Dn: scroll. ";
      if (endRow < static_cast<int>(kHelpLines.size())) {
        footer += std::to_string(static_cast<int>((static_cast<float>(startRow + contentRows) / totalContent) * 100)) + "%/100%";
      } else {
        footer += "End";
      }
      helpBodyElements.push_back(
          text(footer) | center | color(Theme::Active));
    } else {
      helpBodyElements.push_back(
          text(" Press ? or Esc to close this help screen. ") |
              center | color(Theme::Active));
    }

    auto helpContent =
        window(
            text(" Keyboard Controls ") | bold | center,
            vbox(std::move(helpBodyElements)) |
                size(HEIGHT, EQUAL, helpMaxHeight)) |
        clear_under | center;
    composed = dbox({composed, helpContent});
  }

  if (!spotEffectPopupMessage.empty()) {
    auto popup = window(
                     text(" Spot FX ") | bold | center,
                     vbox({
                         text(""),
                         text(spotEffectPopupMessage) | bold | center |
                             color(Theme::Highlight),
                         text(""),
                     }) |
                         size(WIDTH, GREATER_THAN, 28)) |
                 clear_under | center;
    composed = dbox({composed, popup});
  }

  if (numberSelectionOpen) {
    auto modal = NumberSelectionModal(
        numberSelectionDomain, numberSelectionInput, numberSelectionPreview,
        numberSelectionValid, getNumberSelectionImplementedCount(),
        numberSelectionStatus);
    composed = dbox({composed, modal});
  }

  if (keySelectionOpen) {
    auto modal = KeyEntryModal(keySelectionInput, keySelectionPreview,
                               keySelectionValid, keySelectionStatus);
    composed = dbox({composed, modal});
  }

  if (arrangementModalOpen) {
    const uint8_t sectionIndex = arrangementModalCurrentSection;
    const uint8_t sectionProgression =
        arrangementModalSectionProgressions[sectionIndex];
    const bool followGlobal =
        sectionProgression == AppState::kArrangementProgressionFollowGlobal;
    const uint8_t progressionDisplayId =
        followGlobal ? 0 : static_cast<uint8_t>(sectionProgression + 1);

    auto modal = ArrangementModal(
        arrangementModalSectionCount, arrangementModalCurrentSection,
        arrangementModalSectionLengths[sectionIndex], followGlobal,
        progressionDisplayId,
        followGlobal ? "" : arrangementProgressionLabel(sectionProgression),
        arrangementModalSectionTrackMasks[sectionIndex],
        arrangementModalSectionParametersEnabled,
        arrangementModalParameterTrack,
        arrangementModalParameterIndex,
        arrangementModalSectionTrackParameters[sectionIndex][arrangementModalParameterTrack][arrangementModalParameterIndex],
        arrangementPresetLabel(arrangementModalPresetIndex),
        chainStatus,
        arrangementModalFocus, arrangementModalStatus,
        arrangementModalStatusIsError);
    composed = dbox({composed, modal});
  }

  if (grooveModalOpen) {
    auto modal = GrooveModal(
        appState.getSwingAmount(),
        appState.getVelocityHumanize(),
        appState.getTimingJitter(),
        grooveModalFocus,
        grooveModalStatus,
        grooveModalStatusIsError);
    composed = dbox({composed, modal});
  }

  if (projectPathModalOpen) {
    auto modal = ProjectPathModal(
        projectPathModalMode, projectDirectoryInput, projectNameInput,
        projectPathInput, saveProjectFieldFocus, recentProjectPaths,
        selectedRecentProjectIndex, projectPathStatus,
        projectPathStatusIsError);
    composed = dbox({composed, modal});
  }

  if (onboardingTipsOpen) {
    auto modal = ftxui::window(
        ftxui::text(" Quick Tips ") | ftxui::bold | ftxui::center,
        ftxui::vbox({
            ftxui::text("Press ? to open the full keybinds/help screen.") |
                ftxui::color(Theme::ModalForeground),
            ftxui::text("Use 1-5 to select Drums, Bass, Chords, Lead, or Master.") |
                ftxui::color(Theme::ModalForeground),
            ftxui::text("Press A/S/F for algorithm, sound, and FX selectors.") |
                ftxui::color(Theme::ModalForeground),
            ftxui::text("Press R for arrangement, Q for groove, K for key.") |
                ftxui::color(Theme::ModalForeground),
            ftxui::separator(),
            ftxui::text("Enter, Space, or Esc to start.") |
                ftxui::color(Theme::Active),
        }) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 58)) |
        ftxui::clear_under | ftxui::center;
    composed = dbox({composed, modal});
  }

  if (drumSampleModalOpen) {
    ftxui::Elements sampleRows;
    sampleRows.push_back(
        ftxui::text("Slot: " + std::to_string(selectedDrumSlot + 1) +
                    " (1=Kick 2=Snare 3=CHat 4=OHat)") |
        ftxui::color(Theme::Highlight));
    sampleRows.push_back(
        ftxui::text("Global library: " +
                    (drumSampleLibrary != nullptr
                         ? drumSampleLibrary->getGlobalSampleDirectory()
                         : "(unavailable)")) |
        ftxui::color(Theme::Inactive));
    sampleRows.push_back(ftxui::separator());

    sampleRows.push_back(ftxui::text("Path import:") | ftxui::bold);
    const std::string pathField =
        drumSamplePathInput.empty() ? "|" : (drumSamplePathInput + "|");
    sampleRows.push_back(
        ftxui::text(pathField) |
        ftxui::color(drumSampleModalFocus == DrumSampleModalFocus::Path
                         ? Theme::Active
                         : Theme::Foreground));
    sampleRows.push_back(ftxui::separator());

    sampleRows.push_back(ftxui::text("Loaded samples:") | ftxui::bold);
    if (drumSampleEntries.empty()) {
      sampleRows.push_back(ftxui::text("(none)") |
                           ftxui::color(Theme::Inactive));
    } else {
      const int maxRows =
          std::min<int>(8, static_cast<int>(drumSampleEntries.size()));
      for (int i = 0; i < maxRows; ++i) {
        const auto &entry = drumSampleEntries[static_cast<size_t>(i)];
        const bool selected = (i == selectedDrumSampleIndex);
        const std::string prefix = selected ? "> " : "  ";
        sampleRows.push_back(
            ftxui::text(prefix + "#" + std::to_string(entry.id) + " " +
                        entry.name) |
            ftxui::color(selected ? Theme::Highlight : Theme::Foreground));
      }
    }

    sampleRows.push_back(ftxui::separator());
    sampleRows.push_back(ftxui::text("Slot assignments:") | ftxui::bold);
    for (uint8_t slot = 0; slot < AppState::TrackState::DrumSampleSlotCount;
         ++slot) {
      const std::string slotName =
          std::string(DrumSampleCatalog::getSlotDefinition(slot).name);
      const std::string value = getDrumSlotSampleLabel(slot);
      const std::string line =
          std::to_string(slot + 1) + " " + slotName + " -> " + value;
      sampleRows.push_back(ftxui::text(line) |
                           ftxui::color(slot == selectedDrumSlot
                                            ? Theme::Active
                                            : Theme::Foreground));
    }

    sampleRows.push_back(ftxui::separator());
    const float volume =
        appState.tracks[0].getDrumSampleSlotVolume(selectedDrumSlot);
    const float tune =
        appState.tracks[0].getDrumSampleSlotTuneSemitones(selectedDrumSlot);
    const float startOffset =
        appState.tracks[0].getDrumSampleSlotStartOffset(selectedDrumSlot);
    const float decay =
        appState.tracks[0].getDrumSampleSlotDecay(selectedDrumSlot);
    const float velSens =
        appState.tracks[0].getDrumSampleSlotVelocitySensitivity(
            selectedDrumSlot);
    sampleRows.push_back(ftxui::text("Params: vol=" + std::to_string(volume) +
                                     " tune=" + std::to_string(tune) +
                                     " start=" + std::to_string(startOffset) +
                                     " decay=" + std::to_string(decay) +
                                     " vel=" + std::to_string(velSens)) |
                         ftxui::color(Theme::Inactive));

    const std::string defaultStatus =
        "Enter(import/assign)  Tab(path/list)  A(assign) C(clear) R(rescan)\n"
        "Q/F vol  W/S tune  E/D start  T/G decay  Y/H velocity";
    sampleRows.push_back(
        ftxui::text(drumSampleStatus.empty() ? defaultStatus
                                             : drumSampleStatus) |
        ftxui::color(drumSampleStatusIsError ? Theme::Error
                                             : Theme::Foreground));

    auto modal = ftxui::window(ftxui::text(" Drum Sample Manager ") |
                                   ftxui::bold | ftxui::center,
                               ftxui::vbox(std::move(sampleRows))) |
                 ftxui::clear_under | ftxui::center;
    composed = dbox({composed, modal});
  }

  if (soundFileBrowserOpen) {
    ftxui::Elements rows;
    rows.push_back(ftxui::text("Slot: " + std::to_string(selectedDrumSlot + 1) +
                               " (1=Kick 2=Snare 3=CHat 4=OHat)") |
                   ftxui::color(Theme::Highlight));
    rows.push_back(ftxui::separator());
    rows.push_back(ftxui::text("Name                         Fmt  Rate   Ch  Dur   Sender") |
                   ftxui::bold);
    if (downloadedSampleEntries.empty()) {
      rows.push_back(ftxui::text("(no downloaded samples)") |
                     ftxui::color(Theme::Inactive));
    } else {
      const int maxRows = std::min<int>(10, static_cast<int>(downloadedSampleEntries.size()));
      for (int i = 0; i < maxRows; ++i) {
        const auto &entry = downloadedSampleEntries[static_cast<size_t>(i)];
        const bool selected = i == selectedDownloadedSampleIndex;
        std::ostringstream line;
        std::string name = entry.display_name.empty() ? entry.preset_id : entry.display_name;
        if (name.size() > 28) name = name.substr(0, 25) + "...";
        line << (selected ? "> " : "  ")
             << std::left << std::setw(28) << name
             << " " << std::setw(4) << entry.format
             << " " << std::setw(6) << entry.sample_rate
             << " " << std::setw(2) << entry.channels
             << " " << std::fixed << std::setprecision(1) << std::setw(5) << entry.duration
             << " " << (entry.verified ? "verified" : "untrusted");
        rows.push_back(ftxui::text(line.str()) |
                       ftxui::color(selected ? Theme::Highlight : Theme::Foreground));
      }
    }
    rows.push_back(ftxui::separator());
    rows.push_back(ftxui::text(soundFileBrowserStatus.empty()
                                   ? "Enter(import)  1-4(slot)  R(refresh)  Esc(close)"
                                   : soundFileBrowserStatus) |
                   ftxui::color(soundFileBrowserStatusIsError ? Theme::Error : Theme::Foreground));
    auto modal = ftxui::window(ftxui::text(" Sound File Browser ") |
                                   ftxui::bold | ftxui::center,
                               ftxui::vbox(std::move(rows))) |
                 ftxui::clear_under | ftxui::center;
    composed = dbox({composed, modal});
  }

  // Algorithm Editor Modal
  if (algorithmEditorOpen) {
    ftxui::Elements editorRows;
    editorRows.push_back(ftxui::text("Custom Algorithm Editor") | ftxui::bold | ftxui::center);
    editorRows.push_back(ftxui::separator());

    // Step pattern grid
    std::string stepRow = "Steps: ";
    for (uint8_t s = 0; s < algorithmEditorStepCount && s < 32; ++s) {
      if (s == algorithmEditorSelectedStep)
        stepRow += "[";
      else
        stepRow += " ";
      if (s < algorithmEditorDraft.rhythmicPattern.size() && algorithmEditorDraft.rhythmicPattern[s])
        stepRow += "X";
      else
        stepRow += ".";
      if (s == algorithmEditorSelectedStep)
        stepRow += "]";
      else
        stepRow += " ";
    }
    editorRows.push_back(ftxui::text(stepRow) | ftxui::color(Theme::Highlight));

    // Parameter display
    std::ostringstream paramLine;
    paramLine << "Len:" << std::fixed << std::setprecision(2) << algorithmEditorDraft.noteLength
              << " Swing:" << algorithmEditorDraft.swing
              << " Vel:" << static_cast<int>(algorithmEditorDraft.velocityRange.first)
              << "-" << static_cast<int>(algorithmEditorDraft.velocityRange.second);
    editorRows.push_back(ftxui::text(paramLine.str()) | ftxui::color(Theme::Inactive));

    // Custom algorithm list
    if (!cachedCustomAlgorithms.empty()) {
      editorRows.push_back(ftxui::separator());
      editorRows.push_back(ftxui::text("Saved Custom Algorithms:") | ftxui::color(Theme::Highlight));
      for (int i = 0; i < static_cast<int>(cachedCustomAlgorithms.size()) && i < 10; ++i) {
        const auto& algo = cachedCustomAlgorithms[i];
        std::string line = (i == selectedCustomAlgorithmIndex ? "> " : "  ") +
                           std::to_string(i + 1) + ". " + algo.name;
        if (!algo.author.empty())
            line += " by " + algo.author;
        editorRows.push_back(ftxui::text(line) |
            ftxui::color(i == selectedCustomAlgorithmIndex ? Theme::Highlight : Theme::Foreground));
      }
    }

    editorRows.push_back(ftxui::separator());
    editorRows.push_back(ftxui::text(algorithmEditorStatus.empty()
        ? "S(save)  D(delete)  T(toggle step)  +/- (step count)  Esc(close)"
        : algorithmEditorStatus) |
        ftxui::color(algorithmEditorStatusIsError ? Theme::Error : Theme::Foreground));

    auto modal = ftxui::window(ftxui::text(" Algorithm Editor ") | ftxui::bold | ftxui::center,
                               ftxui::vbox(std::move(editorRows))) |
                 ftxui::clear_under | ftxui::center;
    composed = dbox({composed, modal});
  }

  return composed;

}
