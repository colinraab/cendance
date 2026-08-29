#pragma once

#include "ContentHeader.h"
#include "../App/CustomAlgorithmPreset.h"
#include "../App/ProjectIO.h"
#include "../App/ArrangementPresetManager.h"
#include "SecurityManager.h"
#include "../App/AppState.h"

#include <cstdint>
#include <future>
#include <string>
#include <vector>

// Trust level for verified incoming presets.
enum class TrustLevel : uint8_t {
    Verified = 0,
    Untrusted = 1,
    Tampered = 2,
};

struct VerificationResult {
    bool ok = false;
    TrustLevel trustLevel = TrustLevel::Tampered;
    std::string payload_json;
    std::string error;
};

struct SampleEnvelopeMetadata {
    std::string name;
    std::string description;
    std::string format;
    std::vector<std::string> tags;
};

struct VerifiedSampleResult {
    bool ok = false;
    TrustLevel trustLevel = TrustLevel::Tampered;
    std::string local_path;
    std::string name;
    std::string format;
    std::string sha256;
    std::string sender_id;
    std::string error;
    uint32_t sampleRate = 0;
    uint16_t channels = 0;
    double duration = 0.0;
    uint64_t timestamp = 0;
};

class PresetSerializer {
public:
    PresetSerializer() = default;
    ~PresetSerializer() = default;

    std::string createEnvelope(const AppState& appState,
                               SecurityManager& security,
                               std::string& error);

    std::future<VerificationResult> verifyAndLoad(const std::string& envelope_json,
                                                   SecurityManager& security);

    std::string createSampleEnvelope(const std::string& audioFilePath,
                                     const SampleEnvelopeMetadata& metadata,
                                     SecurityManager& security,
                                     std::string& error);

    std::future<VerifiedSampleResult> verifyAndLoadSample(const std::string& envelope_json,
                                                          SecurityManager& security);

    std::string createCustomSoundPresetEnvelope(const AppState& appState,
                                                uint8_t trackIndex,
                                                bool includeSamples,
                                                SecurityManager& security,
                                                std::string& error);

    std::string createAlgorithmEnvelope(const CustomAlgorithmPreset& preset,
                                        SecurityManager& security,
                                        std::string& error);

    std::future<VerificationResult> verifyAndLoadAlgorithm(const std::string& envelope_json,
                                                          SecurityManager& security);

    // --- Arrangement preset sharing ---
    std::string createArrangementEnvelope(const AppState& appState,
                                          const std::string& name,
                                          SecurityManager& security,
                                          std::string& error);

    std::future<VerificationResult> verifyAndLoadArrangement(const std::string& envelope_json,
                                                            SecurityManager& security);

    // --- Project file sharing ---
    std::string createProjectEnvelope(const AppState& appState,
                                      const std::string& name,
                                      SecurityManager& security,
                                      std::string& error);

    std::future<VerificationResult> verifyAndLoadProject(const std::string& envelope_json,
                                                         SecurityManager& security);

    static juce::File downloadDirectory();
    static juce::File downloadSampleDirectory();
    static juce::File downloadProjectDirectory();
    static size_t maxSampleFileBytes();

private:
    std::string buildEnvelopeJson(const ProjectIO::ProjectSnapshot& snapshot,
                                  const ContentHeader& header) const;
};
