#pragma once

#include <juce_core/juce_core.h>
#include "../Security/ContentHeader.h"

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <map>

// Entry in the download registry.
struct P2PDownloadEntry {
    std::string preset_id;
    std::string sender_id;
    uint64_t timestamp = 0;
    bool verified = false;
    std::string local_path;
    ContentType content_type = ContentType::Preset;
    std::string display_name;
    std::string format;
    uint32_t sample_rate = 0;
    uint16_t channels = 0;
    double duration = 0.0;
    std::string sha256;

    // Algorithm-specific metadata
    uint8_t track_index = 0;
    uint32_t genre_tags = 0;
    std::string preset_ref;
    std::string version;
};

// P2PDownloadRegistry tracks downloaded presets.
// Thread-safe for background P2P thread access.
class P2PDownloadRegistry {
public:
    P2PDownloadRegistry() = default;
    ~P2PDownloadRegistry() = default;

    // Add or update a download entry.
    void addEntry(const P2PDownloadEntry& entry);

    // Get all entries.
    std::vector<P2PDownloadEntry> allEntries() const;

    // Get entry by preset_id, returns nullptr if not found.
    const P2PDownloadEntry* findEntry(const std::string& preset_id) const;

    // Persist registry to disk.
    bool save() const;

    // Load registry from disk.
    bool load();

    // Path to the registry file.
    juce::File registryFilePath() const;

private:
    mutable std::mutex mutex_;
    std::map<std::string, P2PDownloadEntry> entries_;  // keyed by preset_id
};
