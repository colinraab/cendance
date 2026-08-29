#pragma once

#include "P2PDownloadRegistry.h"
#include "../Security/ContentHeader.h"

#include <cstdint>
#include <future>
#include <optional>
#include <string>
#include <vector>

struct PublishResult {
    bool ok = false;
    std::string preset_id;
    std::string error;
    std::string sample_id;
};

struct NetworkPresetEntry {
    std::string preset_id;
    std::string sender_id;
    uint64_t timestamp = 0;
    ContentType contentType = ContentType::Preset;
    uint64_t fileSize = 0;
    std::string display_name;
    std::string format;
    uint32_t sampleRate = 0;
    uint16_t channels = 0;
    double duration = 0.0;
};

class P2PDownloadRegistry;

class P2PClient {
public:
    P2PClient();
    ~P2PClient();

    void setEndpoint(const std::string& url);
    std::string endpoint() const;
    bool isConfigured() const;

    // HTTP timeout in milliseconds (default: 10 seconds)
    void setHttpTimeout(int timeoutMs) { httpTimeoutMs_ = timeoutMs; }
    int httpTimeout() const { return httpTimeoutMs_; }

    P2PDownloadRegistry& registry();

    // Preset operations
    std::future<PublishResult> publishPreset(const std::string& signed_envelope);
    std::future<std::string> requestPreset(const std::string& preset_id);
    std::future<std::vector<NetworkPresetEntry>> searchPresets();

    // Sample operations
    std::future<PublishResult> publishSample(const std::string& signed_envelope);
    std::future<std::string> requestSample(const std::string& sample_id);
    std::future<std::vector<NetworkPresetEntry>> searchSamples();

    // Algorithm operations
    std::future<PublishResult> publishAlgorithm(const std::string& signed_envelope);
    std::future<std::string> requestAlgorithm(const std::string& algorithm_id);
    std::future<std::vector<NetworkPresetEntry>> searchAlgorithms(
        const std::string& query = "",
        std::optional<uint8_t> trackIndex = std::nullopt,
        std::optional<uint8_t> genreId = std::nullopt);

private:
    // HTTP transport methods
    std::string httpPost(const std::string& path, const std::string& jsonBody);
    std::string httpGet(const std::string& path);
    std::vector<NetworkPresetEntry> httpGetList(const std::string& path);

    // File-store fallback (for file:// endpoints or when HTTP fails)
    std::string fileStorePublish(const std::string& envelope, ContentType contentType);
    std::string fileStoreGet(const std::string& preset_id, ContentType contentType);
    std::vector<NetworkPresetEntry> fileStoreList(ContentType contentType);

    // ID generators
    std::string generateSampleId();
    std::string generatePresetId();
    std::string generateAlgorithmId();

    // Helpers
    std::string baseUrl() const;
    static std::string contentTypeEndpoint(ContentType type);

    std::string endpoint_;
    int httpTimeoutMs_ = 10000;
    P2PDownloadRegistry registry_;
};
