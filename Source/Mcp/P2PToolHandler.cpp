#include "P2PToolHandler.h"

#include "../App/AlgorithmPresetRegistry.h"
#include "../App/ArrangementPresetManager.h"
#include "../App/ContributionPackage.h"
#include <juce_core/juce_core.h>

#include <string>

using namespace juce;

namespace {

std::string getStringArg(const var& args, const char* key, const char* fallback = "") {
    var v = args.getProperty(key, var());
    return v.toString().toStdString();
}

String makeError(int code, const String& message) {
    auto* errObj = new DynamicObject();
    errObj->setProperty("code", code);
    errObj->setProperty("message", message);
    return JSON::toString(var(errObj), false);
}

} // namespace

P2PToolHandler::P2PToolHandler(AppState& appState,
                               SecurityManager& securityManager,
                               PresetSerializer& presetSerializer,
                               P2PClient& p2pClient)
    : appState(appState),
      securityManager(securityManager),
      presetSerializer(presetSerializer),
      p2pClient(p2pClient) {
}

String P2PToolHandler::handle(const String& toolName, const String& argsJson) {
    auto args = JSON::parse(argsJson);
    auto getStr = [&](const char* key, const char* fallback = "") -> std::string {
        juce::var v = args.getProperty(key, juce::var());
        return v.toString().toStdString();
    };
    if (toolName == "save_and_sign_preset") {
        std::string error;
        std::string envelope = presetSerializer.createEnvelope(appState, securityManager, error);
        if (envelope.empty()) {
            return makeError(5001, juce::String(error));
        }
        auto* res = new juce::DynamicObject();
        res->setProperty("envelope", juce::String(envelope));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "share_on_network") {
        std::string presetJson = getStr("preset_json");
        if (presetJson.empty()) {
            return makeError(4002, "preset_json is required");
        }
        auto result = p2pClient.publishPreset(presetJson).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", result.ok);
        res->setProperty("preset_id", juce::String(result.preset_id));
        if (!result.error.empty())
            res->setProperty("error", juce::String(result.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "search_network") {
        auto results = p2pClient.searchPresets().get();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> entries;
        for (auto& entry : results) {
            auto* e = new juce::DynamicObject();
            e->setProperty("preset_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("display_name", juce::String(entry.display_name));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            entries.add(juce::var(e));
        }
        res->setProperty("presets", juce::var(entries));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "verify_incoming_preset") {
        std::string presetJson = getStr("preset_json");
        if (presetJson.empty()) {
            return makeError(4002, "preset_json is required");
        }
        auto result = presetSerializer.verifyAndLoad(presetJson, securityManager).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", result.ok);
        res->setProperty("trust_level", static_cast<int>(result.trustLevel));
        if (!result.payload_json.empty())
            res->setProperty("payload_json", juce::String(result.payload_json));
        if (!result.error.empty())
            res->setProperty("error", juce::String(result.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "list_downloaded_presets") {
        auto entries = p2pClient.registry().allEntries();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> arr;
        for (auto& entry : entries) {
            auto* e = new juce::DynamicObject();
            e->setProperty("preset_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            e->setProperty("verified", entry.verified);
            e->setProperty("local_path", juce::String(entry.local_path));
            arr.add(juce::var(e));
        }
        res->setProperty("downloads", juce::var(arr));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "save_and_sign_sample") {
        SampleEnvelopeMetadata metadata;
        metadata.name = getStr("name");
        metadata.description = getStr("description");
        if (auto* tags = args.getProperty("tags", juce::var()).getArray()) {
            for (auto& tag : *tags) if (tag.isString()) metadata.tags.push_back(tag.toString().toStdString());
        }
        std::string error;
        const std::string envelope = presetSerializer.createSampleEnvelope(getStr("path"), metadata, securityManager, error);
        if (envelope.empty()) {
            return makeError(5001, juce::String(error));
        }
        auto envelopeVar = juce::JSON::parse(envelope);
        auto payload = juce::JSON::parse(envelopeVar.getProperty("payload", juce::var()).toString());
        auto* payloadObj = payload.getDynamicObject();
        auto* res = new juce::DynamicObject();
        res->setProperty("envelope", juce::String(envelope));
        if (payloadObj != nullptr) {
            res->setProperty("sha256", payloadObj->getProperty("sha256"));
            res->setProperty("format", payloadObj->getProperty("format"));
            res->setProperty("sample_rate", payloadObj->getProperty("sample_rate"));
            res->setProperty("channels", payloadObj->getProperty("channels"));
            res->setProperty("duration", payloadObj->getProperty("duration"));
        }
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "share_sample_on_network") {
        const auto result = p2pClient.publishSample(getStr("sample_json")).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", result.ok);
        res->setProperty("sample_id", juce::String(result.sample_id));
        if (!result.error.empty()) res->setProperty("error", juce::String(result.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "search_samples") {
        const std::string query = getStr("query");
        const std::string format = getStr("format");
        auto results = p2pClient.searchSamples().get();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> arr;
        for (auto& entry : results) {
            if (!format.empty() && entry.format != format) continue;
            if (!query.empty() && entry.display_name.find(query) == std::string::npos) continue;
            auto* e = new juce::DynamicObject();
            e->setProperty("sample_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("display_name", juce::String(entry.display_name));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            e->setProperty("fileSize", static_cast<int64_t>(entry.fileSize));
            e->setProperty("format", juce::String(entry.format));
            e->setProperty("sampleRate", static_cast<int>(entry.sampleRate));
            e->setProperty("channels", static_cast<int>(entry.channels));
            e->setProperty("duration", entry.duration);
            arr.add(juce::var(e));
        }
        res->setProperty("samples", juce::var(arr));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "download_sample") {
        const std::string sampleId = getStr("sample_id");
        const std::string envelope = p2pClient.requestSample(sampleId).get();
        if (envelope.empty()) {
            return makeError(404, "sample_id not found");
        }
        const auto verified = presetSerializer.verifyAndLoadSample(envelope, securityManager).get();
        if (verified.ok) {
            P2PDownloadEntry entry;
            entry.preset_id = sampleId;
            entry.sender_id = verified.sender_id;
            entry.timestamp = verified.timestamp;
            entry.verified = verified.trustLevel == TrustLevel::Verified;
            entry.local_path = verified.local_path;
            entry.content_type = ContentType::Sample;
            entry.display_name = verified.name;
            entry.format = verified.format;
            entry.sample_rate = verified.sampleRate;
            entry.channels = verified.channels;
            entry.duration = verified.duration;
            entry.sha256 = verified.sha256;
            p2pClient.registry().addEntry(entry);
            p2pClient.registry().save();
        }
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", verified.ok);
        res->setProperty("trust_level", static_cast<int>(verified.trustLevel));
        res->setProperty("local_path", juce::String(verified.local_path));
        res->setProperty("name", juce::String(verified.name));
        res->setProperty("format", juce::String(verified.format));
        res->setProperty("sha256", juce::String(verified.sha256));
        if (!verified.error.empty()) res->setProperty("error", juce::String(verified.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "list_downloaded_samples") {
        auto entries = p2pClient.registry().allEntries();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> arr;
        for (auto& entry : entries) {
            if (entry.content_type != ContentType::Sample) continue;
            auto* e = new juce::DynamicObject();
            e->setProperty("sample_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            e->setProperty("verified", entry.verified);
            e->setProperty("local_path", juce::String(entry.local_path));
            e->setProperty("display_name", juce::String(entry.display_name));
            e->setProperty("format", juce::String(entry.format));
            e->setProperty("sampleRate", static_cast<int>(entry.sample_rate));
            e->setProperty("channels", static_cast<int>(entry.channels));
            e->setProperty("duration", entry.duration);
            arr.add(juce::var(e));
        }
        res->setProperty("downloads", juce::var(arr));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "create_custom_sound_preset") {
        const int track = static_cast<int>(args.getProperty("track", 0));
        const bool includeSamples = static_cast<bool>(args.getProperty("includeSamples", true));
        std::string error;
        const std::string envelope = presetSerializer.createCustomSoundPresetEnvelope(
            appState, static_cast<uint8_t>(track > 0 ? track - 1 : 255), includeSamples, securityManager, error);
        if (envelope.empty()) {
            return makeError(5001, juce::String(error));
        }
        auto* res = new juce::DynamicObject();
        res->setProperty("envelope", juce::String(envelope));
        return juce::JSON::toString(juce::var(res), false);
    }

    else if (toolName == "share_algorithm_on_network") {
        const int algoId = static_cast<int>(args.getProperty("algorithm_id", 0));
        const int trackIdx = static_cast<int>(args.getProperty("track_index", 0));
        const auto* preset = globalAlgorithmPresetRegistry().findByRuntimeId(
            static_cast<uint8_t>(trackIdx), static_cast<uint16_t>(algoId));
        if (!preset) {
            return makeError(404, "Algorithm not found");
        }
        std::string error;
        const std::string envelope = presetSerializer.createAlgorithmEnvelope(*preset, securityManager, error);
        if (envelope.empty()) {
            return makeError(500, juce::String(error));
        }
        auto publishResult = p2pClient.publishAlgorithm(envelope).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", publishResult.ok);
        res->setProperty("algorithm_id", juce::String(publishResult.preset_id));
        if (!publishResult.error.empty())
            res->setProperty("error", juce::String(publishResult.error));
        return juce::JSON::toString(juce::var(res), false);
    }

    // ─── Arrangement preset sharing ───
    else if (toolName == "save_and_sign_arrangement") {
        std::string name = getStr("name");
        if (name.empty()) {
            return makeError(4002, "name is required");
        }
        std::string error;
        std::string envelope = presetSerializer.createArrangementEnvelope(appState, name, securityManager, error);
        if (envelope.empty()) {
            return makeError(5001, juce::String(error));
        }
        auto* res = new juce::DynamicObject();
        res->setProperty("envelope", juce::String(envelope));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "share_arrangement_on_network") {
        std::string envelopeJson = getStr("envelope_json");
        if (envelopeJson.empty()) {
            return makeError(4002, "envelope_json is required");
        }
        auto result = p2pClient.publishPreset(envelopeJson).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", result.ok);
        res->setProperty("preset_id", juce::String(result.preset_id));
        if (!result.error.empty())
            res->setProperty("error", juce::String(result.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "search_arrangements") {
        // Reuse the same search as presets — arrangement presets use the same content type
        auto results = p2pClient.searchPresets().get();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> entries;
        for (auto& entry : results) {
            auto* e = new juce::DynamicObject();
            e->setProperty("preset_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("display_name", juce::String(entry.display_name));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            entries.add(juce::var(e));
        }
        res->setProperty("arrangements", juce::var(entries));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "download_arrangement") {
        std::string envelopeJson = getStr("envelope_json");
        if (envelopeJson.empty()) {
            return makeError(4002, "envelope_json is required");
        }
        // Verify the arrangement envelope
        auto verifyResult = presetSerializer.verifyAndLoadArrangement(envelopeJson, securityManager).get();
        if (!verifyResult.ok) {
            auto* res = new juce::DynamicObject();
            res->setProperty("ok", false);
            res->setProperty("trust_level", static_cast<int>(verifyResult.trustLevel));
            res->setProperty("error", juce::String(verifyResult.error));
            return juce::JSON::toString(juce::var(res), false);
        }

        // Parse the verified payload into an ArrangementPresetItem and apply it
        ContributionPackage::ArrangementPresetItem item;
        std::string parseError;
        if (!ArrangementPresetManager::presetFromJson(verifyResult.payload_json, item, parseError)) {
            return makeError(5001, "Failed to parse arrangement preset: " + parseError);
        }

        // Apply directly to state
        ArrangementPresetManager::applyToState(appState, item);

        auto* res = new juce::DynamicObject();
        res->setProperty("ok", true);
        res->setProperty("trust_level", static_cast<int>(verifyResult.trustLevel));
        res->setProperty("name", juce::String(item.name));
        res->setProperty("section_count", static_cast<int>(item.sectionCount));
        return juce::JSON::toString(juce::var(res), false);
    }

    // ─── Project file sharing ───
    else if (toolName == "save_and_sign_project") {
        std::string name = getStr("name");
        std::string error;
        std::string envelope = presetSerializer.createProjectEnvelope(appState, name, securityManager, error);
        if (envelope.empty()) {
            return makeError(5001, juce::String(error));
        }
        auto* res = new juce::DynamicObject();
        res->setProperty("envelope", juce::String(envelope));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "share_project_on_network") {
        std::string envelopeJson = getStr("envelope_json");
        if (envelopeJson.empty()) {
            return makeError(4002, "envelope_json is required");
        }
        auto result = p2pClient.publishPreset(envelopeJson).get();
        auto* res = new juce::DynamicObject();
        res->setProperty("ok", result.ok);
        res->setProperty("project_id", juce::String(result.preset_id));
        if (!result.error.empty())
            res->setProperty("error", juce::String(result.error));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "search_projects") {
        auto results = p2pClient.searchPresets().get();
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> entries;
        for (auto& entry : results) {
            auto* e = new juce::DynamicObject();
            e->setProperty("project_id", juce::String(entry.preset_id));
            e->setProperty("sender_id", juce::String(entry.sender_id));
            e->setProperty("display_name", juce::String(entry.display_name));
            e->setProperty("timestamp", static_cast<int64_t>(entry.timestamp));
            entries.add(juce::var(e));
        }
        res->setProperty("projects", juce::var(entries));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "download_project") {
        std::string envelopeJson = getStr("envelope_json");
        if (envelopeJson.empty()) {
            return makeError(4002, "envelope_json is required");
        }
        // Verify the project envelope
        auto verifyResult = presetSerializer.verifyAndLoadProject(envelopeJson, securityManager).get();
        if (!verifyResult.ok) {
            auto* res = new juce::DynamicObject();
            res->setProperty("ok", false);
            res->setProperty("trust_level", static_cast<int>(verifyResult.trustLevel));
            res->setProperty("error", juce::String(verifyResult.error));
            return juce::JSON::toString(juce::var(res), false);
        }

        // Parse the project name from the payload JSON to use as filename
        auto payloadVar = juce::JSON::parse(juce::String(verifyResult.payload_json));
        auto* payloadObj = payloadVar.getDynamicObject();
        std::string projectName = "project";
        if (payloadObj) {
            std::string name = payloadObj->getProperty("projectName").toString().toStdString();
            if (!name.empty()) projectName = name;
        }

        // Sanitize filename
        std::string safeName;
        for (const unsigned char ch : projectName) {
            safeName.push_back((std::isalnum(ch) || ch == '-' || ch == '_') ? static_cast<char>(ch) : '_');
        }
        if (safeName.empty()) safeName = "project";

        // Save the .cendance file
        auto dir = PresetSerializer::downloadProjectDirectory();
        auto file = dir.getChildFile(juce::String(safeName + ".cendance"));
        // If file exists, append a suffix
        int suffix = 1;
        while (file.existsAsFile()) {
            file = dir.getChildFile(juce::String(safeName + "_" + std::to_string(suffix) + ".cendance"));
            suffix++;
        }
        juce::FileOutputStream fos(file);
        if (!fos.openedOk()) {
            return makeError(5001, "Failed to create project file");
        }
        fos.write(verifyResult.payload_json.data(), verifyResult.payload_json.size());
        fos.flush();

        auto* res = new juce::DynamicObject();
        res->setProperty("ok", true);
        res->setProperty("trust_level", static_cast<int>(verifyResult.trustLevel));
        res->setProperty("local_path", file.getFullPathName());
        res->setProperty("name", juce::String(projectName));
        return juce::JSON::toString(juce::var(res), false);
    }
    else if (toolName == "list_downloaded_projects") {
        auto dir = PresetSerializer::downloadProjectDirectory();
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*.cendance");
        auto* res = new juce::DynamicObject();
        juce::Array<juce::var> arr;
        for (auto& file : files) {
            auto* e = new juce::DynamicObject();
            e->setProperty("name", file.getFileNameWithoutExtension());
            e->setProperty("path", file.getFullPathName());
            e->setProperty("size", static_cast<int64_t>(file.getSize()));
            arr.add(juce::var(e));
        }
        res->setProperty("projects", juce::var(arr));
        return juce::JSON::toString(juce::var(res), false);
    }

    // ─── Unknown P2P tool ───
    return makeError(404, "Unknown P2P tool: " + toolName.toStdString());
};
