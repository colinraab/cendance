#include "P2PDownloadRegistry.h"
#include "../Config/AppDirectories.h"

#include <juce_core/juce_core.h>

#include <fstream>

void P2PDownloadRegistry::addEntry(const P2PDownloadEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_[entry.preset_id] = entry;
}

std::vector<P2PDownloadEntry> P2PDownloadRegistry::allEntries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<P2PDownloadEntry> result;
    result.reserve(entries_.size());
    for (auto& [id, entry] : entries_)
        result.push_back(entry);
    return result;
}

const P2PDownloadEntry* P2PDownloadRegistry::findEntry(const std::string& preset_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = entries_.find(preset_id);
    if (it != entries_.end())
        return &it->second;
    return nullptr;
}

juce::File P2PDownloadRegistry::registryFilePath() const {
    return AppDirectories::dataDirectory().getChildFile("p2p_downloads.json");
}

bool P2PDownloadRegistry::save() const {
    auto file = registryFilePath();
    auto dir = file.getParentDirectory();
    if (!dir.exists())
        dir.createDirectory();

    auto root = std::make_unique<juce::DynamicObject>();
    juce::Array<juce::var> downloadsArray;

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [id, entry] : entries_) {
        auto entryObj = std::make_unique<juce::DynamicObject>();
        entryObj->setProperty("preset_id", juce::String(entry.preset_id));
        entryObj->setProperty("sender_id", juce::String(entry.sender_id));
        entryObj->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
        entryObj->setProperty("verified", entry.verified);
        entryObj->setProperty("local_path", juce::String(entry.local_path));
        entryObj->setProperty("content_type", static_cast<int>(entry.content_type));
        entryObj->setProperty("display_name", juce::String(entry.display_name));
        entryObj->setProperty("format", juce::String(entry.format));
        entryObj->setProperty("sample_rate", static_cast<int>(entry.sample_rate));
        entryObj->setProperty("channels", static_cast<int>(entry.channels));
        entryObj->setProperty("duration", entry.duration);
        entryObj->setProperty("sha256", juce::String(entry.sha256));
        entryObj->setProperty("track_index", static_cast<int>(entry.track_index));
        entryObj->setProperty("genre_tags", static_cast<int64_t>(entry.genre_tags));
        entryObj->setProperty("preset_ref", juce::String(entry.preset_ref));
        entryObj->setProperty("version", juce::String(entry.version));
        downloadsArray.add(juce::var(entryObj.release()));
    }
    root->setProperty("downloads", juce::var(downloadsArray));

    juce::String json = juce::JSON::toString(juce::var(root.release()), true);
    std::ofstream out(file.getFullPathName().toStdString(),
                      std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out << json.toStdString();
    out.close();
    return true;
}

bool P2PDownloadRegistry::load() {
    auto file = registryFilePath();
    if (!file.existsAsFile()) return true;  // no existing registry is fine

    juce::String content = file.loadFileAsString();
    if (content.isEmpty()) return true;

    auto parsed = juce::JSON::parse(content);
    if (parsed.isVoid() || !parsed.isObject()) return false;

    auto* obj = parsed.getDynamicObject();
    if (!obj) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();

    juce::var downloadsVar = obj->getProperty("downloads");
    if (downloadsVar.isArray()) {
        auto* arr = downloadsVar.getArray();
        for (auto& item : *arr) {
            if (item.isObject()) {
                auto* entryObj = item.getDynamicObject();
                P2PDownloadEntry entry;
                entry.preset_id = entryObj->getProperty("preset_id").toString().toStdString();
                entry.sender_id = entryObj->getProperty("sender_id").toString().toStdString();
                entry.timestamp = static_cast<uint64_t>(
                    static_cast<int64_t>(entryObj->getProperty("timestamp")));
                entry.verified = static_cast<bool>(entryObj->getProperty("verified"));
                entry.local_path = entryObj->getProperty("local_path").toString().toStdString();
                entry.content_type = static_cast<ContentType>(static_cast<int>(entryObj->getProperty("content_type")));
                entry.display_name = entryObj->getProperty("display_name").toString().toStdString();
                entry.format = entryObj->getProperty("format").toString().toStdString();
                entry.sample_rate = static_cast<uint32_t>(static_cast<int64_t>(entryObj->getProperty("sample_rate")));
                entry.channels = static_cast<uint16_t>(static_cast<int64_t>(entryObj->getProperty("channels")));
                entry.duration = static_cast<double>(entryObj->getProperty("duration"));
                entry.sha256 = entryObj->getProperty("sha256").toString().toStdString();
                entry.track_index = static_cast<uint8_t>(static_cast<int>(entryObj->getProperty("track_index")));
                entry.genre_tags = static_cast<uint32_t>(static_cast<int64_t>(entryObj->getProperty("genre_tags")));
                entry.preset_ref = entryObj->getProperty("preset_ref").toString().toStdString();
                entry.version = entryObj->getProperty("version").toString().toStdString();
                if (!entry.preset_id.empty())
                    entries_[entry.preset_id] = entry;
            }
        }
    }
    return true;
}
