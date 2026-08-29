#include "CustomAlgorithmPreset.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

static juce::String toJuce(const std::string& s) {
    return juce::String(s);
}

static std::string fromJuce(const juce::String& s) {
    return s.toStdString();
}

static std::string readString(const juce::var& obj, const char* key, const std::string& fallback = "") {
    if (obj.isObject()) {
        auto* dyn = obj.getDynamicObject();
        if (dyn != nullptr && dyn->hasProperty(key))
            return fromJuce(dyn->getProperty(key).toString());
    }
    return fallback;
}

static double readDouble(const juce::var& obj, const char* key, double fallback = 0.0) {
    if (obj.isObject()) {
        auto* dyn = obj.getDynamicObject();
        if (dyn != nullptr && dyn->hasProperty(key))
            return static_cast<double>(dyn->getProperty(key));
    }
    return fallback;
}

static int64_t readInt64(const juce::var& obj, const char* key, int64_t fallback = 0) {
    if (obj.isObject()) {
        auto* dyn = obj.getDynamicObject();
        if (dyn != nullptr && dyn->hasProperty(key))
            return static_cast<int64_t>(dyn->getProperty(key));
    }
    return fallback;
}

static std::vector<std::string> readStringArray(const juce::var& obj, const char* key) {
    std::vector<std::string> result;
    if (obj.isObject()) {
        auto* dyn = obj.getDynamicObject();
        if (dyn != nullptr && dyn->hasProperty(key)) {
            auto arr = dyn->getProperty(key);
            if (arr.isArray()) {
                for (auto& v : *arr.getArray())
                    result.push_back(fromJuce(v.toString()));
            }
        }
    }
    return result;
}

template <typename T>
static std::vector<T> readNumberArray(const juce::var& obj, const char* key) {
    std::vector<T> result;
    if (obj.isObject()) {
        auto* dyn = obj.getDynamicObject();
        if (dyn != nullptr && dyn->hasProperty(key)) {
            auto arr = dyn->getProperty(key);
            if (arr.isArray()) {
                for (auto& v : *arr.getArray())
                    result.push_back(static_cast<T>(static_cast<double>(v)));
                return result;
            }
        }
    }
    return result;
}

static juce::var makeStringArray(const std::vector<std::string>& v) {
    juce::Array<juce::var> arr;
    for (auto& s : v)
        arr.add(juce::var(toJuce(s)));
    return juce::var(arr);
}

template <typename T>
static juce::var makeNumberArray(const std::vector<T>& v) {
    juce::Array<juce::var> arr;
    for (auto& val : v)
        arr.add(juce::var(static_cast<double>(val)));
    return juce::var(arr);
}

bool validate(const CustomAlgorithmPreset& preset, std::string& error) {
    if (preset.name.empty()) {
        error = "Algorithm name is required.";
        return false;
    }
    if (preset.trackIndex > 3) {
        error = "Track index must be 0..3.";
        return false;
    }
    if (preset.stepCount < CustomAlgorithmPreset::kMinStepCount ||
        preset.stepCount > CustomAlgorithmPreset::kMaxStepCount) {
        error = "Step count must be " + std::to_string(CustomAlgorithmPreset::kMinStepCount) +
                ".." + std::to_string(CustomAlgorithmPreset::kMaxStepCount) + ".";
        return false;
    }
    if (preset.noteLength < 0.05f || preset.noteLength > 1.0f) {
        error = "Note length must be 0.05..1.0.";
        return false;
    }
    if (preset.swing < 0.0f || preset.swing > 0.5f) {
        error = "Swing must be 0.0..0.5.";
        return false;
    }
    if (preset.velocityRange.first < 1 || preset.velocityRange.first > 127 ||
        preset.velocityRange.second < 1 || preset.velocityRange.second > 127 ||
        preset.velocityRange.first > preset.velocityRange.second) {
        error = "Velocity range must be 1..127 with min <= max.";
        return false;
    }
    if (preset.octaveRange.first < -4 || preset.octaveRange.first > 4 ||
        preset.octaveRange.second < -4 || preset.octaveRange.second > 4 ||
        preset.octaveRange.first > preset.octaveRange.second) {
        error = "Octave range must be -4..4 with min <= max.";
        return false;
    }
    for (auto interval : preset.melodicPattern) {
        if (interval < CustomAlgorithmPreset::kMinMelodicInterval ||
            interval > CustomAlgorithmPreset::kMaxMelodicInterval) {
            error = "Melodic interval must be " +
                    std::to_string(CustomAlgorithmPreset::kMinMelodicInterval) + ".." +
                    std::to_string(CustomAlgorithmPreset::kMaxMelodicInterval) + ".";
            return false;
        }
    }
    return true;
}

std::string toJson(const CustomAlgorithmPreset& preset) {
    auto obj = std::make_unique<juce::DynamicObject>();
    obj->setProperty("id", toJuce(preset.id));
    obj->setProperty("name", toJuce(preset.name));
    obj->setProperty("description", toJuce(preset.description));
    obj->setProperty("author", toJuce(preset.author));
    obj->setProperty("version", toJuce(preset.version));
    obj->setProperty("createdAt", toJuce(preset.createdAt));
    obj->setProperty("tags", makeStringArray(preset.tags));
    obj->setProperty("trackIndex", static_cast<int>(preset.trackIndex));
    obj->setProperty("noteLength", preset.noteLength);
    obj->setProperty("stepCount", static_cast<int>(preset.stepCount));
    obj->setProperty("swing", preset.swing);
    {
        juce::Array<juce::var> vr;
        vr.add(static_cast<int>(preset.velocityRange.first));
        vr.add(static_cast<int>(preset.velocityRange.second));
        obj->setProperty("velocityRange", juce::var(vr));
    }
    {
        juce::Array<juce::var> or_;
        or_.add(static_cast<int>(preset.octaveRange.first));
        or_.add(static_cast<int>(preset.octaveRange.second));
        obj->setProperty("octaveRange", juce::var(or_));
    }
    obj->setProperty("scaleMask", static_cast<int>(preset.scaleMask));
    obj->setProperty("rhythmicPattern", makeNumberArray(preset.rhythmicPattern));
    obj->setProperty("melodicPattern", makeNumberArray(preset.melodicPattern));
    obj->setProperty("densityCurve", makeNumberArray(preset.densityCurve));
    obj->setProperty("complexityCurve", makeNumberArray(preset.complexityCurve));
    obj->setProperty("genreTags", static_cast<int>(preset.genreTags));
    if (preset.compatibleBuiltinAlgorithmId.has_value())
        obj->setProperty("compatibleBuiltinAlgorithmId", static_cast<int>(preset.compatibleBuiltinAlgorithmId.value()));
    return juce::JSON::toString(juce::var(obj.release()), false).toStdString();
}

std::optional<CustomAlgorithmPreset> fromJson(const std::string& json, std::string& error) {
    auto parsed = juce::JSON::parse(toJuce(json));
    if (parsed.isVoid() || !parsed.isObject()) {
        error = "Invalid JSON object.";
        return std::nullopt;
    }

    CustomAlgorithmPreset preset;
    preset.id = readString(parsed, "id");
    preset.name = readString(parsed, "name");
    preset.description = readString(parsed, "description");
    preset.author = readString(parsed, "author");
    preset.version = readString(parsed, "version", "1.0");
    preset.createdAt = readString(parsed, "createdAt");
    preset.tags = readStringArray(parsed, "tags");
    preset.trackIndex = static_cast<uint8_t>(readInt64(parsed, "trackIndex", 0));
    preset.noteLength = static_cast<float>(readDouble(parsed, "noteLength", 0.75));
    preset.stepCount = static_cast<uint8_t>(readInt64(parsed, "stepCount", CustomAlgorithmPreset::kDefaultStepCount));
    preset.swing = static_cast<float>(readDouble(parsed, "swing", 0.0));
    {
        auto vr = readNumberArray<int64_t>(parsed, "velocityRange");
        if (vr.size() >= 2) {
            preset.velocityRange.first = static_cast<uint8_t>(std::clamp<int64_t>(vr[0], 1, 127));
            preset.velocityRange.second = static_cast<uint8_t>(std::clamp<int64_t>(vr[1], 1, 127));
        }
    }
    {
        auto or_ = readNumberArray<int64_t>(parsed, "octaveRange");
        if (or_.size() >= 2) {
            preset.octaveRange.first = static_cast<int8_t>(std::clamp<int64_t>(or_[0], -4, 4));
            preset.octaveRange.second = static_cast<int8_t>(std::clamp<int64_t>(or_[1], -4, 4));
        }
    }
    preset.scaleMask = static_cast<uint16_t>(readInt64(parsed, "scaleMask", 0x7f));
    preset.rhythmicPattern = readNumberArray<uint8_t>(parsed, "rhythmicPattern");
    preset.melodicPattern = readNumberArray<int8_t>(parsed, "melodicPattern");
    preset.densityCurve = readNumberArray<float>(parsed, "densityCurve");
    preset.complexityCurve = readNumberArray<float>(parsed, "complexityCurve");
    preset.genreTags = static_cast<uint32_t>(readInt64(parsed, "genreTags", 0));

    auto builtinId = readInt64(parsed, "compatibleBuiltinAlgorithmId", -1);
    if (builtinId >= 0)
        preset.compatibleBuiltinAlgorithmId = static_cast<uint16_t>(builtinId);

    // Clamp stepCount BEFORE resizing pattern arrays to avoid OOM on bad input
    preset.stepCount = std::clamp(preset.stepCount,
                                  static_cast<uint8_t>(CustomAlgorithmPreset::kMinStepCount),
                                  static_cast<uint8_t>(CustomAlgorithmPreset::kMaxStepCount));

    // Normalize patterns to stepCount
    preset.rhythmicPattern.resize(static_cast<size_t>(preset.stepCount), 0);
    preset.melodicPattern.resize(static_cast<size_t>(preset.stepCount), 0);
    preset.densityCurve.resize(static_cast<size_t>(preset.stepCount), 1.0f);
    preset.complexityCurve.resize(static_cast<size_t>(preset.stepCount), 1.0f);

    // Clamp remaining values
    preset.noteLength = std::clamp(preset.noteLength, 0.05f, 1.0f);
    preset.swing = std::clamp(preset.swing, 0.0f, 0.5f);
    preset.trackIndex = std::min(preset.trackIndex, static_cast<uint8_t>(3));

    if (!validate(preset, error))
        return std::nullopt;

    return preset;
}

std::string makeStablePresetRef(const CustomAlgorithmPreset& preset) {
    return "local.algorithms/" + preset.id + "/" + preset.version;
}

std::string sanitizeAlgorithmId(const std::string& nameOrId) {
    std::string result;
    result.reserve(nameOrId.size());
    for (char c : nameOrId) {
        if (std::isalnum(static_cast<unsigned char>(c)))
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        else if (c == ' ' || c == '-' || c == '_')
            result += '_';
    }
    if (result.empty())
        result = "custom_algorithm";
    return result;
}
