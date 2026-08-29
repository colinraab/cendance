#include "PresetSerializer.h"

#include "../App/AlgorithmPresetRegistry.h"
#include "../App/ProjectIO.h"
#include "../App/ProjectIOLoad.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace {

std::string lowerCopy(std::string text) {
    for (char& ch : text) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return text;
}

std::string sanitizeStem(const std::string& text) {
    std::string out;
    for (const unsigned char ch : text) {
        out.push_back((std::isalnum(ch) || ch == '-' || ch == '_') ? static_cast<char>(ch) : '_');
    }
    return out.empty() ? "sample" : out;
}

void addStringArray(juce::DynamicObject& object, const char* key, const std::vector<std::string>& values) {
    juce::Array<juce::var> array;
    for (const auto& value : values) array.add(juce::String(value));
    object.setProperty(key, juce::var(array));
}

std::string makeEnvelope(ContentType type, const std::string& payload, ContentHeader& header) {
    auto envelope = std::make_unique<juce::DynamicObject>();
    envelope->setProperty("content_type", static_cast<int>(type));
    envelope->setProperty("header", juce::String(header.toJson()));
    envelope->setProperty("payload", juce::String(payload));
    return juce::JSON::toString(juce::var(envelope.release()), false).toStdString();
}

} // namespace

std::string PresetSerializer::createEnvelope(const AppState& appState,
                                              SecurityManager& security,
                                              std::string& error) {
    error.clear();

    if (!security.hasKeyPair()) {
        error = "SecurityManager not initialized";
        return {};
    }

    // 1. Snapshot current state
    ProjectIO::ProjectSnapshot snapshot = ProjectIO::snapshotFromState(appState);

    auto tempFile = juce::File::createTempFile(".cendance");
    if (!ProjectIO::saveProjectFile(snapshot, tempFile.getFullPathName().toStdString(), error)) {
        tempFile.deleteFile();
        return {};
    }

    juce::String payloadStr = tempFile.loadFileAsString();
    tempFile.deleteFile();
    if (payloadStr.isEmpty()) {
        error = "Failed to serialize project snapshot";
        return {};
    }

    // 3. Hash the payload
    std::string payloadStrStd = payloadStr.toStdString();
    std::vector<uint8_t> payloadBytes(payloadStrStd.begin(), payloadStrStd.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);

    // 4. Build header
    ContentHeader header;
    header.sender_id = security.senderId();
    header.timestamp = static_cast<uint64_t>(
        juce::Time::getCurrentTime().toMilliseconds());
    header.content_hash = hashHex;
    header.content_type = ContentType::Preset;

    // 5. Sign the payload
    auto sigFuture = security.sign(payloadBytes);
    std::string signature = sigFuture.get();
    if (signature.empty()) {
        error = "Failed to sign preset data";
        return {};
    }
    header.signature = signature;

    // 6. Build transport envelope
    return makeEnvelope(ContentType::Preset, payloadStrStd, header);
}

std::future<VerificationResult> PresetSerializer::verifyAndLoad(const std::string& envelope_json,
                                                                  SecurityManager& security) {
    return std::async(std::launch::async, [envelope_json, &security]() -> VerificationResult {
        VerificationResult result;

        auto parsed = juce::JSON::parse(juce::String(envelope_json));
        if (parsed.isVoid() || !parsed.isObject()) {
            result.error = "Invalid envelope JSON";
            return result;
        }

        auto* obj = parsed.getDynamicObject();
        if (!obj) {
            result.error = "Envelope is not a JSON object";
            return result;
        }

        // Extract header JSON
        juce::String headerJson = obj->getProperty("header");
        if (headerJson.isEmpty()) {
            result.error = "Missing header in envelope";
            return result;
        }

        ContentHeader header;
        std::string headerError;
        if (!ContentHeader::fromJson(headerJson.toStdString(), header, headerError)) {
            result.error = "Failed to parse header: " + headerError;
            return result;
        }
        if (header.content_type != ContentType::Preset) {
            result.error = "Envelope is not a preset";
            return result;
        }

        // Extract payload
        juce::String payloadStr = obj->getProperty("payload");
        if (payloadStr.isEmpty()) {
            result.error = "Missing payload in envelope";
            return result;
        }

        std::string payloadStd = payloadStr.toStdString();
        std::vector<uint8_t> payloadBytes(payloadStd.begin(), payloadStd.end());

        // Re-hash and compare
        std::string rehash = SecurityManager::sha256Hex(payloadBytes);
        if (rehash != header.content_hash) {
            result.error = "Content hash mismatch — data may be tampered";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }

        // Replay protection: reject envelopes older than 5 minutes
        {
            const uint64_t nowMs = static_cast<uint64_t>(
                juce::Time::getCurrentTime().toMilliseconds());
            constexpr uint64_t kMaxEnvelopeAgeMs = 5 * 60 * 1000;
            if (header.timestamp > nowMs + 60000 ||
                header.timestamp + kMaxEnvelopeAgeMs < nowMs) {
                result.error = "Envelope timestamp too old — possible replay";
                result.trustLevel = TrustLevel::Tampered;
                return result;
            }
        }

        // Verify signature
        auto pkHex = header.sender_id;  // sender_id is the public key hex (or first 16 chars)
        // For verification we need the full public key. In a real P2P scenario,
        // the full key would be exchanged out-of-band or embedded in the header.
        // For now, we use sender_id as the key identifier and attempt verification.
        auto verifyFuture = security.verify(pkHex, payloadBytes, header.signature);
        bool sigValid = verifyFuture.get();

        if (sigValid) {
            result.ok = true;
            result.trustLevel = TrustLevel::Verified;
            result.payload_json = payloadStd;
        } else {
            // Hash matches but signature doesn't — untrusted sender
            result.ok = true;  // payload is intact
            result.trustLevel = TrustLevel::Untrusted;
            result.payload_json = payloadStd;
            result.error = "Signature verification failed — untrusted sender";
        }

        return result;
    });
}

std::string PresetSerializer::createSampleEnvelope(const std::string& audioFilePath,
                                                   const SampleEnvelopeMetadata& metadata,
                                                   SecurityManager& security,
                                                   std::string& error) {
    error.clear();
    if (!security.hasKeyPair()) {
        error = "SecurityManager not initialized";
        return {};
    }

    auto future = std::async(std::launch::async, [audioFilePath, metadata, &security, &error]() -> std::string {
        juce::File file(audioFilePath);
        if (!file.existsAsFile()) {
            error = "Sample file does not exist";
            return {};
        }
        if (static_cast<size_t>(file.getSize()) > PresetSerializer::maxSampleFileBytes()) {
            error = "Sample file exceeds size limit";
            return {};
        }
        const std::string extension = lowerCopy(file.getFileExtension().toStdString());
        const std::string format = metadata.format.empty()
            ? (extension == ".flac" ? "flac" : extension == ".ogg" ? "ogg" : "wav")
            : lowerCopy(metadata.format);
        if ((extension != ".wav" && extension != ".flac" && extension != ".ogg") || (format != "wav" && format != "flac" && format != "ogg")) {
            error = "Only WAV, FLAC, and OGG samples can be shared";
            return {};
        }

        juce::AudioFormatManager manager;
        manager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(manager.createReaderFor(file));
        if (!reader || reader->sampleRate <= 0.0 || reader->numChannels == 0 || reader->lengthInSamples <= 0) {
            error = "Sample file is not readable audio";
            return {};
        }

        juce::MemoryBlock block;
        if (!file.loadFileAsData(block)) {
            error = "Failed to read sample data";
            return {};
        }
        std::vector<uint8_t> bytes(static_cast<const uint8_t*>(block.getData()),
                                   static_cast<const uint8_t*>(block.getData()) + block.getSize());
        const std::string sha = SecurityManager::sha256Hex(bytes);

        auto payload = std::make_unique<juce::DynamicObject>();
        payload->setProperty("name", juce::String(metadata.name.empty() ? file.getFileNameWithoutExtension().toStdString() : metadata.name));
        payload->setProperty("description", juce::String(metadata.description));
        payload->setProperty("format", juce::String(format));
        payload->setProperty("sample_rate", static_cast<int>(std::llround(reader->sampleRate)));
        payload->setProperty("channels", static_cast<int>(reader->numChannels));
        payload->setProperty("duration", static_cast<double>(reader->lengthInSamples) / reader->sampleRate);
        payload->setProperty("data_base64", juce::Base64::toBase64(block.getData(), block.getSize()));
        payload->setProperty("sha256", juce::String(sha));
        addStringArray(*payload, "tags", metadata.tags);
        const std::string payloadJson = juce::JSON::toString(juce::var(payload.release()), false).toStdString();
        std::vector<uint8_t> payloadBytes(payloadJson.begin(), payloadJson.end());

        ContentHeader header;
        header.sender_id = security.senderId();
        header.timestamp = static_cast<uint64_t>(juce::Time::getCurrentTime().toMilliseconds());
        header.content_hash = SecurityManager::sha256Hex(payloadBytes);
        header.content_type = ContentType::Sample;
        header.signature = security.sign(payloadBytes).get();
        if (header.signature.empty()) {
            error = "Failed to sign sample data";
            return {};
        }
        return makeEnvelope(ContentType::Sample, payloadJson, header);
    });
    return future.get();
}

std::future<VerifiedSampleResult> PresetSerializer::verifyAndLoadSample(const std::string& envelope_json,
                                                                        SecurityManager& security) {
    return std::async(std::launch::async, [envelope_json, &security]() -> VerifiedSampleResult {
        VerifiedSampleResult result;
        auto parsed = juce::JSON::parse(juce::String(envelope_json));
        auto* obj = parsed.getDynamicObject();
        if (obj == nullptr) {
            result.error = "Invalid envelope JSON";
            return result;
        }
        ContentHeader header;
        std::string headerError;
        if (!ContentHeader::fromJson(obj->getProperty("header").toString().toStdString(), header, headerError)) {
            result.error = "Failed to parse header: " + headerError;
            return result;
        }
        if (header.content_type != ContentType::Sample) {
            result.error = "Envelope is not a sample";
            return result;
        }
        const std::string payloadStd = obj->getProperty("payload").toString().toStdString();
        std::vector<uint8_t> payloadBytes(payloadStd.begin(), payloadStd.end());
        if (SecurityManager::sha256Hex(payloadBytes) != header.content_hash) {
            result.error = "Content hash mismatch - data may be tampered";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }

        // Replay protection: reject envelopes older than 5 minutes
        {
            const uint64_t nowMs = static_cast<uint64_t>(
                juce::Time::getCurrentTime().toMilliseconds());
            constexpr uint64_t kMaxEnvelopeAgeMs = 5 * 60 * 1000;
            if (header.timestamp > nowMs + 60000 ||
                header.timestamp + kMaxEnvelopeAgeMs < nowMs) {
                result.error = "Envelope timestamp too old - possible replay";
                result.trustLevel = TrustLevel::Tampered;
                return result;
            }
        }

        result.sender_id = header.sender_id;
        result.timestamp = header.timestamp;
        result.trustLevel = security.verify(header.sender_id, payloadBytes, header.signature).get()
            ? TrustLevel::Verified
            : TrustLevel::Untrusted;

        auto payload = juce::JSON::parse(juce::String(payloadStd));
        auto* payloadObj = payload.getDynamicObject();
        if (payloadObj == nullptr) {
            result.error = "Sample payload is invalid";
            return result;
        }
        result.name = payloadObj->getProperty("name").toString().toStdString();
        result.format = lowerCopy(payloadObj->getProperty("format").toString().toStdString());
        result.sampleRate = static_cast<uint32_t>(static_cast<int64_t>(payloadObj->getProperty("sample_rate")));
        result.channels = static_cast<uint16_t>(static_cast<int64_t>(payloadObj->getProperty("channels")));
        result.duration = static_cast<double>(payloadObj->getProperty("duration"));
        result.sha256 = payloadObj->getProperty("sha256").toString().toStdString();
        if (result.format != "wav" && result.format != "flac" && result.format != "ogg") {
            result.error = "Unsupported sample format";
            return result;
        }
        juce::MemoryOutputStream decoded;
        if (!juce::Base64::convertFromBase64(decoded, payloadObj->getProperty("data_base64").toString())) {
            result.error = "Sample data is not valid base64";
            return result;
        }
        juce::MemoryBlock data = decoded.getMemoryBlock();
        std::vector<uint8_t> bytes(static_cast<const uint8_t*>(data.getData()),
                                   static_cast<const uint8_t*>(data.getData()) + data.getSize());
        if (SecurityManager::sha256Hex(bytes) != result.sha256) {
            result.error = "Decoded sample hash mismatch";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }
        auto dir = downloadSampleDirectory();
        const std::string extension = result.format == "flac" ? ".flac" : result.format == "ogg" ? ".ogg" : ".wav";
        auto file = dir.getChildFile(juce::String(sanitizeStem(result.name) + "_" + result.sha256.substr(0, 12) + extension));
        if (!file.replaceWithData(data.getData(), data.getSize())) {
            result.error = "Failed to write downloaded sample";
            return result;
        }
        result.local_path = file.getFullPathName().toStdString();
        result.ok = true;
        if (result.trustLevel == TrustLevel::Untrusted) {
            result.error = "Signature verification failed - untrusted sender";
        }
        return result;
    });
}

std::string PresetSerializer::createCustomSoundPresetEnvelope(const AppState& appState,
                                                              uint8_t trackIndex,
                                                              bool includeSamples,
                                                              SecurityManager& security,
                                                              std::string& error) {
    if (trackIndex >= AppState::kTrackCount) {
        error = "track is out of range";
        return {};
    }
    if (!includeSamples) {
        error = "Linked samples are not implemented yet; includeSamples must be true";
        return {};
    }
    ProjectIO::ProjectSnapshot snapshot = ProjectIO::snapshotFromState(appState);
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("preset_kind", "custom_sound");
    root->setProperty("track", static_cast<int>(trackIndex + 1));
    root->setProperty("include_samples", true);
    root->setProperty("linked_samples", juce::var(juce::Array<juce::var>()));
    root->setProperty("project_snapshot_note", "Track-level custom sound payload; embedded sample slots are schema placeholders in this build.");
    auto track = std::make_unique<juce::DynamicObject>();
    const auto& src = snapshot.tracks[trackIndex];
    track->setProperty("algorithmId", static_cast<int>(src.algorithmId));
    track->setProperty("synthPreset", static_cast<int>(src.synthPreset));
    track->setProperty("soundPresetRef", juce::String(src.soundPresetRef));
    track->setProperty("density", src.density);
    track->setProperty("complexity", src.complexity);
    track->setProperty("tone", src.tone);
    track->setProperty("motion", src.motion);
    track->setProperty("gain", src.gain);
    root->setProperty("track_state", juce::var(track.release()));
    const std::string payload = juce::JSON::toString(juce::var(root.release()), false).toStdString();
    std::vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    ContentHeader header;
    header.sender_id = security.senderId();
    header.timestamp = static_cast<uint64_t>(juce::Time::getCurrentTime().toMilliseconds());
    header.content_hash = SecurityManager::sha256Hex(payloadBytes);
    header.content_type = ContentType::Preset;
    header.signature = security.sign(payloadBytes).get();
    if (header.signature.empty()) {
        error = "Failed to sign custom sound preset";
        return {};
    }
    return makeEnvelope(ContentType::Preset, payload, header);
}

juce::File PresetSerializer::downloadDirectory() {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(juce::String("cendance"))
        .getChildFile(juce::String("downloads"));
    if (!dir.exists())
        dir.createDirectory();
    return dir;
}

juce::File PresetSerializer::downloadSampleDirectory() {
    auto dir = downloadDirectory().getChildFile(juce::String("samples"));
    if (!dir.exists()) dir.createDirectory();
    return dir;
}

size_t PresetSerializer::maxSampleFileBytes() {
    if (const char* raw = std::getenv("CENDANCE_MAX_SAMPLE_FILE_BYTES")) {
        try {
            const auto parsed = std::stoull(raw);
            if (parsed > 0) return static_cast<size_t>(parsed);
        } catch (...) {
        }
    }
    return 50u * 1024u * 1024u;
}

std::string PresetSerializer::createAlgorithmEnvelope(const CustomAlgorithmPreset& preset,
                                                      SecurityManager& security,
                                                      std::string& error) {
    ContentHeader header;
    header.sender_id = security.publicKeyHex();
    header.content_type = ContentType::Algorithm;
    header.timestamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    std::string payloadJson = toJson(preset);
    std::vector<uint8_t> payloadBytes(payloadJson.begin(), payloadJson.end());
    header.content_hash = SecurityManager::sha256Hex(payloadBytes);

    // Sign the payload bytes (not the hash)
    auto sigFuture = security.sign(payloadBytes);
    std::string sigHex = sigFuture.get();
    if (sigHex.empty()) {
        error = "Signing failed";
        return "";
    }
    header.signature = sigHex;

    auto envelope = std::make_unique<juce::DynamicObject>();
    envelope->setProperty("content_type", static_cast<int>(ContentType::Algorithm));
    envelope->setProperty("header", juce::String(header.toJson()));
    envelope->setProperty("payload", juce::String(payloadJson));
    return juce::JSON::toString(juce::var(envelope.release()), false).toStdString();
}

std::future<VerificationResult> PresetSerializer::verifyAndLoadAlgorithm(const std::string& envelope_json,
                                                                          SecurityManager& security) {
    return std::async(std::launch::async, [envelope_json, &security]() -> VerificationResult {
        VerificationResult result;
        auto parsed = juce::JSON::parse(juce::String(envelope_json));
        if (parsed.isVoid() || !parsed.isObject()) {
            result.error = "Invalid envelope JSON";
            return result;
        }
        auto* obj = parsed.getDynamicObject();
        if (obj == nullptr) {
            result.error = "Invalid envelope object";
            return result;
        }
        auto headerJson = obj->getProperty("header").toString().toStdString();
        ContentHeader header;
        std::string err;
        if (!ContentHeader::fromJson(headerJson, header, err)) {
            result.error = "Invalid content header: " + err;
            return result;
        }
        if (header.content_type != ContentType::Algorithm) {
            result.error = "Expected Algorithm content type";
            return result;
        }
        auto payloadJson = obj->getProperty("payload").toString().toStdString();
        std::vector<uint8_t> payloadBytes(payloadJson.begin(), payloadJson.end());
        std::string computedHash = SecurityManager::sha256Hex(payloadBytes);
        if (computedHash != header.content_hash) {
            result.error = "Payload hash mismatch";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }

        // Replay protection: reject envelopes older than 5 minutes
        {
            const uint64_t nowMs = static_cast<uint64_t>(
                juce::Time::getCurrentTime().toMilliseconds());
            constexpr uint64_t kMaxEnvelopeAgeMs = 5 * 60 * 1000;
            if (header.timestamp > nowMs + 60000 ||
                header.timestamp + kMaxEnvelopeAgeMs < nowMs) {
                result.error = "Envelope timestamp too old - possible replay";
                result.trustLevel = TrustLevel::Tampered;
                return result;
            }
        }

        // Verify signature against payload bytes
        auto verifyFuture = security.verify(header.sender_id, payloadBytes, header.signature);
        if (!verifyFuture.get()) {
            result.error = "Signature verification failed";
            result.trustLevel = TrustLevel::Untrusted;
            return result;
        }
        auto presetOpt = fromJson(payloadJson, err);
        if (!presetOpt.has_value()) {
            result.error = "Failed to parse algorithm preset: " + err;
            return result;
        }
        auto& registry = const_cast<AlgorithmPresetRegistry&>(globalAlgorithmPresetRegistry());
        if (!registry.savePreset(presetOpt.value(), err)) {
            result.error = "Failed to install algorithm: " + err;
            return result;
        }
        result.ok = true;
        result.trustLevel = TrustLevel::Verified;
        result.payload_json = payloadJson;
        return result;
    });
}

//==============================================================================
// Arrangement preset sharing
//==============================================================================

std::string PresetSerializer::createArrangementEnvelope(const AppState& appState,
                                                        const std::string& name,
                                                        SecurityManager& security,
                                                        std::string& error) {
    error.clear();

    if (!security.hasKeyPair()) {
        error = "SecurityManager not initialized";
        return {};
    }

    // 1. Snapshot arrangement state
    auto item = ArrangementPresetManager::snapshotFromState(appState, name);

    // 2. Serialize to JSON payload
    std::string payload = ArrangementPresetManager::presetToJson(item);

    // 3. Hash the payload
    std::vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);

    // 4. Build header
    ContentHeader header;
    header.content_type = ContentType::Preset;
    header.content_hash = hashHex;
    header.sender_id = security.publicKeyHex();
    header.timestamp = static_cast<uint64_t>(juce::Time::getCurrentTime().toMilliseconds());

    // 5. Sign the payload bytes
    auto sig = security.sign(payloadBytes);
    if (!sig.valid()) {
        error = "Failed to start signing operation";
        return {};
    }
    std::string sigStr = sig.get();
    if (sigStr.empty()) {
        error = "Failed to sign arrangement preset";
        return {};
    }
    header.signature = sigStr;

    // 6. Build envelope
    return makeEnvelope(ContentType::Preset, payload, header);
}

std::future<VerificationResult> PresetSerializer::verifyAndLoadArrangement(const std::string& envelope_json,
                                                                            SecurityManager& security) {
    return std::async(std::launch::async, [envelope_json, &security]() -> VerificationResult {
        VerificationResult result;

        auto var = juce::JSON::parse(envelope_json);
        if (var.isVoid()) {
            result.error = "Failed to parse envelope JSON";
            return result;
        }

        auto* obj = var.getDynamicObject();
        if (obj == nullptr) {
            result.error = "Envelope is not a JSON object";
            return result;
        }

        // Extract header
        ContentHeader header;
        {
            auto headerJson = obj->getProperty("header").toString();
            std::string headerError;
            if (!ContentHeader::fromJson(headerJson.toStdString(), header, headerError)) {
                result.error = "Failed to parse content header: " + headerError;
                return result;
            }
        }

        // Verify content type
        if (header.content_type != ContentType::Preset) {
            result.error = "Expected Preset content type for arrangement";
            return result;
        }

        // Extract and verify payload hash
        auto payloadJson = obj->getProperty("payload").toString().toStdString();
        std::vector<uint8_t> payloadBytes(payloadJson.begin(), payloadJson.end());
        std::string computedHash = SecurityManager::sha256Hex(payloadBytes);
        if (computedHash != header.content_hash) {
            result.error = "Payload hash mismatch";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }

        // Replay protection: reject envelopes older than 5 minutes
        {
            const uint64_t nowMs = static_cast<uint64_t>(
                juce::Time::getCurrentTime().toMilliseconds());
            constexpr uint64_t kMaxEnvelopeAgeMs = 5 * 60 * 1000;
            if (header.timestamp > nowMs + 60000 ||
                header.timestamp + kMaxEnvelopeAgeMs < nowMs) {
                result.error = "Envelope timestamp too old - possible replay";
                result.trustLevel = TrustLevel::Tampered;
                return result;
            }
        }

        // Verify signature
        auto verifyFuture = security.verify(header.sender_id, payloadBytes, header.signature);
        if (!verifyFuture.get()) {
            result.error = "Signature verification failed";
            result.trustLevel = TrustLevel::Untrusted;
            return result;
        }

        result.ok = true;
        result.trustLevel = TrustLevel::Verified;
        result.payload_json = payloadJson;
        return result;
    });
}

//==============================================================================
// Project file sharing
//==============================================================================

std::string PresetSerializer::createProjectEnvelope(const AppState& appState,
                                                    const std::string& name,
                                                    SecurityManager& security,
                                                    std::string& error) {
    error.clear();

    if (!security.hasKeyPair()) {
        error = "SecurityManager not initialized";
        return {};
    }

    // 1. Snapshot current state
    ProjectIO::ProjectSnapshot snapshot = ProjectIO::snapshotFromState(appState);
    if (!name.empty()) {
        snapshot.projectName = name;
    }

    // 2. Serialize to temp file (produces .cendance JSON)
    auto tempFile = juce::File::createTempFile(".cendance");
    if (!ProjectIO::saveProjectFile(snapshot, tempFile.getFullPathName().toStdString(), error)) {
        tempFile.deleteFile();
        return {};
    }

    juce::String payloadStr = tempFile.loadFileAsString();
    tempFile.deleteFile();
    if (payloadStr.isEmpty()) {
        error = "Failed to serialize project snapshot";
        return {};
    }

    // 3. Hash the payload
    std::string payloadStrStd = payloadStr.toStdString();
    std::vector<uint8_t> payloadBytes(payloadStrStd.begin(), payloadStrStd.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);

    // 4. Build header
    ContentHeader header;
    header.sender_id = security.publicKeyHex();
    header.timestamp = static_cast<uint64_t>(
        juce::Time::getCurrentTime().toMilliseconds());
    header.content_hash = hashHex;
    header.content_type = ContentType::Project;

    // 5. Sign the payload bytes
    auto sigFuture = security.sign(payloadBytes);
    std::string signature = sigFuture.get();
    if (signature.empty()) {
        error = "Failed to sign project data";
        return {};
    }
    header.signature = signature;

    // 6. Build transport envelope
    return makeEnvelope(ContentType::Project, payloadStrStd, header);
}

std::future<VerificationResult> PresetSerializer::verifyAndLoadProject(const std::string& envelope_json,
                                                                       SecurityManager& security) {
    return std::async(std::launch::async, [envelope_json, &security]() -> VerificationResult {
        VerificationResult result;

        auto parsed = juce::JSON::parse(juce::String(envelope_json));
        if (parsed.isVoid() || !parsed.isObject()) {
            result.error = "Invalid envelope JSON";
            return result;
        }

        auto* obj = parsed.getDynamicObject();
        if (!obj) {
            result.error = "Envelope is not a JSON object";
            return result;
        }

        // Extract header JSON
        juce::String headerJson = obj->getProperty("header");
        if (headerJson.isEmpty()) {
            result.error = "Missing header in envelope";
            return result;
        }

        ContentHeader header;
        std::string headerError;
        if (!ContentHeader::fromJson(headerJson.toStdString(), header, headerError)) {
            result.error = "Failed to parse header: " + headerError;
            return result;
        }
        if (header.content_type != ContentType::Project) {
            result.error = "Envelope is not a project";
            return result;
        }

        // Extract payload
        juce::String payloadStr = obj->getProperty("payload");
        if (payloadStr.isEmpty()) {
            result.error = "Missing payload in envelope";
            return result;
        }

        std::string payloadStd = payloadStr.toStdString();
        std::vector<uint8_t> payloadBytes(payloadStd.begin(), payloadStd.end());

        // Re-hash and compare
        std::string rehash = SecurityManager::sha256Hex(payloadBytes);
        if (rehash != header.content_hash) {
            result.error = "Content hash mismatch — data may be tampered";
            result.trustLevel = TrustLevel::Tampered;
            return result;
        }

        // Replay protection: reject envelopes older than 5 minutes
        {
            const uint64_t nowMs = static_cast<uint64_t>(
                juce::Time::getCurrentTime().toMilliseconds());
            constexpr uint64_t kMaxEnvelopeAgeMs = 5 * 60 * 1000;
            if (header.timestamp > nowMs + 60000 ||
                header.timestamp + kMaxEnvelopeAgeMs < nowMs) {
                result.error = "Envelope timestamp too old — possible replay";
                result.trustLevel = TrustLevel::Tampered;
                return result;
            }
        }

        // Verify signature
        auto verifyFuture = security.verify(header.sender_id, payloadBytes, header.signature);
        bool sigValid = verifyFuture.get();

        if (sigValid) {
            result.ok = true;
            result.trustLevel = TrustLevel::Verified;
            result.payload_json = payloadStd;
        } else {
            result.ok = true;
            result.trustLevel = TrustLevel::Untrusted;
            result.payload_json = payloadStd;
            result.error = "Signature verification failed — untrusted sender";
        }

        return result;
    });
}

juce::File PresetSerializer::downloadProjectDirectory() {
    auto dir = downloadDirectory().getChildFile(juce::String("projects"));
    if (!dir.exists()) dir.createDirectory();
    return dir;
}
