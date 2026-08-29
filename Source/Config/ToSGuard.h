#pragma once

#include <juce_core/juce_core.h>

#include <string>

namespace ToSGuard {

// Returns true if ToS has been accepted (config.json exists with tos_accepted=true).
bool isAccepted();

// Returns the ISO 8601 timestamp stored at acceptance time, or empty string.
std::string acceptedTimestamp();

// Sets tos_accepted=true plus ISO 8601 timestamp and writes config.json.
// Returns true on success.
bool accept();

// Returns the path to the config directory (JUCE userApplicationDataDirectory + "/cendance").
juce::File configDirectory();

// Returns the path to config.json inside the config directory.
juce::File configFile();

// Returns the ToS display text shown in the modal.
std::string tosText();

} // namespace ToSGuard
