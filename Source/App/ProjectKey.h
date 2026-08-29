#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ProjectKey {

constexpr uint8_t kModeMajor = 0;
constexpr uint8_t kModeNaturalMinor = 1;

struct ParsedValue {
    uint8_t root = 0;
    uint8_t mode = kModeNaturalMinor;
};

bool isValidRoot(uint8_t root);
bool isValidMode(uint8_t mode);
uint8_t clampRoot(uint8_t root);
uint8_t clampMode(uint8_t mode);

bool parse(std::string_view input, ParsedValue& outValue);
std::string format(uint8_t root, uint8_t mode);

} // namespace ProjectKey
