#include "ArrangementPresetManager.h"
#include "PresetRef.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

//==============================================================================
// Free helpers (file I/O)
//==============================================================================

static std::string readFileContents(const juce::File& file, std::string& error) {
    if (!file.existsAsFile()) {
        error = "File not found: " + file.getFullPathName().toStdString();
        return "";
    }
    auto content = file.loadFileAsString();
    return content.toStdString();
}

static bool writeFileContents(const juce::File& file, const std::string& content, std::string& error) {
    if (!file.getParentDirectory().createDirectory()) {
        // Directory may already exist, that's fine
    }
    juce::FileOutputStream stream(file);
    if (!stream.openedOk()) {
        error = "Cannot open file for writing: " + file.getFullPathName().toStdString();
        return false;
    }
    stream.setPosition(0);
    stream.truncate();
    stream.write(content.data(), content.size());
    return true;
}

//==============================================================================
// ArrangementPresetManager
//==============================================================================

ArrangementPresetManager::ArrangementPresetManager() {
    rootDir_ = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("cendance")
                   .getChildFile("arrangementPresets");
}

ArrangementPresetManager::~ArrangementPresetManager() = default;

bool ArrangementPresetManager::ensureDirectories(std::string& error) const {
    if (!rootDir_.createDirectory()) {
        error = "Failed to create arrangement presets directory: " + rootDir_.getFullPathName().toStdString();
        return false;
    }
    return true;
}

bool ArrangementPresetManager::reload(std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    presets_.clear();

    if (!rootDir_.exists() && !ensureDirectories(error)) {
        return false;
    }

    auto files = rootDir_.findChildFiles(juce::File::findFiles, false, "*.json");
    for (const auto& file : files) {
        ContributionPackage::ArrangementPresetItem item;
        std::string loadError;
        if (loadPresetFromFile(file, item, loadError)) {
            PresetEntry entry;
            entry.item = std::move(item);
            entry.file = file;
            presets_[entry.item.itemId] = std::move(entry);
        }
    }

    return true;
}

std::string ArrangementPresetManager::savePreset(const AppState& appState, const std::string& name, std::string& error) {
    if (name.empty()) {
        error = "Preset name is required";
        return "";
    }

    auto item = snapshotFromState(appState, name);
    std::string presetId = generatePresetId(name);
    item.itemId = presetId;

    if (!ensureDirectories(error)) {
        return "";
    }

    juce::File file = rootDir_.getChildFile(juce::String(presetId + ".json"));
    if (!writePresetToFile(item, file, error)) {
        return "";
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        PresetEntry entry;
        entry.item = item;
        entry.file = file;
        presets_[presetId] = std::move(entry);
    }

    return presetId;
}

bool ArrangementPresetManager::loadPreset(const std::string& presetId, AppState& appState, std::string& error) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = presets_.find(presetId);
    if (it == presets_.end()) {
        error = "Arrangement preset not found: " + presetId;
        return false;
    }

    applyToState(appState, it->second.item);
    return true;
}

bool ArrangementPresetManager::deletePreset(const std::string& presetId, std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = presets_.find(presetId);
    if (it == presets_.end()) {
        error = "Arrangement preset not found: " + presetId;
        return false;
    }

    it->second.file.deleteFile();
    presets_.erase(it);
    return true;
}

std::vector<ContributionPackage::ArrangementPresetItem> ArrangementPresetManager::listPresets() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContributionPackage::ArrangementPresetItem> result;
    result.reserve(presets_.size());
    for (const auto& [id, entry] : presets_) {
        result.push_back(entry.item);
    }
    return result;
}

const ContributionPackage::ArrangementPresetItem* ArrangementPresetManager::findPreset(const std::string& presetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = presets_.find(presetId);
    if (it != presets_.end()) {
        return &it->second.item;
    }
    return nullptr;
}

ContributionPackage::ArrangementPresetItem ArrangementPresetManager::snapshotFromState(const AppState& appState, const std::string& name) {
    ContributionPackage::ArrangementPresetItem item;
    item.name = name;
    item.description = "Saved arrangement preset";
    item.sectionCount = appState.arrangementSectionCount.load(std::memory_order_relaxed);
    item.currentSection = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
    item.mode = appState.arrangementMode.load(std::memory_order_relaxed);

    for (uint8_t i = 0; i < AppState::kArrangementMaxSections; ++i) {
        item.sectionLengths[i] = appState.getArrangementSectionLength(i);
        item.sectionProgressions[i] = appState.getArrangementSectionProgression(i);
        item.trackMasks[i] = appState.getArrangementSectionTrackMask(i);
    }

    item.chainEnabled = appState.arrangementChainEnabled.load(std::memory_order_relaxed);
    item.chainLength = appState.getArrangementChainLength();
    for (uint8_t i = 0; i < AppState::kArrangementMaxSections; ++i) {
        item.chainSequence[i] = appState.getArrangementChainStep(i);
    }

    return item;
}

void ArrangementPresetManager::applyToState(AppState& appState, const ContributionPackage::ArrangementPresetItem& preset) {
    appState.setArrangementSectionCount(preset.sectionCount);
    appState.setArrangementCurrentSection(preset.currentSection);
    appState.setArrangementMode(preset.mode);

    for (uint8_t i = 0; i < AppState::kArrangementMaxSections; ++i) {
        appState.setArrangementSectionLength(i, preset.sectionLengths[i]);
        appState.setArrangementSectionProgression(i, preset.sectionProgressions[i]);
        appState.setArrangementSectionTrackMask(i, preset.trackMasks[i]);
    }

    appState.setArrangementChainEnabled(preset.chainEnabled);
    appState.setArrangementChainLength(preset.chainLength);
    for (uint8_t i = 0; i < AppState::kArrangementMaxSections; ++i) {
        appState.setArrangementChainStep(i, preset.chainSequence[i]);
    }
}

std::string ArrangementPresetManager::presetToJson(const ContributionPackage::ArrangementPresetItem& item) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"cendanceArrangementPreset.v1\",\n";
    out << "  \"itemId\": " << juce::JSON::toString(juce::String(item.itemId)).toStdString() << ",\n";
    out << "  \"name\": " << juce::JSON::toString(juce::String(item.name)).toStdString() << ",\n";
    out << "  \"description\": " << juce::JSON::toString(juce::String(item.description)).toStdString() << ",\n";
    out << "  \"sectionCount\": " << static_cast<int>(item.sectionCount) << ",\n";
    out << "  \"currentSection\": " << static_cast<int>(item.currentSection) << ",\n";
    out << "  \"mode\": " << static_cast<int>(item.mode) << ",\n";
    out << "  \"sectionLengths\": [";
    for (int i = 0; i < AppState::kArrangementMaxSections; ++i) {
        if (i > 0) out << ", ";
        out << static_cast<int>(item.sectionLengths[i]);
    }
    out << "],\n";
    out << "  \"sectionProgressions\": [";
    for (int i = 0; i < AppState::kArrangementMaxSections; ++i) {
        if (i > 0) out << ", ";
        out << static_cast<int>(item.sectionProgressions[i]);
    }
    out << "],\n";
    out << "  \"trackMasks\": [";
    for (int i = 0; i < AppState::kArrangementMaxSections; ++i) {
        if (i > 0) out << ", ";
        out << static_cast<int>(item.trackMasks[i]);
    }
    out << "],\n";
    out << "  \"chainEnabled\": " << (item.chainEnabled ? "true" : "false") << ",\n";
    out << "  \"chainLength\": " << static_cast<int>(item.chainLength) << ",\n";
    out << "  \"chainSequence\": [";
    for (int i = 0; i < AppState::kArrangementMaxSections; ++i) {
        if (i > 0) out << ", ";
        out << static_cast<int>(item.chainSequence[i]);
    }
    out << "]\n";
    out << "}";
    return out.str();
}

bool ArrangementPresetManager::presetFromJson(const std::string& json, ContributionPackage::ArrangementPresetItem& item, std::string& error) {
    auto var = juce::JSON::parse(json);
    if (var.isVoid()) {
        error = "Failed to parse arrangement preset JSON";
        return false;
    }

    auto* obj = var.getDynamicObject();
    if (obj == nullptr) {
        error = "Arrangement preset JSON is not an object";
        return false;
    }

    item.itemId = obj->getProperty("itemId").toString().toStdString();
    item.name = obj->getProperty("name").toString().toStdString();
    item.description = obj->getProperty("description").toString().toStdString();

    if (item.itemId.empty()) {
        error = "Arrangement preset itemId is required";
        return false;
    }
    if (item.name.empty()) {
        item.name = item.itemId;
    }

    item.sectionCount = static_cast<uint8_t>(static_cast<int>(obj->getProperty("sectionCount")) & 0xFF);
    item.currentSection = static_cast<uint8_t>(static_cast<int>(obj->getProperty("currentSection")) & 0xFF);
    item.mode = static_cast<uint8_t>(static_cast<int>(obj->getProperty("mode")) & 0xFF);

    auto sectionLengths = obj->getProperty("sectionLengths");
    if (sectionLengths.isArray()) {
        auto* arr = sectionLengths.getArray();
        for (int i = 0; i < std::min<int>(AppState::kArrangementMaxSections, arr->size()); ++i) {
            item.sectionLengths[static_cast<size_t>(i)] = static_cast<uint8_t>(static_cast<int>((*arr)[i]) & 0xFF);
        }
    }

    auto sectionProgressions = obj->getProperty("sectionProgressions");
    if (sectionProgressions.isArray()) {
        auto* arr = sectionProgressions.getArray();
        for (int i = 0; i < std::min<int>(AppState::kArrangementMaxSections, arr->size()); ++i) {
            item.sectionProgressions[static_cast<size_t>(i)] = static_cast<uint8_t>(static_cast<int>((*arr)[i]) & 0xFF);
        }
    }

    auto trackMasks = obj->getProperty("trackMasks");
    if (trackMasks.isArray()) {
        auto* arr = trackMasks.getArray();
        for (int i = 0; i < std::min<int>(AppState::kArrangementMaxSections, arr->size()); ++i) {
            item.trackMasks[static_cast<size_t>(i)] = static_cast<uint8_t>(static_cast<int>((*arr)[i]) & 0xFF);
        }
    }

    item.chainEnabled = obj->getProperty("chainEnabled");
    item.chainLength = static_cast<uint8_t>(static_cast<int>(obj->getProperty("chainLength")) & 0xFF);

    auto chainSequence = obj->getProperty("chainSequence");
    if (chainSequence.isArray()) {
        auto* arr = chainSequence.getArray();
        for (int i = 0; i < std::min<int>(AppState::kArrangementMaxSections, arr->size()); ++i) {
            item.chainSequence[static_cast<size_t>(i)] = static_cast<uint8_t>(static_cast<int>((*arr)[i]) & 0xFF);
        }
    }

    return true;
}

std::string ArrangementPresetManager::presetsJson() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream out;
    out << "{\"arrangementPresets\":[";
    bool first = true;
    for (const auto& [id, entry] : presets_) {
        if (!first) out << ",";
        first = false;
        out << "{\"id\":" << juce::JSON::toString(juce::String(entry.item.itemId)).toStdString()
            << ",\"name\":" << juce::JSON::toString(juce::String(entry.item.name)).toStdString()
            << ",\"sectionCount\":" << static_cast<int>(entry.item.sectionCount)
            << ",\"mode\":" << static_cast<int>(entry.item.mode)
            << "}";
    }
    out << "]}";
    return out.str();
}

bool ArrangementPresetManager::loadPresetFromFile(const juce::File& file, ContributionPackage::ArrangementPresetItem& item, std::string& error) const {
    std::string content = readFileContents(file, error);
    if (content.empty() && !error.empty()) {
        return false;
    }
    return presetFromJson(content, item, error);
}

bool ArrangementPresetManager::writePresetToFile(const ContributionPackage::ArrangementPresetItem& item, const juce::File& file, std::string& error) const {
    std::string json = presetToJson(item);
    return writeFileContents(file, json, error);
}

std::string ArrangementPresetManager::generatePresetId(const std::string& name) const {
    // Generate a URL-safe ID from the name + timestamp
    std::string id;
    id.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            id += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (c == ' ' || c == '-' || c == '_') {
            id += '-';
        }
    }
    // Add timestamp suffix for uniqueness
    auto now = juce::Time::getCurrentTime();
    id += "-" + juce::String(now.toMilliseconds()).toStdString();
    return id;
}

// Global singleton
static ArrangementPresetManager* s_instance = nullptr;

ArrangementPresetManager& globalArrangementPresetManager() {
    if (s_instance == nullptr) {
        s_instance = new ArrangementPresetManager();
    }
    return *s_instance;
}
