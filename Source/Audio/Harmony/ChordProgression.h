#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

struct ChordProgression {
    std::string name;
    std::array<int, 4> degrees; // 0-indexed scale degrees (e.g., 0=I, 1=ii, etc.)
    uint32_t genreTags; // Same bit layout as GenreCatalog: bit 0=genre 1, bit 1=genre 2, etc.

    // Returns a progression by index.
    static const ChordProgression& get(int index);
    static int getNumProgressions();
    static uint32_t getGenreMask(int index);
    static bool hasGenre(int index, uint8_t genreId);

    static bool isValidDisplayId(uint16_t displayId);
    static int displayIdToProgressionIndex(uint16_t displayId);
    static std::string_view getNameByDisplayId(uint16_t displayId);
};
