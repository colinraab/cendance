#include "TrackPanel.h"
#include "ParameterBar.h"
#include "../Themes/Colors.h"
#include "../../App/AlgorithmCatalog.h"
#include "../../App/AppState.h"
#include "../../App/SynthCatalog.h"
#include <array>
#include <algorithm>

namespace {

constexpr std::array<const char*, 5> kTrackNames = {"DRUMS", "BASS", "CHORDS", "LEAD", "MASTER"};
constexpr std::array<const char*, 5> kTrackLetters = {"D", "B", "C", "L", "M"};

// Build a compact level meter bar of the given width
std::string makeLevelBar(float level, float gain, float maxGain, int width) {
    const int meterFill = static_cast<int>(std::clamp(level, 0.0f, 1.0f) * width);
    const int gainPos   = std::clamp(static_cast<int>((std::clamp(gain, 0.0f, maxGain) / maxGain) * width), 0, width - 1);
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i == gainPos) {
            bar += "│";
        } else {
            bar += (i < meterFill) ? "█" : "░";
        }
    }
    bar += "]";
    return bar;
}

std::string makeDivider(int width) {
    std::string divider;
    divider.reserve(static_cast<size_t>(std::max(1, width)) * 3);
    for (int i = 0; i < std::max(1, width); ++i) {
        divider += "═";
    }
    return divider;
}

} // namespace

ftxui::Element TrackPanel(int index,
                          bool isSelected,
                          uint8_t algoId,
                          uint8_t synthPreset,
                          const std::string& fxSlot1,
                          const std::string& fxSlot2,
                          const std::string& fxSlot3,
                          float density,
                          float complexity,
                          float tone,
                          float motion,
                          bool muted,
                          float gain,
                          float level,
                          TrackLayoutMode layoutMode,
                          int panelInnerWidth) {
    using namespace ftxui;

    const bool isMaster  = (index == 4);
    const bool isCompact = (layoutMode == TrackLayoutMode::Compact);
    const float maxGain = isMaster ? AppState::kMaxMasterGain : AppState::kMaxTrackGain;

    const std::string trackName = std::string("[") + std::to_string(index + 1) + "] " + kTrackNames[index];
    const std::string algoName  = isMaster ? "--"
        : std::string(AlgorithmCatalog::getAlgorithmName(static_cast<uint8_t>(index), algoId));
    const std::string presetName = isMaster ? "--"
        : std::string(SynthCatalog::getPresetName(static_cast<uint8_t>(index), synthPreset));

    const std::string stateStr = isMaster ? "BUS" : (muted ? "MUTED" : "ON");

    auto title = text(trackName) | bold;
    if (!isMaster && index < 4) {
        title |= color(Theme::TrackLabelColors[index]);
    }
    auto divLine = text(makeDivider(panelInnerWidth)) | color(Theme::TrackBorder);

    if (isCompact) {
        // Compact mode: abbreviated labels, bars sized to track width
        // Label "Dn:" = 3 chars, brackets "[...]" = 2 chars overhead → barW = inner - 5
        const int barW    = std::max(1, panelInnerWidth - 5);
        const int meterW  = std::max(1, panelInnerWidth - 5); // "Lv:" + brackets
        auto levelBar = text("Lv:" + makeLevelBar(level, gain, maxGain, meterW)) |
                        color(Theme::TrackSelected);
        auto content = vbox({
            title,
            divLine,
            text("Al:" + algoName)  | ftxui::xflex_shrink,
            text("Sn:" + presetName) | ftxui::xflex_shrink,
            text("St:" + stateStr),
            text("F1:" + fxSlot1)   | ftxui::xflex_shrink,
            text("F2:" + fxSlot2)   | ftxui::xflex_shrink,
            text("F3:" + fxSlot3)   | ftxui::xflex_shrink,
            levelBar,
            ParameterBar("Dn", isMaster ? 0.0f : density,    barW),
            ParameterBar("Cm", isMaster ? 0.0f : complexity, barW),
            ParameterBar("Tn", (!isMaster) ? tone    : 0.0f, barW),
            ParameterBar("Mv", (!isMaster) ? motion  : 0.0f, barW),
        });
        auto block = window(text(""), content) | flex;
        return isSelected ? block | color(Theme::TrackSelected)
                          : block | color(Theme::Foreground);
    }

    // Full / Focused mode: bars sized to track width
    // Label "Dens: " = 6 chars, brackets "[...]" = 2 chars overhead → barW = inner - 8
    const int barW      = std::max(1, panelInnerWidth - 8);
    const int meterW    = std::max(1, panelInnerWidth - 8); // "Levl: " + brackets
    auto levelBar = text("Levl: " + makeLevelBar(level, gain, maxGain, meterW)) |
                    color(Theme::TrackSelected);

    auto content = vbox({
        title,
        divLine,
        text("Algo: " + algoName)  | ftxui::xflex_shrink,
        text("Syn : " + presetName) | ftxui::xflex_shrink,
        text("State: " + stateStr),
        text("FX1 : " + fxSlot1)   | ftxui::xflex_shrink,
        text("FX2 : " + fxSlot2),
        text("FX3 : " + fxSlot3),
        levelBar,
        ParameterBar("Dens", isMaster ? 0.0f : density,    barW),
        ParameterBar("Cmpl", isMaster ? 0.0f : complexity,  barW),
        ParameterBar("Tone", (!isMaster) ? tone    : 0.0f,  barW),
        ParameterBar("Move", (!isMaster) ? motion  : 0.0f,  barW),
    });


    auto block = window(text(""), content) | flex;

    return isSelected ? block | color(Theme::TrackSelected)
                      : block | color(Theme::Foreground);
}

ftxui::Element MiniTrackStrip(const std::vector<MiniTrackInfo>& tracks) {
    using namespace ftxui;

    // One segment per track: [D] ▶  or  D ●  etc.
    // Selected = brackets + highlight, muted = dim, playing = active dot
    Elements segments;
    for (size_t i = 0; i < tracks.size(); ++i) {
        const auto& t = tracks[i];
        std::string label;
        label += t.letter;
        const std::string stateGlyph = t.muted ? "✕" : "▶";

        Element seg;
        if (t.isSelected) {
            seg = text(std::string("[") + t.letter + "]" + stateGlyph)
                | bold | color(Theme::TrackSelected);
        } else if (t.muted) {
            seg = text(std::string(" ") + t.letter + " " + stateGlyph)
                | color(Theme::Inactive);
        } else {
            ftxui::Color trackColor = Theme::Foreground;
            if (t.index < 4) {
                trackColor = Theme::TrackLabelColors[t.index];
            }
            seg = text(std::string(" ") + t.letter + " " + stateGlyph)
                | color(trackColor);
        }
        segments.push_back(seg);

        if (i + 1 < tracks.size()) {
            segments.push_back(text("│") | color(Theme::TrackBorder));
        }
    }

    return hbox(std::move(segments));
}
