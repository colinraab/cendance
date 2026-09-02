#include "AlgorithmPresetRegistry.h"
#include "AlgorithmCatalog.h"
#include "../Config/AppDirectories.h"

#include <algorithm>
#include <fstream>

AlgorithmPresetRegistry::AlgorithmPresetRegistry() {
    rootDir_ = AppDirectories::dataDirectory().getChildFile("algorithms");
    indexFile_ = rootDir_.getChildFile("index.json");
}

AlgorithmPresetRegistry::~AlgorithmPresetRegistry() = default;

juce::File AlgorithmPresetRegistry::rootDirectory() const {
    return rootDir_;
}

juce::File AlgorithmPresetRegistry::indexFile() const {
    return indexFile_;
}

bool AlgorithmPresetRegistry::ensureDirectories(std::string& error) const {
    if (!rootDir_.createDirectory()) {
        error = "Failed to create algorithm directory: " + rootDir_.getFullPathName().toStdString();
        return false;
    }
    return true;
}

bool AlgorithmPresetRegistry::reload(std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& track : trackPresets_)
        track.clear();
    presetIdIndex_.clear();
    loadIndexFromDisk();
    assignRuntimeIds();
    return true;
}

void AlgorithmPresetRegistry::loadIndexFromDisk() {
    if (!indexFile_.existsAsFile())
        return;

    auto content = indexFile_.loadFileAsString();
    if (content.isEmpty())
        return;

    auto parsed = juce::JSON::parse(content);
    if (parsed.isVoid() || !parsed.isObject())
        return;

    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return;

    for (uint8_t track = 0; track < 4; ++track) {
        juce::String key = "track" + juce::String(track);
        if (!obj->hasProperty(key))
            continue;

        auto arr = obj->getProperty(key);
        if (!arr.isArray())
            continue;

        for (auto& v : *arr.getArray()) {
            juce::String filename = v.toString();
            if (filename.isEmpty())
                continue;

            juce::File file = rootDir_.getChildFile(filename);
            CustomAlgorithmPreset preset;
            std::string err;
            if (loadPresetFromFile(file, preset, err)) {
                PresetEntry entry;
                entry.preset = std::move(preset);
                entry.file = file;
                trackPresets_[track].push_back(std::move(entry));
            }
        }
    }
}

bool AlgorithmPresetRegistry::writeIndexToDisk(std::string& error) const {
    auto obj = std::make_unique<juce::DynamicObject>();
    for (uint8_t track = 0; track < 4; ++track) {
        juce::Array<juce::var> arr;
        for (auto& entry : trackPresets_[track]) {
            if (entry.file.existsAsFile())
                arr.add(juce::var(entry.file.getFileName().toStdString()));
        }
        obj->setProperty("track" + juce::String(track), juce::var(arr));
    }

    std::string json = juce::JSON::toString(juce::var(obj.release()), false).toStdString();
    if (!indexFile_.replaceWithText(juce::String(json))) {
        error = "Failed to write index file: " + indexFile_.getFullPathName().toStdString();
        return false;
    }
    return true;
}

bool AlgorithmPresetRegistry::loadPresetFromFile(const juce::File& file, CustomAlgorithmPreset& preset, std::string& error) const {
    auto content = file.loadFileAsString();
    if (content.isEmpty()) {
        error = "Empty file: " + file.getFullPathName().toStdString();
        return false;
    }

    auto parsed = juce::JSON::parse(content);
    if (parsed.isVoid() || !parsed.isObject()) {
        error = "Invalid JSON in: " + file.getFullPathName().toStdString();
        return false;
    }

    auto result = fromJson(content.toStdString(), error);
    if (!result.has_value())
        return false;

    preset = std::move(result.value());
    return true;
}

bool AlgorithmPresetRegistry::writePresetToFile(const CustomAlgorithmPreset& preset, const juce::File& file, std::string& error) const {
    std::string json = toJson(preset);
    if (!file.replaceWithText(juce::String(json))) {
        error = "Failed to write preset file: " + file.getFullPathName().toStdString();
        return false;
    }
    return true;
}

void AlgorithmPresetRegistry::assignRuntimeIds() {
    presetIdIndex_.clear();
    for (uint8_t track = 0; track < 4; ++track) {
        uint16_t runtimeId = kCustomAlgorithmIdBase;
        for (auto& entry : trackPresets_[track]) {
            entry.runtimeId = runtimeId;
            presetIdIndex_[entry.preset.id] = {track, runtimeId};
            ++runtimeId;
        }
    }
}

bool AlgorithmPresetRegistry::savePreset(const CustomAlgorithmPreset& preset, std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (preset.trackIndex > 3) {
        error = "Invalid track index.";
        return false;
    }

    // Validate preset data
    CustomAlgorithmPreset toSave = preset;
    if (!validate(toSave, error))
        return false;

    // Generate ID if not set
    if (toSave.id.empty())
        toSave.id = sanitizeAlgorithmId(toSave.name);

    if (toSave.version.empty())
        toSave.version = "1.0";

    // Check for duplicate ID
    if (presetIdIndex_.count(toSave.id)) {
        error = "Preset with ID '" + toSave.id + "' already exists.";
        return false;
    }

    // Write file
    juce::String filename = juce::String(toSave.id) + ".json";
    juce::File file = rootDir_.getChildFile(filename);
    if (!writePresetToFile(toSave, file, error))
        return false;

    // Add to registry
    PresetEntry entry;
    entry.preset = std::move(toSave);
    entry.file = file;
    trackPresets_[preset.trackIndex].push_back(std::move(entry));
    assignRuntimeIds();

    return writeIndexToDisk(error);
}

bool AlgorithmPresetRegistry::updatePreset(uint16_t algorithmId, const CustomAlgorithmPreset& preset, std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!isCustomAlgorithmId(algorithmId)) {
        error = "Cannot update built-in algorithm.";
        return false;
    }

    uint16_t idx = customRuntimeIndex(algorithmId);
    uint8_t trackIndex = preset.trackIndex;
    if (trackIndex > 3) {
        error = "Invalid track index.";
        return false;
    }

    if (idx >= trackPresets_[trackIndex].size()) {
        error = "Algorithm not found.";
        return false;
    }

    CustomAlgorithmPreset toSave = preset;
    // Preserve original ID
    toSave.id = trackPresets_[trackIndex][idx].preset.id;

    // Write file
    if (!writePresetToFile(toSave, trackPresets_[trackIndex][idx].file, error))
        return false;

    trackPresets_[trackIndex][idx].preset = std::move(toSave);
    assignRuntimeIds();
    return writeIndexToDisk(error);
}

bool AlgorithmPresetRegistry::deletePreset(uint16_t algorithmId, std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!isCustomAlgorithmId(algorithmId)) {
        error = "Cannot delete built-in algorithm.";
        return false;
    }

    uint16_t idx = customRuntimeIndex(algorithmId);

    // Find which track this belongs to
    for (uint8_t track = 0; track < 4; ++track) {
        if (idx < trackPresets_[track].size()) {
            auto& entry = trackPresets_[track][idx];
            if (entry.runtimeId == algorithmId) {
                entry.file.deleteFile();
                trackPresets_[track].erase(trackPresets_[track].begin() + idx);
                assignRuntimeIds();
                return writeIndexToDisk(error);
            }
        }
    }

    error = "Algorithm not found.";
    return false;
}

const CustomAlgorithmPreset* AlgorithmPresetRegistry::findByRuntimeId(uint8_t trackIndex, uint16_t algorithmId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (trackIndex >= 4 || !isCustomAlgorithmId(algorithmId))
        return nullptr;

    uint16_t idx = customRuntimeIndex(algorithmId);
    if (idx >= trackPresets_[trackIndex].size())
        return nullptr;

    return &trackPresets_[trackIndex][idx].preset;
}

const CustomAlgorithmPreset* AlgorithmPresetRegistry::findByPresetId(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = presetIdIndex_.find(id);
    if (it == presetIdIndex_.end())
        return nullptr;

    uint8_t trackIndex = it->second.first;
    uint16_t runtimeId = it->second.second;
    uint16_t idx = customRuntimeIndex(runtimeId);
    if (idx >= trackPresets_[trackIndex].size())
        return nullptr;

    return &trackPresets_[trackIndex][idx].preset;
}

std::vector<CustomAlgorithmPreset> AlgorithmPresetRegistry::listForTrack(uint8_t trackIndex) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<CustomAlgorithmPreset> result;
    if (trackIndex >= 4)
        return result;

    for (auto& entry : trackPresets_[trackIndex])
        result.push_back(entry.preset);

    return result;
}

std::vector<CustomAlgorithmPreset> AlgorithmPresetRegistry::listByGenre(uint8_t trackIndex, uint8_t genreId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<CustomAlgorithmPreset> result;
    if (trackIndex >= 4 || genreId == 0 || genreId >= 32)
        return result;

    uint32_t mask = 1u << (genreId - 1);
    for (auto& entry : trackPresets_[trackIndex]) {
        if ((entry.preset.genreTags & mask) != 0)
            result.push_back(entry.preset);
    }
    return result;
}

std::optional<uint16_t> AlgorithmPresetRegistry::runtimeIdForPresetId(uint8_t trackIndex, const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (trackIndex >= 4)
        return std::nullopt;

    for (auto& entry : trackPresets_[trackIndex]) {
        if (entry.preset.id == id)
            return entry.runtimeId;
    }
    return std::nullopt;
}

uint16_t AlgorithmPresetRegistry::getCustomAlgorithmCountForTrack(uint8_t trackIndex) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (trackIndex >= 4)
        return 0;
    return static_cast<uint16_t>(trackPresets_[trackIndex].size());
}

uint16_t AlgorithmPresetRegistry::getTotalAlgorithmCountForTrack(uint8_t trackIndex) const {
    if (trackIndex >= 4)
        return 0;
    // Returns the total number of algorithms (built-in + custom) for this track.
    // Note: custom IDs are sparse (starting at kCustomAlgorithmIdBase), so this
    // count does not represent a contiguous ID range.
    return static_cast<uint16_t>(AlgorithmCatalog::kAlgorithmsPerTrack + getCustomAlgorithmCountForTrack(trackIndex));
}

bool AlgorithmPresetRegistry::isCustomAlgorithmId(uint16_t algorithmId) const {
    return algorithmId >= kCustomAlgorithmIdBase;
}

uint16_t AlgorithmPresetRegistry::customRuntimeIndex(uint16_t algorithmId) const {
    return algorithmId - kCustomAlgorithmIdBase;
}

std::string AlgorithmPresetRegistry::algorithmMetadataJson(uint8_t trackIndex, uint16_t algorithmId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (trackIndex >= 4)
        return "{}";

    if (!isCustomAlgorithmId(algorithmId)) {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("id", static_cast<int>(algorithmId));
        obj->setProperty("name", juce::String(std::string(AlgorithmCatalog::getAlgorithmName(trackIndex, algorithmId))));
        obj->setProperty("trackIndex", static_cast<int>(trackIndex));
        obj->setProperty("builtin", true);
        return juce::JSON::toString(juce::var(obj.release()), false).toStdString();
    }

    uint16_t idx = customRuntimeIndex(algorithmId);
    if (idx >= trackPresets_[trackIndex].size())
        return "{}";

    auto& preset = trackPresets_[trackIndex][idx].preset;
    auto obj = std::make_unique<juce::DynamicObject>();
    obj->setProperty("id", static_cast<int>(algorithmId));
    obj->setProperty("presetId", juce::String(preset.id));
    obj->setProperty("name", juce::String(preset.name));
    obj->setProperty("author", juce::String(preset.author));
    obj->setProperty("trackIndex", static_cast<int>(trackIndex));
    obj->setProperty("builtin", false);
    obj->setProperty("genreTags", static_cast<int>(preset.genreTags));
    obj->setProperty("version", juce::String(preset.version));
    return juce::JSON::toString(juce::var(obj.release()), false).toStdString();
}

std::string AlgorithmPresetRegistry::catalogJson() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto root = std::make_unique<juce::DynamicObject>();
    for (uint8_t track = 0; track < 4; ++track) {
        juce::Array<juce::var> arr;
        for (auto& entry : trackPresets_[track]) {
            auto obj = std::make_unique<juce::DynamicObject>();
            obj->setProperty("id", static_cast<int>(entry.runtimeId));
            obj->setProperty("presetId", juce::String(entry.preset.id));
            obj->setProperty("name", juce::String(entry.preset.name));
            obj->setProperty("author", juce::String(entry.preset.author));
            obj->setProperty("trackIndex", static_cast<int>(track));
            obj->setProperty("genreTags", static_cast<int>(entry.preset.genreTags));
            obj->setProperty("version", juce::String(entry.preset.version));
            arr.add(juce::var(obj.release()));
        }
        root->setProperty("track" + juce::String(track), juce::var(arr));
    }
    return juce::JSON::toString(juce::var(root.release()), false).toStdString();
}

void AlgorithmPresetRegistry::rebuildIndex() {
    std::lock_guard<std::mutex> lock(mutex_);
    assignRuntimeIds();
    std::string err;
    writeIndexToDisk(err);
}

AlgorithmPresetRegistry& globalAlgorithmPresetRegistry() {
    static AlgorithmPresetRegistry instance;
    return instance;
}
