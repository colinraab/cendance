#include "ChordProgression.h"
#include <initializer_list>
#include <vector>

namespace {

uint32_t genreMask(std::initializer_list<uint8_t> genreIds) {
    uint32_t mask = 0;
    for (uint8_t genreId : genreIds) {
        if (genreId > 0 && genreId <= 32) {
            mask |= (1u << (genreId - 1));
        }
    }
    return mask;
}

const std::vector<ChordProgression>& getProgressions() {
    static const std::vector<ChordProgression> progressions = {
        // A: Trance/Pop (i -> VI -> III -> VII)
        {"Trance/Pop", {0, 5, 2, 6}, genreMask({7, 8})},
        // S: Jazz/Soul (ii -> V -> I -> I) - padded with I to fit 4 bars
        {"Jazz/Soul", {1, 4, 0, 0}, genreMask({5, 8})},
        // D: Pop/Rock (I -> V -> vi -> IV)
        {"Pop/Rock", {0, 4, 5, 3}, genreMask({1, 8})},
        // F: Dark (i -> iv -> v -> i)
        {"Dark", {0, 3, 4, 0}, genreMask({4, 6})},
        // EDM/Trance: i -> VII -> VI -> V
        {"Uplift Gate", {0, 6, 5, 4}, genreMask({6, 7})},
        // EDM/Trance: i -> VI -> iv -> V
        {"Festival Lift", {0, 5, 3, 4}, genreMask({1, 7})},
        // EDM/Trance: i -> V -> VII -> VI
        {"Euphoric Loop", {0, 4, 6, 5}, genreMask({7, 8})},
        // House/Techno: i -> V -> iv -> V
        {"Warehouse Drive", {0, 4, 3, 4}, genreMask({1, 6})},
        // House/Techno: i -> VI -> V -> iv
        {"Deep House Glide", {0, 5, 4, 3}, genreMask({1, 2, 6})},
        // House/Techno: i -> i -> iv -> V
        {"Techno Pulse", {0, 0, 3, 4}, genreMask({3, 6})},
        // Jazz/Neo-soul: ii -> V -> I -> VI
        {"NeoSoul Turnaround", {1, 4, 0, 5}, genreMask({5, 8})},
        // Jazz/Neo-soul: VI -> ii -> V -> I
        {"Midnight Jazz", {5, 1, 4, 0}, genreMask({5})},
        // Jazz/Neo-soul: iv -> V -> I -> I
        {"Soul Resolve", {3, 4, 0, 0}, genreMask({5, 8})},
        // Cinematic/Dark: i -> VII -> iv -> V
        {"Noir Descent", {0, 6, 3, 4}, genreMask({3, 4, 6})},
        // Cinematic/Dark: i -> ii -> iv -> VII
        {"Tension Arc", {0, 1, 3, 6}, genreMask({3, 4, 6})}
    };

    return progressions;
}

} // namespace

const ChordProgression& ChordProgression::get(int index) {
    const auto& progressions = getProgressions();

    if (index >= 0 && index < static_cast<int>(progressions.size())) {
        return progressions[index];
    }
    return progressions[0]; // fallback
}

int ChordProgression::getNumProgressions() {
    return static_cast<int>(getProgressions().size());
}

uint32_t ChordProgression::getGenreMask(int index) {
    return get(index).genreTags;
}

bool ChordProgression::hasGenre(int index, uint8_t genreId) {
    if (genreId == 0 || genreId > 32) {
        return false;
    }

    return (getGenreMask(index) >> (genreId - 1)) & 1u;
}

bool ChordProgression::isValidDisplayId(uint16_t displayId) {
    return displayId >= 1 && displayId <= static_cast<uint16_t>(getNumProgressions());
}

int ChordProgression::displayIdToProgressionIndex(uint16_t displayId) {
    return displayId > 0 ? static_cast<int>(displayId - 1) : 0;
}

std::string_view ChordProgression::getNameByDisplayId(uint16_t displayId) {
    if (!isValidDisplayId(displayId)) {
        return "Invalid";
    }

    return get(displayIdToProgressionIndex(displayId)).name;
}
