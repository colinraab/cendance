#include "ProjectKey.h"

#include <array>
#include <cctype>

namespace ProjectKey {
namespace {

constexpr std::array<std::string_view, 12> kCanonicalRootNames = {
    "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"
};

int basePitchClassFromLetter(char upperLetter) {
    switch (upperLetter) {
        case 'C': return 0;
        case 'D': return 2;
        case 'E': return 4;
        case 'F': return 5;
        case 'G': return 7;
        case 'A': return 9;
        case 'B': return 11;
        default: return -1;
    }
}

std::string toLower(std::string_view text) {
    std::string lower;
    lower.reserve(text.size());
    for (const char ch : text) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return lower;
}

std::string_view trim(std::string_view text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }

    size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return text.substr(begin, end - begin);
}

} // namespace

bool isValidRoot(uint8_t root) {
    return root < 12;
}

bool isValidMode(uint8_t mode) {
    return mode == kModeMajor || mode == kModeNaturalMinor;
}

uint8_t clampRoot(uint8_t root) {
    return static_cast<uint8_t>(root % 12);
}

uint8_t clampMode(uint8_t mode) {
    return (mode == kModeMajor) ? kModeMajor : kModeNaturalMinor;
}

bool parse(std::string_view input, ParsedValue& outValue) {
    const std::string_view trimmed = trim(input);
    if (trimmed.empty()) {
        return false;
    }

    const char tonic = trimmed[0];
    if (std::isalpha(static_cast<unsigned char>(tonic)) == 0) {
        return false;
    }

    const char tonicUpper = static_cast<char>(std::toupper(static_cast<unsigned char>(tonic)));
    const int basePitchClass = basePitchClassFromLetter(tonicUpper);
    if (basePitchClass < 0) {
        return false;
    }

    const bool inferredMajor = std::isupper(static_cast<unsigned char>(tonic)) != 0;

    size_t cursor = 1;
    int accidentalOffset = 0;
    if (cursor < trimmed.size()) {
        const char accidental = trimmed[cursor];
        if (accidental == '#') {
            accidentalOffset = 1;
            ++cursor;
        } else if (accidental == 'b' || accidental == 'B') {
            accidentalOffset = -1;
            ++cursor;
        }
    }

    while (cursor < trimmed.size()
           && std::isspace(static_cast<unsigned char>(trimmed[cursor])) != 0) {
        ++cursor;
    }

    uint8_t mode = inferredMajor ? kModeMajor : kModeNaturalMinor;
    if (cursor < trimmed.size()) {
        const std::string_view suffix = trimmed.substr(cursor);
        if (suffix == "m") {
            mode = kModeNaturalMinor;
        } else if (suffix == "M") {
            mode = kModeMajor;
        } else {
            const std::string lowerSuffix = toLower(suffix);
            if (lowerSuffix == "min" || lowerSuffix == "minor") {
                mode = kModeNaturalMinor;
            } else if (lowerSuffix == "maj" || lowerSuffix == "major") {
                mode = kModeMajor;
            } else {
                return false;
            }
        }
    }

    const int rootClass = (basePitchClass + accidentalOffset + 12) % 12;
    outValue.root = static_cast<uint8_t>(rootClass);
    outValue.mode = mode;
    return true;
}

std::string format(uint8_t root, uint8_t mode) {
    const uint8_t normalizedRoot = clampRoot(root);
    const uint8_t normalizedMode = clampMode(mode);

    std::string label(kCanonicalRootNames[normalizedRoot]);
    if (normalizedMode == kModeMajor) {
        label += " Major";
    } else {
        label += " Minor";
    }

    return label;
}

} // namespace ProjectKey
