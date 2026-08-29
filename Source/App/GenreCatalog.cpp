#include "GenreCatalog.h"
#include "AlgorithmPresetRegistry.h"

#include <juce_core/juce_core.h>

#include <sstream>

namespace GenreCatalog {

bool algorithmHasGenreRuntime(uint8_t trackIndex, uint16_t algorithmId, uint8_t genreId) {
    if (trackIndex >= AlgorithmCatalog::kTrackCount || genreId == 0 || genreId >= 32)
        return false;

    // Built-in algorithm: use constexpr lookup
    if (algorithmId < AlgorithmCatalog::kAlgorithmsPerTrack)
        return algorithmHasGenre(trackIndex, algorithmId, genreId);

    // Custom algorithm: resolve through registry
    auto& registry = globalAlgorithmPresetRegistry();
    const auto* preset = registry.findByRuntimeId(trackIndex, algorithmId);
    if (!preset)
        return false;

    // genreTags is a uint32_t bitmask matching the same genre bit layout
    return (preset->genreTags >> (genreId - 1)) & 1;
}

uint32_t getAlgorithmGenreMaskRuntime(uint8_t trackIndex, uint16_t algorithmId) {
    if (trackIndex >= AlgorithmCatalog::kTrackCount)
        return 0;

    // Built-in algorithm: use constexpr lookup
    if (algorithmId < AlgorithmCatalog::kAlgorithmsPerTrack)
        return kAlgorithmGenreMasks[trackIndex][algorithmId];

    // Custom algorithm: resolve through registry
    auto& registry = globalAlgorithmPresetRegistry();
    const auto* preset = registry.findByRuntimeId(trackIndex, algorithmId);
    if (!preset)
        return 0;

    return preset->genreTags;
}

std::string genreMaskToJson(uint32_t mask) {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (uint8_t i = 0; i < 32; ++i) {
        if ((mask >> i) & 1) {
            if (!first) oss << ",";
            oss << (int)(i + 1);
            first = false;
        }
    }
    oss << "]";
    return oss.str();
}

} // namespace GenreCatalog
