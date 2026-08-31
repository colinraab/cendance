#pragma once

#include <juce_core/juce_core.h>

#include <cstdlib>

namespace AppDirectories {

inline juce::File dataDirectory() {
    if (const char* overrideDir = std::getenv("CENDANCE_DATA_DIR");
        overrideDir != nullptr && overrideDir[0] != '\0') {
        return juce::File(juce::String(overrideDir));
    }

    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("cendance");
}

}  // namespace AppDirectories
