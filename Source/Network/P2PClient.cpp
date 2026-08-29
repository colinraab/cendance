#include "P2PClient.h"

#include "../Security/PresetSerializer.h"

#include <juce_core/juce_core.h>

#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <random>

//==============================================================================
// Construction / configuration
//==============================================================================

P2PClient::P2PClient() {
    const char* envEndpoint = std::getenv("CENDANCE_P2P_ENDPOINT");
    endpoint_ = (envEndpoint != nullptr && envEndpoint[0] != '\0')
        ? std::string(envEndpoint)
        : "file://" + PresetSerializer::downloadDirectory()
              .getChildFile(juce::String("store"))
              .getFullPathName().toStdString();
    registry_.load();
}

P2PClient::~P2PClient() {
    registry_.save();
}

void P2PClient::setEndpoint(const std::string& url) {
    endpoint_ = url;
}

std::string P2PClient::endpoint() const {
    return endpoint_;
}

bool P2PClient::isConfigured() const {
    return !endpoint_.empty();
}

P2PDownloadRegistry& P2PClient::registry() {
    return registry_;
}

//==============================================================================
// Helpers
//==============================================================================

std::string P2PClient::baseUrl() const {
    std::string url = endpoint_;
    while (!url.empty() && url.back() == '/')
        url.pop_back();
    return url;
}

std::string P2PClient::contentTypeEndpoint(ContentType type) {
    switch (type) {
        case ContentType::Preset:    return "/api/v1/presets";
        case ContentType::Sample:    return "/api/v1/samples";
        case ContentType::Algorithm: return "/api/v1/algorithms";
        case ContentType::Project:   return "/api/v1/projects";
    }
    return "/api/v1/presets";
}

static bool isFileEndpoint(const std::string& url) {
    return url.find("file://") == 0;
}

//==============================================================================
// HTTP transport
//==============================================================================

static std::string readInputStream(juce::InputStream& stream) {
    juce::MemoryOutputStream output;
    output.writeFromInputStream(stream, -1);
    return output.toUTF8().toStdString();
}

std::string P2PClient::httpPost(const std::string& path, const std::string& jsonBody) {
    juce::URL url(baseUrl() + path);

    auto stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\nAccept: application/json\r\n")
            .withConnectionTimeoutMs(httpTimeoutMs_)
            .withHttpRequestCmd("POST"));

    if (stream == nullptr)
        return {};

    return readInputStream(*stream);
}

std::string P2PClient::httpGet(const std::string& path) {
    juce::URL url(baseUrl() + path);

    auto stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withExtraHeaders("Accept: application/json\r\n")
            .withConnectionTimeoutMs(httpTimeoutMs_));

    if (stream == nullptr)
        return {};

    return readInputStream(*stream);
}

std::vector<NetworkPresetEntry> P2PClient::httpGetList(const std::string& path) {
    std::vector<NetworkPresetEntry> results;
    auto response = httpGet(path);
    if (response.empty()) return results;

    auto parsed = juce::JSON::parse(juce::String(response));
    if (parsed.isVoid() || !parsed.isObject()) return results;

    auto* obj = parsed.getDynamicObject();
    if (!obj) return results;

    juce::var entries;
    for (const char* key : {"presets", "samples", "algorithms", "items"}) {
        entries = obj->getProperty(key);
        if (entries.isArray()) break;
    }
    if (!entries.isArray()) return results;

    auto* arr = entries.getArray();
    for (const auto& item : *arr) {
        if (!item.isObject()) continue;
        auto* entry = item.getDynamicObject();
        if (!entry) continue;

        NetworkPresetEntry ne;
        ne.preset_id = entry->getProperty("id").toString().toStdString();
        ne.sender_id = entry->getProperty("sender_id").toString().toStdString();
        ne.timestamp = static_cast<uint64_t>(static_cast<int64_t>(entry->getProperty("timestamp")));
        ne.display_name = entry->getProperty("display_name").toString().toStdString();
        ne.fileSize = static_cast<uint64_t>(static_cast<int64_t>(entry->getProperty("file_size")));
        ne.format = entry->getProperty("format").toString().toStdString();
        ne.sampleRate = static_cast<uint32_t>(static_cast<int>(static_cast<int64_t>(entry->getProperty("sample_rate"))));
        ne.channels = static_cast<uint16_t>(static_cast<int>(static_cast<int64_t>(entry->getProperty("channels"))));
        ne.duration = static_cast<double>(entry->getProperty("duration"));

        if (ne.display_name.empty()) ne.display_name = ne.preset_id;
        results.push_back(ne);
    }

    return results;
}

//==============================================================================
// Preset operations
//==============================================================================

std::future<PublishResult> P2PClient::publishPreset(const std::string& signed_envelope) {
    return std::async(std::launch::async, [this, signed_envelope]() -> PublishResult {
        PublishResult result;
        if (signed_envelope.empty()) { result.error = "Empty envelope"; return result; }

        auto parsed = juce::JSON::parse(juce::String(signed_envelope));
        if (parsed.isVoid() || !parsed.isObject()) { result.error = "Invalid envelope JSON"; return result; }

        std::string presetId;
        if (isFileEndpoint(endpoint_)) {
            presetId = fileStorePublish(signed_envelope, ContentType::Preset);
        } else {
            auto response = httpPost("/api/v1/publish", signed_envelope);
            if (response.empty()) {
                presetId = fileStorePublish(signed_envelope, ContentType::Preset);
            } else {
                auto resp = juce::JSON::parse(juce::String(response));
                if (resp.isObject()) {
                    auto* obj = resp.getDynamicObject();
                    presetId = obj->getProperty("id").toString().toStdString();
                    if (presetId.empty()) presetId = obj->getProperty("preset_id").toString().toStdString();
                }
                if (presetId.empty()) presetId = fileStorePublish(signed_envelope, ContentType::Preset);
            }
        }

        if (presetId.empty()) { result.error = "Failed to publish preset"; return result; }
        result.ok = true;
        result.preset_id = presetId;
        return result;
    });
}

std::future<std::string> P2PClient::requestPreset(const std::string& preset_id) {
    return std::async(std::launch::async, [this, preset_id]() -> std::string {
        if (preset_id.empty()) return {};
        if (isFileEndpoint(endpoint_)) return fileStoreGet(preset_id, ContentType::Preset);
        auto response = httpGet("/api/v1/presets/" + preset_id);
        if (response.empty()) return fileStoreGet(preset_id, ContentType::Preset);
        return response;
    });
}

std::future<std::vector<NetworkPresetEntry>> P2PClient::searchPresets() {
    return std::async(std::launch::async, [this]() -> std::vector<NetworkPresetEntry> {
        if (isFileEndpoint(endpoint_)) return fileStoreList(ContentType::Preset);
        auto results = httpGetList("/api/v1/presets");
        if (results.empty()) return fileStoreList(ContentType::Preset);
        return results;
    });
}

//==============================================================================
// Sample operations
//==============================================================================

std::future<PublishResult> P2PClient::publishSample(const std::string& signed_envelope) {
    return std::async(std::launch::async, [this, signed_envelope]() -> PublishResult {
        PublishResult result;
        if (signed_envelope.empty()) { result.error = "Empty envelope"; return result; }
        auto parsed = juce::JSON::parse(juce::String(signed_envelope));
        if (parsed.isVoid() || !parsed.isObject()) { result.error = "Invalid envelope JSON"; return result; }

        if (isFileEndpoint(endpoint_)) {
            result.sample_id = fileStorePublish(signed_envelope, ContentType::Sample);
        } else {
            auto response = httpPost("/api/v1/publish", signed_envelope);
            if (response.empty()) {
                result.sample_id = fileStorePublish(signed_envelope, ContentType::Sample);
            } else {
                auto resp = juce::JSON::parse(juce::String(response));
                if (resp.isObject()) {
                    auto* obj = resp.getDynamicObject();
                    result.sample_id = obj->getProperty("id").toString().toStdString();
                    if (result.sample_id.empty()) result.sample_id = obj->getProperty("sample_id").toString().toStdString();
                }
                if (result.sample_id.empty()) result.sample_id = fileStorePublish(signed_envelope, ContentType::Sample);
            }
        }
        result.ok = !result.sample_id.empty();
        if (!result.ok) result.error = "Failed to publish sample";
        return result;
    });
}

std::future<std::string> P2PClient::requestSample(const std::string& sample_id) {
    return std::async(std::launch::async, [this, sample_id]() -> std::string {
        if (sample_id.empty()) return {};
        if (isFileEndpoint(endpoint_)) return fileStoreGet(sample_id, ContentType::Sample);
        auto response = httpGet("/api/v1/samples/" + sample_id);
        if (response.empty()) return fileStoreGet(sample_id, ContentType::Sample);
        return response;
    });
}

std::future<std::vector<NetworkPresetEntry>> P2PClient::searchSamples() {
    return std::async(std::launch::async, [this]() -> std::vector<NetworkPresetEntry> {
        if (isFileEndpoint(endpoint_)) return fileStoreList(ContentType::Sample);
        auto results = httpGetList("/api/v1/samples");
        if (results.empty()) return fileStoreList(ContentType::Sample);
        return results;
    });
}

//==============================================================================
// Algorithm operations
//==============================================================================

std::future<PublishResult> P2PClient::publishAlgorithm(const std::string& signed_envelope) {
    return std::async(std::launch::async, [this, signed_envelope]() -> PublishResult {
        PublishResult result;
        if (signed_envelope.empty()) { result.error = "Empty envelope"; return result; }
        auto parsed = juce::JSON::parse(juce::String(signed_envelope));
        if (parsed.isVoid() || !parsed.isObject()) { result.error = "Invalid envelope JSON"; return result; }

        if (isFileEndpoint(endpoint_)) {
            result.preset_id = fileStorePublish(signed_envelope, ContentType::Algorithm);
        } else {
            auto response = httpPost("/api/v1/publish", signed_envelope);
            if (response.empty()) {
                result.preset_id = fileStorePublish(signed_envelope, ContentType::Algorithm);
            } else {
                auto resp = juce::JSON::parse(juce::String(response));
                if (resp.isObject()) {
                    auto* obj = resp.getDynamicObject();
                    result.preset_id = obj->getProperty("id").toString().toStdString();
                }
                if (result.preset_id.empty()) result.preset_id = fileStorePublish(signed_envelope, ContentType::Algorithm);
            }
        }
        result.ok = !result.preset_id.empty();
        if (!result.ok) result.error = "Failed to publish algorithm";
        return result;
    });
}

std::future<std::string> P2PClient::requestAlgorithm(const std::string& algorithm_id) {
    return std::async(std::launch::async, [this, algorithm_id]() -> std::string {
        if (algorithm_id.empty()) return {};
        if (isFileEndpoint(endpoint_)) return fileStoreGet(algorithm_id, ContentType::Algorithm);
        auto response = httpGet("/api/v1/algorithms/" + algorithm_id);
        if (response.empty()) return fileStoreGet(algorithm_id, ContentType::Algorithm);
        return response;
    });
}

std::future<std::vector<NetworkPresetEntry>> P2PClient::searchAlgorithms(
    const std::string& query,
    std::optional<uint8_t> trackIndex,
    std::optional<uint8_t> genreId) {
    return std::async(std::launch::async, [this, query, trackIndex, genreId]() -> std::vector<NetworkPresetEntry> {
        if (isFileEndpoint(endpoint_)) return fileStoreList(ContentType::Algorithm);

        std::string path = "/api/v1/algorithms";
        bool first = true;
        auto append = [&](const std::string& key, const std::string& val) {
            path += (first ? "?" : "&") + key + "=" + val;
            first = false;
        };
        if (!query.empty()) append("q", query);
        if (trackIndex.has_value()) append("track", std::to_string(trackIndex.value()));
        if (genreId.has_value()) append("genre", std::to_string(genreId.value()));

        auto results = httpGetList(path);
        if (results.empty()) return fileStoreList(ContentType::Algorithm);
        return results;
    });
}

//==============================================================================
// File-store fallback
//==============================================================================

std::string P2PClient::fileStorePublish(const std::string& envelope, ContentType contentType) {
    auto storeDir = PresetSerializer::downloadDirectory()
        .getChildFile(juce::String("store"));
    if (!storeDir.exists())
        storeDir.createDirectory();

    std::string presetId = contentType == ContentType::Sample ? generateSampleId() : generatePresetId();
    if (contentType == ContentType::Algorithm)
        presetId = generateAlgorithmId();
    const std::string extension = contentType == ContentType::Sample ? ".sample" :
                                  contentType == ContentType::Algorithm ? ".algorithm" : ".preset";
    auto file = storeDir.getChildFile(juce::String(presetId + extension));

    std::ofstream out(file.getFullPathName().toStdString(),
                      std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return {};
    out << envelope;
    out.close();

    return presetId;
}

std::string P2PClient::fileStoreGet(const std::string& preset_id, ContentType contentType) {
    const std::string extension = contentType == ContentType::Sample ? ".sample" :
                                  contentType == ContentType::Algorithm ? ".algorithm" : ".preset";
    auto file = PresetSerializer::downloadDirectory()
        .getChildFile(juce::String("store"))
        .getChildFile(juce::String(juce::String(preset_id) + extension));

    if (!file.existsAsFile()) return {};

    juce::String content = file.loadFileAsString();
    return content.toStdString();
}

std::vector<NetworkPresetEntry> P2PClient::fileStoreList(ContentType contentType) {
    std::vector<NetworkPresetEntry> results;
    auto storeDir = PresetSerializer::downloadDirectory()
        .getChildFile(juce::String("store"));

    if (!storeDir.exists()) return results;

    const std::string pattern = contentType == ContentType::Sample ? "*.sample" :
                                contentType == ContentType::Algorithm ? "*.algorithm" : "*.preset";
    auto files = storeDir.findChildFiles(juce::File::findFiles, false, pattern);
    for (auto& file : files) {
        NetworkPresetEntry entry;
        entry.preset_id = file.getFileNameWithoutExtension().toStdString();
        entry.contentType = contentType;
        entry.fileSize = static_cast<uint64_t>(file.getSize());

        juce::String content = file.loadFileAsString();
        if (!content.isEmpty()) {
            auto parsed = juce::JSON::parse(content);
            if (parsed.isObject()) {
                auto* obj = parsed.getDynamicObject();
                juce::String headerJson = obj->getProperty("header");
                if (!headerJson.isEmpty()) {
                    ContentHeader header;
                    std::string err;
                    if (ContentHeader::fromJson(headerJson.toStdString(), header, err)) {
                        entry.sender_id = header.sender_id;
                        entry.timestamp = header.timestamp;
                    }
                }
                if (contentType == ContentType::Sample) {
                    juce::String payloadJson = obj->getProperty("payload");
                    auto payload = juce::JSON::parse(payloadJson);
                    if (payload.isObject()) {
                        auto* payloadObj = payload.getDynamicObject();
                        entry.display_name = payloadObj->getProperty("name").toString().toStdString();
                        entry.format = payloadObj->getProperty("format").toString().toStdString();
                        entry.sampleRate = static_cast<uint32_t>(static_cast<int>(static_cast<int64_t>(payloadObj->getProperty("sample_rate"))));
                        entry.channels = static_cast<uint16_t>(static_cast<int>(static_cast<int64_t>(payloadObj->getProperty("channels"))));
                        entry.duration = static_cast<double>(payloadObj->getProperty("duration"));
                    }
                }
            }
        }

        if (entry.display_name.empty()) entry.display_name = entry.preset_id;
        results.push_back(entry);
    }
    return results;
}

//==============================================================================
// ID generators
//==============================================================================

std::string P2PClient::generateSampleId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::mt19937 rng(static_cast<unsigned>(ms ^ 0x51a7u));
    std::uniform_int_distribution<int> dist(0, 15);
    std::ostringstream oss;
    oss << "sample_" << ms << "_";
    for (int i = 0; i < 8; ++i) oss << std::hex << dist(rng);
    return oss.str();
}

std::string P2PClient::generatePresetId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::mt19937 rng(static_cast<unsigned>(ms));
    std::uniform_int_distribution<int> dist(0, 15);
    std::ostringstream oss;
    oss << "preset_" << ms << "_";
    for (int i = 0; i < 8; ++i) oss << std::hex << dist(rng);
    return oss.str();
}

std::string P2PClient::generateAlgorithmId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::mt19937 rng(static_cast<unsigned>(ms ^ 0xa1f7u));
    std::uniform_int_distribution<int> dist(0, 15);
    std::ostringstream oss;
    oss << "algorithm_" << ms << "_";
    for (int i = 0; i < 8; ++i) oss << std::hex << dist(rng);
    return oss.str();
}
