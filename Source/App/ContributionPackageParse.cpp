#include "ContributionPackageParse.h"

#include "AlgorithmCatalog.h"
#include "ProjectKey.h"
#include "SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace ContributionPackage {
namespace {

constexpr uint64_t kFnvOffset = 14695981039346656037ull;
constexpr uint64_t kFnvPrime = 1099511628211ull;
constexpr size_t kMaxManifestBytes = 1024u * 1024u;

std::string jsonEscape(const std::string& text) {
    std::ostringstream out;
    for (const char ch : text) {
        switch (ch) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(ch))
                    << std::dec << std::setfill(' ');
            } else {
                out << ch;
            }
            break;
        }
    }
    return out.str();
}



bool hasPathTraversal(const std::filesystem::path& path) {
    if (path.is_absolute()) {
        return true;
    }
    for (const auto& part : path) {
        if (part == "..") {
            return true;
        }
    }
    return false;
}


std::string fnv1a64(const std::string& text) {
    uint64_t hash = kFnvOffset;
    for (const unsigned char ch : text) {
        hash ^= static_cast<uint64_t>(ch);
        hash *= kFnvPrime;
    }
    std::ostringstream out;
    out << "fnv1a64:" << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

std::string canonicalForHash(std::string text) {
    const auto findString = [](const std::string& source, const std::string& key) {
        const std::string needle = "\"" + key + "\"";
        const size_t keyPos = source.find(needle);
        if (keyPos == std::string::npos) {
            return std::pair<size_t, size_t>{std::string::npos, 0};
        }
        const size_t colon = source.find(':', keyPos + needle.size());
        if (colon == std::string::npos) {
            return std::pair<size_t, size_t>{std::string::npos, 0};
        }
        size_t quote = source.find('"', colon + 1);
        if (quote == std::string::npos) {
            return std::pair<size_t, size_t>{std::string::npos, 0};
        }
        ++quote;
        bool escaping = false;
        for (size_t i = quote; i < source.size(); ++i) {
            const char ch = source[i];
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                return std::pair<size_t, size_t>{quote, i - quote};
            }
        }
        return std::pair<size_t, size_t>{std::string::npos, 0};
    };

    const auto [hashStart, hashLength] = findString(text, "contentHash");
    if (hashStart != std::string::npos) {
        text.replace(hashStart, hashLength, "");
    }
    const auto [signatureStart, signatureLength] = findString(text, "signature");
    if (signatureStart != std::string::npos) {
        text.replace(signatureStart, signatureLength, "");
    }
    return text;
}

bool readString(const juce::DynamicObject& object,
                const juce::Identifier& key,
                std::string& out,
                std::string& error,
                bool required = true) {
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        if (required) {
            error = key.toString().toStdString() + " is missing.";
            return false;
        }
        return true;
    }
    if (!value.isString()) {
        error = key.toString().toStdString() + " must be a string.";
        return false;
    }
    out = value.toString().toStdString();
    return true;
}

bool readFloat(const juce::DynamicObject& object,
               const juce::Identifier& key,
               float& out,
               std::string& error,
               bool required = false) {
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        if (required) {
            error = key.toString().toStdString() + " is missing.";
            return false;
        }
        return true;
    }
    if (!value.isDouble() && !value.isInt() && !value.isInt64()) {
        error = key.toString().toStdString() + " must be numeric.";
        return false;
    }
    out = static_cast<float>(static_cast<double>(value));
    if (!std::isfinite(out)) {
        error = key.toString().toStdString() + " must be finite.";
        return false;
    }
    return true;
}

bool readUInt(const juce::DynamicObject& object,
              const juce::Identifier& key,
              uint32_t& out,
              std::string& error,
              bool required = false) {
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        if (required) {
            error = key.toString().toStdString() + " is missing.";
            return false;
        }
        return true;
    }
    if (!value.isInt() && !value.isInt64()) {
        error = key.toString().toStdString() + " must be an integer.";
        return false;
    }
    const int64_t raw = static_cast<int64_t>(value);
    if (raw < 0 || raw > std::numeric_limits<uint32_t>::max()) {
        error = key.toString().toStdString() + " is out of range.";
        return false;
    }
    out = static_cast<uint32_t>(raw);
    return true;
}

bool readDouble(const juce::DynamicObject& object,
                const juce::Identifier& key,
                double& out,
                std::string& error,
                bool required = false) {
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        if (required) {
            error = key.toString().toStdString() + " is missing.";
            return false;
        }
        return true;
    }
    if (!value.isDouble() && !value.isInt() && !value.isInt64()) {
        error = key.toString().toStdString() + " must be numeric.";
        return false;
    }
    out = static_cast<double>(value);
    if (!std::isfinite(out)) {
        error = key.toString().toStdString() + " must be finite.";
        return false;
    }
    return true;
}

bool readBool(const juce::DynamicObject& object,
              const juce::Identifier& key,
              bool& out,
              std::string& error,
              bool required = false) {
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        if (required) {
            error = key.toString().toStdString() + " is missing.";
            return false;
        }
        return true;
    }
    if (!value.isBool()) {
        error = key.toString().toStdString() + " must be a boolean.";
        return false;
    }
    out = static_cast<bool>(value);
    return true;
}

std::vector<std::string> readStringArray(const juce::DynamicObject& object,
                                         const juce::Identifier& key,
                                         std::string& error) {
    std::vector<std::string> result;
    const juce::var value = object.getProperty(key);
    if (value.isVoid()) {
        return result;
    }
    auto* array = value.getArray();
    if (array == nullptr) {
        error = key.toString().toStdString() + " must be an array.";
        return {};
    }
    for (const auto& entry : *array) {
        if (!entry.isString()) {
            error = key.toString().toStdString() + " entries must be strings.";
            return {};
        }
        result.push_back(entry.toString().toStdString());
    }
    return result;
}

std::optional<float> readOptionalFloat(const juce::DynamicObject& object,
                                       const juce::Identifier& key,
                                       float minValue,
                                       float maxValue,
                                       std::string& error) {
    if (object.getProperty(key).isVoid()) {
        return std::nullopt;
    }
    float value = 0.0f;
    if (!readFloat(object, key, value, error, true)) {
        return std::nullopt;
    }
    if (value < minValue || value > maxValue) {
        error = key.toString().toStdString() + " is out of range.";
        return std::nullopt;
    }
    return value;
}

bool parseMacros(const juce::DynamicObject& object,
                 MacroDefaults& macros,
                 std::string& error) {
    auto* macroObject = object.getProperty("macros").getDynamicObject();
    if (macroObject == nullptr) {
        return true;
    }
    macros.density = readOptionalFloat(*macroObject, "density", 0.0f, 1.0f, error);
    if (!error.empty()) return false;
    macros.complexity = readOptionalFloat(*macroObject, "complexity", 0.0f, 1.0f, error);
    if (!error.empty()) return false;
    macros.tone = readOptionalFloat(*macroObject, "tone", 0.0f, 1.0f, error);
    if (!error.empty()) return false;
    macros.motion = readOptionalFloat(*macroObject, "motion", 0.0f, 1.0f, error);
    if (!error.empty()) return false;
    macros.gain = readOptionalFloat(*macroObject, "gain", 0.0f, 2.0f, error);
    return error.empty();
}

std::optional<uint8_t> parseTrackIndex(const juce::DynamicObject& object, std::string& error) {
    uint32_t track = 0;
    if (!readUInt(object, "track", track, error, true)) {
        return std::nullopt;
    }
    if (track < 1 || track > AppState::kTrackCount) {
        error = "track is out of range.";
        return std::nullopt;
    }
    return static_cast<uint8_t>(track - 1);
}

bool parseFxSlots(const juce::DynamicObject& object,
                  std::array<uint16_t, 3>& fxPresetIds,
                  std::array<std::optional<PresetRefs::PresetRef>, 3>* fxPresetRefs,
                  std::string& error) {
    const juce::var value = object.getProperty("fx");
    if (value.isVoid()) {
        return true;
    }
    auto* array = value.getArray();
    if (array == nullptr || array->size() > 3) {
        error = "fx must be an array with at most 3 entries.";
        return false;
    }
    for (int i = 0; i < array->size(); ++i) {
        const juce::var entry = (*array)[i];
        uint32_t displayId = 0;
        if (entry.isInt() || entry.isInt64()) {
            const int64_t raw = static_cast<int64_t>(entry);
            if (raw < 1 || raw > std::numeric_limits<uint16_t>::max()) {
                error = "fx display id is out of range.";
                return false;
            }
            displayId = static_cast<uint32_t>(raw);
        } else {
            auto* fxObject = entry.getDynamicObject();
            if (fxObject == nullptr) {
                error = error.empty() ? "fx entries must be ids or objects." : error;
                return false;
            }
            const std::string refText = fxObject->getProperty("ref").toString().toStdString();
            if (!refText.empty()) {
                const auto ref = PresetRefs::parseStableString(refText);
                if (!ref.has_value() || ref->domain != PresetRefs::Domain::Effect) {
                    error = "fx ref is invalid.";
                    return false;
                }
                if (fxPresetRefs != nullptr) {
                    (*fxPresetRefs)[static_cast<size_t>(i)] = *ref;
                }
                continue;
            }
            if (!readUInt(*fxObject, "effectDisplayId", displayId, error, true)) {
                error = error.empty() ? "fx entries must be ids, refs, or objects." : error;
                return false;
            }
        }
        const auto presetId = EffectPresetCatalog::displayIdToPresetId(static_cast<uint16_t>(displayId));
        if (!EffectPresetCatalog::isValidPresetId(presetId)) {
            error = "fx display id is invalid.";
            return false;
        }
        fxPresetIds[static_cast<size_t>(i)] = presetId;
    }
    return true;
}

std::optional<EffectPresetCatalog::EffectType> effectTypeFromString(const std::string& text) {
    const std::string lower = lowerCopy(text);
#define CENDANCE_EFFECT_NAME(name) if (lower == lowerCopy(#name)) return EffectPresetCatalog::EffectType::name
    CENDANCE_EFFECT_NAME(None);
    CENDANCE_EFFECT_NAME(TapeStop);
    CENDANCE_EFFECT_NAME(BeatRepeat);
    CENDANCE_EFFECT_NAME(HighPassSweep);
    CENDANCE_EFFECT_NAME(ReverbWash);
    CENDANCE_EFFECT_NAME(ReduxCrush);
    CENDANCE_EFFECT_NAME(DelayEcho);
    CENDANCE_EFFECT_NAME(SaturationWaveshaper);
    CENDANCE_EFFECT_NAME(SoftHardClip);
    CENDANCE_EFFECT_NAME(Wavefolder);
    CENDANCE_EFFECT_NAME(AsymShaper);
    CENDANCE_EFFECT_NAME(CompressorGlue);
    CENDANCE_EFFECT_NAME(PeakLimiter);
    CENDANCE_EFFECT_NAME(TransientShaper);
    CENDANCE_EFFECT_NAME(CombFilter);
    CENDANCE_EFFECT_NAME(MultiModeEQ);
    CENDANCE_EFFECT_NAME(FormantFilter);
    CENDANCE_EFFECT_NAME(Autopan);
    CENDANCE_EFFECT_NAME(RingModulator);
    CENDANCE_EFFECT_NAME(Chorus);
    CENDANCE_EFFECT_NAME(Phaser);
    CENDANCE_EFFECT_NAME(Flanger);
    CENDANCE_EFFECT_NAME(JitterDegrade);
    CENDANCE_EFFECT_NAME(ErosionDegrade);
    CENDANCE_EFFECT_NAME(TranceGate);
    CENDANCE_EFFECT_NAME(SidechainDucker);
    CENDANCE_EFFECT_NAME(BeatRepeatInsert);
    CENDANCE_EFFECT_NAME(FrequencyShifter);
    CENDANCE_EFFECT_NAME(PitchShifter);
    CENDANCE_EFFECT_NAME(Harmonizer);
    CENDANCE_EFFECT_NAME(TimeFreezer);
    CENDANCE_EFFECT_NAME(GrainDelay);
    CENDANCE_EFFECT_NAME(PhysicalModelingResonator);
#undef CENDANCE_EFFECT_NAME
    return std::nullopt;
}

std::string effectTypeToString(EffectPresetCatalog::EffectType type) {
    switch (type) {
    case EffectPresetCatalog::EffectType::None: return "None";
    case EffectPresetCatalog::EffectType::TapeStop: return "TapeStop";
    case EffectPresetCatalog::EffectType::BeatRepeat: return "BeatRepeat";
    case EffectPresetCatalog::EffectType::HighPassSweep: return "HighPassSweep";
    case EffectPresetCatalog::EffectType::ReverbWash: return "ReverbWash";
    case EffectPresetCatalog::EffectType::ReduxCrush: return "ReduxCrush";
    case EffectPresetCatalog::EffectType::DelayEcho: return "DelayEcho";
    case EffectPresetCatalog::EffectType::SaturationWaveshaper: return "SaturationWaveshaper";
    case EffectPresetCatalog::EffectType::SoftHardClip: return "SoftHardClip";
    case EffectPresetCatalog::EffectType::Wavefolder: return "Wavefolder";
    case EffectPresetCatalog::EffectType::AsymShaper: return "AsymShaper";
    case EffectPresetCatalog::EffectType::CompressorGlue: return "CompressorGlue";
    case EffectPresetCatalog::EffectType::PeakLimiter: return "PeakLimiter";
    case EffectPresetCatalog::EffectType::TransientShaper: return "TransientShaper";
    case EffectPresetCatalog::EffectType::CombFilter: return "CombFilter";
    case EffectPresetCatalog::EffectType::MultiModeEQ: return "MultiModeEQ";
    case EffectPresetCatalog::EffectType::FormantFilter: return "FormantFilter";
    case EffectPresetCatalog::EffectType::Autopan: return "Autopan";
    case EffectPresetCatalog::EffectType::RingModulator: return "RingModulator";
    case EffectPresetCatalog::EffectType::Chorus: return "Chorus";
    case EffectPresetCatalog::EffectType::Phaser: return "Phaser";
    case EffectPresetCatalog::EffectType::Flanger: return "Flanger";
    case EffectPresetCatalog::EffectType::JitterDegrade: return "JitterDegrade";
    case EffectPresetCatalog::EffectType::ErosionDegrade: return "ErosionDegrade";
    case EffectPresetCatalog::EffectType::TranceGate: return "TranceGate";
    case EffectPresetCatalog::EffectType::SidechainDucker: return "SidechainDucker";
    case EffectPresetCatalog::EffectType::BeatRepeatInsert: return "BeatRepeatInsert";
    case EffectPresetCatalog::EffectType::FrequencyShifter: return "FrequencyShifter";
    case EffectPresetCatalog::EffectType::PitchShifter: return "PitchShifter";
    case EffectPresetCatalog::EffectType::Harmonizer: return "Harmonizer";
    case EffectPresetCatalog::EffectType::TimeFreezer: return "TimeFreezer";
    case EffectPresetCatalog::EffectType::GrainDelay: return "GrainDelay";
    case EffectPresetCatalog::EffectType::PhysicalModelingResonator: return "PhysicalModelingResonator";
    case EffectPresetCatalog::EffectType::MultibandOtt: return "MultibandOtt";
    case EffectPresetCatalog::EffectType::ConvolutionReverb: return "ConvolutionReverb";
    case EffectPresetCatalog::EffectType::TapeDelay: return "TapeDelay";
    case EffectPresetCatalog::EffectType::PingPongDelay: return "PingPongDelay";
    case EffectPresetCatalog::EffectType::CloudGenerator: return "CloudGenerator";
    case EffectPresetCatalog::EffectType::SpectralBlur: return "SpectralBlur";
    case EffectPresetCatalog::EffectType::SpectralDelay: return "SpectralDelay";
    case EffectPresetCatalog::EffectType::CompositeCategory: return "CompositeCategory";
    }
    return "Unknown";
}


bool validateCommonItem(const juce::DynamicObject& object,
                        std::string& itemId,
                        std::string& name,
                        std::string& description,
                        std::vector<std::string>& tags,
                        std::string& error) {
    if (!readString(object, "id", itemId, error, true)
        || !validateIdText(itemId, "item id", error)
        || !readString(object, "name", name, error, true)
        || !readString(object, "description", description, error, false)) {
        return false;
    }
    tags = readStringArray(object, "tags", error);
    return error.empty();
}

bool parseEffectItem(const juce::DynamicObject& object, EffectPresetItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)) {
        return false;
    }
    std::string type;
    if (!readString(object, "effectType", type, error, true)) {
        return false;
    }
    const auto parsedType = effectTypeFromString(type);
    if (!parsedType.has_value() || *parsedType == EffectPresetCatalog::EffectType::CompositeCategory) {
        error = "effectType is unsupported.";
        return false;
    }
    item.type = *parsedType;
    if (!readFloat(object, "paramA", item.paramA, error, false)
        || !readFloat(object, "paramB", item.paramB, error, false)
        || !readFloat(object, "paramC", item.paramC, error, false)) {
        return false;
    }
    uint32_t basedOn = 0;
    const std::string basedOnRef = object.getProperty("basedOnEffectRef").toString().toStdString();
    if (!basedOnRef.empty()) {
        const auto ref = PresetRefs::parseStableString(basedOnRef);
        if (!ref.has_value() || ref->domain != PresetRefs::Domain::Effect) {
            error = "basedOnEffectRef is invalid.";
            return false;
        }
        item.basedOnEffectRef = *ref;
    }
    if (!object.getProperty("basedOnEffectDisplayId").isVoid()) {
        if (!readUInt(object, "basedOnEffectDisplayId", basedOn, error, true)) {
            return false;
        }
        const auto presetId = EffectPresetCatalog::displayIdToPresetId(static_cast<uint16_t>(basedOn));
        if (!EffectPresetCatalog::isValidPresetId(presetId)) {
            error = "basedOnEffectDisplayId is invalid.";
            return false;
        }
        item.basedOnEffectPresetId = presetId;
    }
    return true;
}

bool parseSoundItem(const juce::DynamicObject& object, SoundPresetItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)) {
        return false;
    }
    const auto track = parseTrackIndex(object, error);
    if (!track.has_value()) {
        return false;
    }
    item.trackIndex = *track;
    const std::string soundRefText = object.getProperty("soundRef").toString().toStdString();
    if (!soundRefText.empty()) {
        const auto ref = PresetRefs::parseStableString(soundRefText);
        if (!ref.has_value() || ref->domain != PresetRefs::Domain::Sound) {
            error = "soundRef is invalid.";
            return false;
        }
        item.soundRef = *ref;
    }
    uint32_t soundDisplayId = 0;
    if (!item.soundRef.has_value()
        && (!readUInt(object, "soundDisplayId", soundDisplayId, error, true)
            || !SynthCatalog::isValidDisplayIdForTrack(item.trackIndex, static_cast<uint16_t>(soundDisplayId)))) {
        error = error.empty() ? "soundDisplayId is out of range for this track." : error;
        return false;
    }
    if (!item.soundRef.has_value()) {
        item.synthPresetId = static_cast<uint8_t>(SynthCatalog::displayIdToPresetId(static_cast<uint16_t>(soundDisplayId)));
    }
    return parseFxSlots(object, item.fxPresetIds, &item.fxPresetRefs, error) && parseMacros(object, item.macros, error);
}

bool parseDrumKitItem(const juce::DynamicObject& object, DrumKitPresetItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)) {
        return false;
    }
    auto* slots = object.getProperty("slots").getArray();
    if (slots == nullptr || slots->size() != static_cast<int>(item.slots.size())) {
        error = "drum kit slots must have 4 entries.";
        return false;
    }
    for (int i = 0; i < slots->size(); ++i) {
        auto* slotObject = (*slots)[i].getDynamicObject();
        if (slotObject == nullptr) {
            error = "drum kit slot entries must be objects.";
            return false;
        }
        auto& slot = item.slots[static_cast<size_t>(i)];
        uint32_t sampleId = 0;
        if (!readUInt(*slotObject, "sampleId", sampleId, error, true) || sampleId > Command::kDrumSampleIdMask) {
            error = error.empty() ? "sampleId is out of range." : error;
            return false;
        }
        slot.sampleId = static_cast<uint16_t>(sampleId);
        if (!readFloat(*slotObject, "volume", slot.volume, error, false)
            || !readFloat(*slotObject, "tuneSemitones", slot.tuneSemitones, error, false)
            || !readFloat(*slotObject, "startOffset", slot.startOffset, error, false)
            || !readFloat(*slotObject, "decay", slot.decay, error, false)
            || !readFloat(*slotObject, "velocitySensitivity", slot.velocitySensitivity, error, false)) {
            return false;
        }
        if (slot.volume < 0.0f || slot.volume > 2.0f
            || slot.tuneSemitones < -24.0f || slot.tuneSemitones > 24.0f
            || slot.startOffset < 0.0f || slot.startOffset > 0.95f
            || slot.decay < 0.0f || slot.decay > 1.0f
            || slot.velocitySensitivity < 0.0f || slot.velocitySensitivity > 1.0f) {
            error = "drum kit slot parameters are out of range.";
            return false;
        }
    }
    return parseFxSlots(object, item.fxPresetIds, &item.fxPresetRefs, error);
}

bool parseArrangementFields(const juce::DynamicObject& object, ArrangementPresetItem& item, std::string& error) {
    uint32_t sectionCount = item.sectionCount;
    uint32_t currentSection = item.currentSection + 1;
    uint32_t mode = item.mode;
    if (!readUInt(object, "sectionCount", sectionCount, error, false)
        || !readUInt(object, "currentSection", currentSection, error, false)
        || !readUInt(object, "mode", mode, error, false)) {
        return false;
    }
    if (sectionCount < 1 || sectionCount > AppState::kArrangementMaxSections
        || currentSection < 1 || currentSection > sectionCount
        || mode > AppState::kArrangementModeMixed) {
        error = "arrangement section or mode values are out of range.";
        return false;
    }
    item.sectionCount = static_cast<uint8_t>(sectionCount);
    item.currentSection = static_cast<uint8_t>(currentSection - 1);
    item.mode = static_cast<uint8_t>(mode);

    const auto parseByteArray = [&](const char* key, auto& target, uint8_t minValue, uint8_t maxValue) {
        const juce::var value = object.getProperty(key);
        if (value.isVoid()) {
            return true;
        }
        auto* array = value.getArray();
        if (array == nullptr || array->size() > static_cast<int>(target.size())) {
            error = std::string(key) + " must be an array with supported length.";
            return false;
        }
        for (int i = 0; i < array->size(); ++i) {
            if (!(*array)[i].isInt() && !(*array)[i].isInt64()) {
                error = std::string(key) + " entries must be integers.";
                return false;
            }
            const int64_t raw = static_cast<int64_t>((*array)[i]);
            if (raw < minValue || raw > maxValue) {
                error = std::string(key) + " entry is out of range.";
                return false;
            }
            target[static_cast<size_t>(i)] = static_cast<uint8_t>(raw);
        }
        return true;
    };

    if (!parseByteArray("sectionLengths", item.sectionLengths,
                        AppState::kArrangementMinSectionLengthBars,
                        AppState::kArrangementMaxSectionLengthBars)
        || !parseByteArray("sectionProgressions", item.sectionProgressions,
                           0, AppState::kArrangementProgressionFollowGlobal)
        || !parseByteArray("trackMasks", item.trackMasks,
                           0, AppState::kArrangementTrackMaskAll)) {
        return false;
    }

    readBool(object, "chainEnabled", item.chainEnabled, error, false);
    if (!error.empty()) return false;
    uint32_t chainLength = item.chainLength;
    if (!readUInt(object, "chainLength", chainLength, error, false)) {
        return false;
    }
    if (chainLength < 1 || chainLength > AppState::kArrangementMaxSections) {
        error = "chainLength is out of range.";
        return false;
    }
    item.chainLength = static_cast<uint8_t>(chainLength);
    return parseByteArray("chainSequence", item.chainSequence, 0, AppState::kArrangementMaxSections - 1);
}

bool parseArrangementItem(const juce::DynamicObject& object, ArrangementPresetItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)) {
        return false;
    }
    return parseArrangementFields(object, item, error);
}

bool parseSceneItem(const juce::DynamicObject& object, ScenePresetItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)) {
        return false;
    }
    item.bpm = readOptionalFloat(object, "bpm", 20.0f, 260.0f, error);
    if (!error.empty()) return false;

    uint32_t value = 0;
    if (!object.getProperty("progressionDisplayId").isVoid()) {
        if (!readUInt(object, "progressionDisplayId", value, error, true)
            || value < 1 || value > ChordProgression::getNumProgressions()) {
            error = error.empty() ? "progressionDisplayId is out of range." : error;
            return false;
        }
        item.chordProgression = static_cast<uint8_t>(value - 1);
    }
    std::string keyText;
    if (!object.getProperty("key").isVoid()) {
        if (!readString(object, "key", keyText, error, true)) {
            return false;
        }
    ProjectKey::ParsedValue parsed;
        if (!ProjectKey::parse(keyText, parsed)) {
            error = "key is invalid.";
            return false;
        }
        item.projectKeyRoot = parsed.root;
        item.projectKeyMode = parsed.mode;
    }
    parseFxSlots(object, item.masterFxPresetIds, &item.masterFxPresetRefs, error);
    if (!error.empty()) return false;
    item.hasMasterFx = !object.getProperty("fx").isVoid();
    item.masterGain = readOptionalFloat(object, "masterGain", 0.0f, AppState::kMaxMasterGain, error);
    if (!error.empty()) return false;

    auto* tracks = object.getProperty("tracks").getArray();
    if (tracks != nullptr) {
        for (const auto& entry : *tracks) {
            auto* trackObject = entry.getDynamicObject();
            if (trackObject == nullptr) {
                error = "scene tracks entries must be objects.";
                return false;
            }
            const auto track = parseTrackIndex(*trackObject, error);
            if (!track.has_value()) {
                return false;
            }
            auto& sceneTrack = item.tracks[*track];
            item.hasTrack[*track] = true;
            uint32_t displayId = 0;
            if (!trackObject->getProperty("algorithmDisplayId").isVoid()) {
                if (!readUInt(*trackObject, "algorithmDisplayId", displayId, error, true)
                    || !AlgorithmCatalog::isValidDisplayIdForTrack(*track, static_cast<uint16_t>(displayId))) {
                    error = error.empty() ? "algorithmDisplayId is out of range." : error;
                    return false;
                }
                sceneTrack.algorithmId = static_cast<uint8_t>(AlgorithmCatalog::displayIdToAlgorithmId(static_cast<uint16_t>(displayId)));
            }
            if (!trackObject->getProperty("soundDisplayId").isVoid()) {
                if (!readUInt(*trackObject, "soundDisplayId", displayId, error, true)
                    || !SynthCatalog::isValidDisplayIdForTrack(*track, static_cast<uint16_t>(displayId))) {
                    error = error.empty() ? "soundDisplayId is out of range." : error;
                    return false;
                }
                sceneTrack.synthPresetId = static_cast<uint8_t>(SynthCatalog::displayIdToPresetId(static_cast<uint16_t>(displayId)));
            }
            bool muted = false;
            if (!trackObject->getProperty("muted").isVoid()) {
                if (!readBool(*trackObject, "muted", muted, error, true)) {
                    return false;
                }
                sceneTrack.hasMuted = true;
                sceneTrack.muted = muted;
            }
            if (!parseFxSlots(*trackObject, sceneTrack.fxPresetIds, &sceneTrack.fxPresetRefs, error)
                || !parseMacros(*trackObject, sceneTrack.macros, error)) {
                return false;
            }
        }
    }
    auto* arrangementObject = object.getProperty("arrangement").getDynamicObject();
    if (arrangementObject != nullptr) {
        ArrangementPresetItem arrangement;
        arrangement.itemId = item.itemId + ".arrangement";
        arrangement.name = item.name + " Arrangement";
        if (!parseArrangementFields(*arrangementObject, arrangement, error)) {
            return false;
        }
        item.arrangement = arrangement;
    }
    return true;
}

bool parseSamplePackItem(const juce::DynamicObject& object, SamplePackItem& item, std::string& error) {
    if (!validateCommonItem(object, item.itemId, item.name, item.description, item.tags, error)
        || !readString(object, "filePath", item.filePath, error, true)
        || !readString(object, "format", item.format, error, true)
        || !readString(object, "sha256", item.sha256, error, true)) {
        return false;
    }
    uint32_t channels = 0;
    if (!readUInt(object, "sampleRate", item.sampleRate, error, true)
        || !readUInt(object, "channels", channels, error, true)
        || !readDouble(object, "duration", item.duration, error, true)) {
        return false;
    }
    const std::filesystem::path relative(item.filePath);
    const std::string extension = lowerCopy(relative.extension().string());
    item.format = lowerCopy(item.format);
    if (hasPathTraversal(relative)) {
        error = "sample pack filePath is unsafe.";
        return false;
    }
    if ((extension != ".wav" && extension != ".flac")
        || (item.format != "wav" && item.format != "flac")) {
        error = "sample pack items support only wav or flac files.";
        return false;
    }
    if (item.sampleRate == 0 || channels == 0 || channels > std::numeric_limits<uint16_t>::max()
        || item.duration <= 0.0 || item.sha256.empty()) {
        error = "sample pack metadata is incomplete.";
        return false;
    }
    item.channels = static_cast<uint16_t>(channels);
    return true;
}







}

// --- Public API ---

std::string quoted(const std::string& text) {
    return "\"" + jsonEscape(text) + "\"";
}


std::string lowerCopy(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}


bool validateIdText(const std::string& text, const std::string& label, std::string& error) {
    if (text.empty() || text.size() > 128) {
        error = label + " must be 1-128 characters.";
        return false;
    }
    for (const char ch : text) {
        const bool ok = std::isalnum(static_cast<unsigned char>(ch))
            || ch == '.' || ch == '-' || ch == '_' || ch == ':';
        if (!ok) {
            error = label + " contains unsupported characters.";
            return false;
        }
    }
    return true;
}


bool parsePackage(const std::string& text,
                  const std::string& sourcePath,
                  Preview& preview) {
    juce::var parsed;
    const juce::Result parseResult = juce::JSON::parse(text, parsed);
    if (parseResult.failed()) {
        preview.errors.push_back("Package JSON parse failed: " + parseResult.getErrorMessage().toStdString());
        return false;
    }
    auto* root = parsed.getDynamicObject();
    if (root == nullptr) {
        preview.errors.push_back("Package root must be an object.");
        return false;
    }

    Package package;
    package.sourcePath = sourcePath;
    std::string error;
    if (!readString(*root, "schema", package.schema, error, true)
        || !readString(*root, "id", package.packageId, error, true)
        || !readString(*root, "version", package.version, error, true)
        || !readString(*root, "name", package.name, error, true)
        || !readString(*root, "description", package.description, error, true)
        || !readString(*root, "authorAgent", package.authorAgent, error, true)
        || !readString(*root, "createdAt", package.createdAt, error, true)
        || !readString(*root, "license", package.license, error, true)
        || !readString(*root, "contentHash", package.contentHash, error, false)
        || !readString(*root, "signature", package.signature, error, false)
        || !validateIdText(package.packageId, "package id", error)) {
        preview.errors.push_back(error);
        return false;
    }
    if (package.schema != kSchema) {
        preview.errors.push_back("Unsupported package schema.");
        return false;
    }
    std::string kindText;
    if (!readString(*root, "kind", kindText, error, true)) {
        preview.errors.push_back(error);
        return false;
    }
    const auto kind = kindFromString(kindText);
    if (!kind.has_value()) {
        preview.errors.push_back("Unsupported package kind.");
        return false;
    }
    package.kind = *kind;
    package.signaturePresent = !package.signature.empty();
    if (!package.signaturePresent) {
        preview.warnings.push_back("Package is unsigned; local development only.");
    }
    package.dependencies = readStringArray(*root, "dependencies", error);
    if (!error.empty()) {
        preview.errors.push_back(error);
        return false;
    }
    package.tags = readStringArray(*root, "tags", error);
    if (!error.empty()) {
        preview.errors.push_back(error);
        return false;
    }
    auto* compatibility = root->getProperty("compatibility").getDynamicObject();
    if (compatibility == nullptr) {
        preview.errors.push_back("compatibility is missing.");
        return false;
    }
    readString(*compatibility, "minAppVersion", package.compatibility.minAppVersion, error, false);
    readString(*compatibility, "maxAppVersion", package.compatibility.maxAppVersion, error, false);
    readString(*compatibility, "minPackageSchema", package.compatibility.minPackageSchema, error, false);
    readString(*compatibility, "maxPackageSchema", package.compatibility.maxPackageSchema, error, false);
    if (!error.empty()) {
        preview.errors.push_back(error);
        return false;
    }
    if (package.compatibility.minPackageSchema != kSchema
        || package.compatibility.maxPackageSchema != kSchema) {
        preview.errors.push_back("Package schema compatibility is unsupported.");
        return false;
    }

    const std::string computedHash = fnv1a64(canonicalForHash(text));
    if (package.contentHash.empty()) {
        preview.warnings.push_back("Package has no contentHash; computed " + computedHash + ".");
    } else if (package.contentHash != computedHash) {
        preview.errors.push_back("contentHash mismatch; expected " + package.contentHash + " but computed " + computedHash + ".");
        return false;
    } else {
        package.hashVerified = true;
    }

    auto* payloads = root->getProperty("payloads").getArray();
    if (payloads != nullptr) {
        const std::filesystem::path sourceDir = std::filesystem::path(sourcePath).parent_path();
        for (const auto& payload : *payloads) {
            if (!payload.isString()) {
                preview.errors.push_back("payloads entries must be strings.");
                return false;
            }
            const std::filesystem::path relative(payload.toString().toStdString());
            if (hasPathTraversal(relative)) {
                preview.errors.push_back("payload path is unsafe.");
                return false;
            }
            const std::filesystem::path fullPath = sourceDir / relative;
            if (!std::filesystem::exists(fullPath)) {
                preview.errors.push_back("payload path does not exist: " + relative.string());
                return false;
            }
        }
    }

    auto* items = root->getProperty("items").getArray();
    if (items == nullptr || items->isEmpty()) {
        preview.errors.push_back("items must be a non-empty array.");
        return false;
    }
    for (const auto& entry : *items) {
        auto* itemObject = entry.getDynamicObject();
        if (itemObject == nullptr) {
            preview.errors.push_back("items entries must be objects.");
            return false;
        }
        switch (package.kind) {
        case Kind::EffectPresetPack: {
            EffectPresetItem item;
            if (!parseEffectItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.effectPresets.push_back(item);
            break;
        }
        case Kind::SoundPresetPack: {
            SoundPresetItem item;
            if (!parseSoundItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.soundPresets.push_back(item);
            break;
        }
        case Kind::DrumKitPresetPack: {
            DrumKitPresetItem item;
            if (!parseDrumKitItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.drumKitPresets.push_back(item);
            break;
        }
        case Kind::ArrangementPresetPack: {
            ArrangementPresetItem item;
            if (!parseArrangementItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.arrangementPresets.push_back(item);
            break;
        }
        case Kind::ScenePresetPack: {
            ScenePresetItem item;
            if (!parseSceneItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.scenePresets.push_back(item);
            break;
        }
        case Kind::SamplePack: {
            SamplePackItem item;
            if (!parseSamplePackItem(*itemObject, item, error)) {
                preview.errors.push_back(error);
                return false;
            }
            package.samplePacks.push_back(item);
            break;
        }
        }
    }

    if (package.kind == Kind::SamplePack) {
        const std::filesystem::path sourceDir = std::filesystem::path(sourcePath).parent_path();
        for (const auto& item : package.samplePacks) {
            const std::filesystem::path relative(item.filePath);
            const std::filesystem::path fullPath = sourceDir / relative;
            const std::filesystem::path installedPayloadPath =
                sourceDir.parent_path() / "Payloads" / package.packageId / relative;
            if ((!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
                && (!std::filesystem::exists(installedPayloadPath) || !std::filesystem::is_regular_file(installedPayloadPath))) {
                preview.errors.push_back("sample pack payload does not exist: " + item.filePath);
                return false;
            }
        }
    }

    preview.package = package;
    preview.ok = true;
    return true;
}


std::string readFile(const std::string& path, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "Package file could not be opened.";
        return {};
    }
    in.seekg(0, std::ios::end);
    const auto size = in.tellg();
    if (size < 0 || static_cast<size_t>(size) > kMaxManifestBytes) {
        error = "Package manifest exceeds the supported size limit.";
        return {};
    }
    in.seekg(0, std::ios::beg);
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}


std::string defaultRootDirectory() {
    if (const char* overrideDir = std::getenv("CENDANCE_CONTRIBUTIONS_DIR")) {
        if (overrideDir[0] != '\0') {
            return overrideDir;
        }
    }
    if (const char* home = std::getenv("HOME")) {
        return (std::filesystem::path(home) / ".cendance" / "Contributions").string();
    }
    return (std::filesystem::current_path() / ".cendance" / "Contributions").string();
}


std::string packageFileName(const std::string& packageId) {
    std::string safe;
    for (const char ch : packageId) {
        safe.push_back((std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_') ? ch : '_');
    }
    return safe + ".cendance-package.json";
}


void appendJsonStringArray(std::ostringstream& out, const std::vector<std::string>& values) {
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out << ",";
        out << quoted(values[i]);
    }
    out << "]";
}


void appendPackageSummary(std::ostringstream& out, const Package& package, bool includeItems) {
    out << "{"
        << "\"id\":" << quoted(package.packageId)
        << ",\"version\":" << quoted(package.version)
        << ",\"kind\":" << quoted(kindToString(package.kind))
        << ",\"name\":" << quoted(package.name)
        << ",\"description\":" << quoted(package.description)
        << ",\"authorAgent\":" << quoted(package.authorAgent)
        << ",\"license\":" << quoted(package.license)
        << ",\"installed\":" << (package.installed ? "true" : "false")
        << ",\"hashVerified\":" << (package.hashVerified ? "true" : "false")
        << ",\"signaturePresent\":" << (package.signaturePresent ? "true" : "false")
        << ",\"tags\":";
    appendJsonStringArray(out, package.tags);
    if (includeItems) {
        out << ",\"items\":[";
        bool first = true;
        const auto appendItem = [&](const std::string& id, const std::string& name, const std::string& description) {
            if (!first) out << ",";
            first = false;
            out << "{\"id\":" << quoted(id)
                << ",\"name\":" << quoted(name)
                << ",\"description\":" << quoted(description) << "}";
        };
        for (const auto& item : package.effectPresets) appendItem(item.itemId, item.name, item.description);
        for (const auto& item : package.soundPresets) appendItem(item.itemId, item.name, item.description);
        for (const auto& item : package.drumKitPresets) appendItem(item.itemId, item.name, item.description);
        for (const auto& item : package.arrangementPresets) appendItem(item.itemId, item.name, item.description);
        for (const auto& item : package.scenePresets) appendItem(item.itemId, item.name, item.description);
        for (const auto& item : package.samplePacks) {
            if (!first) out << ",";
            first = false;
            out << "{\"id\":" << quoted(item.itemId)
                << ",\"name\":" << quoted(item.name)
                << ",\"description\":" << quoted(item.description)
                << ",\"filePath\":" << quoted(item.filePath)
                << ",\"format\":" << quoted(item.format)
                << ",\"sampleRate\":" << item.sampleRate
                << ",\"channels\":" << item.channels
                << ",\"duration\":" << item.duration
                << ",\"sha256\":" << quoted(item.sha256)
                << ",\"tags\":";
            appendJsonStringArray(out, item.tags);
            out << "}";
        }
        out << "]";
    }
    out << "}";
}

} // namespace ContributionPackage
