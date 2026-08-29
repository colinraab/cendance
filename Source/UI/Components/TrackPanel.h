#pragma once
#include <ftxui/dom/elements.hpp>
#include <string>
#include <cstdint>
#include <vector>

enum class TrackLayoutMode : uint8_t {
    Full    = 0,  // >= 140 cols: all fields, dynamic bars
    Compact = 1,  // 80-139 cols: 5 tracks, abbreviated labels, dynamic bars
    Focused = 2,  // < 80 cols: selected track only + mini strip
};

struct MiniTrackInfo {
    int   index;
    bool  isSelected;
    bool  muted;
    char  letter;  // D B C L M
};

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
                          int panelInnerWidth);

// Renders a compact 1-row strip showing all 5 tracks (for Focused mode)
ftxui::Element MiniTrackStrip(const std::vector<MiniTrackInfo>& tracks);
