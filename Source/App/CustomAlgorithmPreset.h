#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <juce_core/juce_core.h>

struct CustomAlgorithmPreset {
    static constexpr uint8_t kMinStepCount = 1;
    static constexpr uint8_t kDefaultStepCount = 16;
    static constexpr uint8_t kMaxStepCount = 64;
    static constexpr int8_t kMinMelodicInterval = -24;
    static constexpr int8_t kMaxMelodicInterval = 24;

    std::string id;
    std::string name;
    std::string description;
    std::string author;
    std::string version;
    std::string createdAt;
    std::vector<std::string> tags;

    uint8_t trackIndex = 0;
    float noteLength = 0.75f;
    uint8_t stepCount = kDefaultStepCount;
    float swing = 0.0f;
    std::pair<uint8_t, uint8_t> velocityRange{64, 110};
    std::pair<int8_t, int8_t> octaveRange{0, 1};
    uint16_t scaleMask = 0x7f;

    std::vector<uint8_t> rhythmicPattern;
    std::vector<int8_t> melodicPattern;
    std::vector<float> densityCurve;
    std::vector<float> complexityCurve;

    uint32_t genreTags = 0;
    std::optional<uint16_t> compatibleBuiltinAlgorithmId;
};

bool validate(const CustomAlgorithmPreset& preset, std::string& error);
std::string toJson(const CustomAlgorithmPreset& preset);
std::optional<CustomAlgorithmPreset> fromJson(const std::string& json, std::string& error);
std::string makeStablePresetRef(const CustomAlgorithmPreset& preset);
std::string sanitizeAlgorithmId(const std::string& nameOrId);
