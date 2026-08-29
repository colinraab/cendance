#include "DrumSampleLibrary.h"

#include "DrumKitBinaryData.h"
#include "DrumKitPresetCatalog.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <sstream>

namespace {

uint32_t fnv1a32(const std::string& text) {
    uint32_t hash = 2166136261u;
    for (const unsigned char ch : text) {
        hash ^= static_cast<uint32_t>(ch);
        hash *= 16777619u;
    }
    return hash;
}

} // namespace

DrumSampleLibrary::DrumSampleLibrary()
    : globalSampleDirectory([]() {
#if defined(_WIN32)
        const char* home = std::getenv("USERPROFILE");
#else
        const char* home = std::getenv("HOME");
#endif

        if (home != nullptr && home[0] != '\0') {
            return (std::filesystem::path(home) / "Documents" / "cendance" / "Samples" / "Drums").string();
        }

        return (std::filesystem::current_path() / "cendance" / "Samples" / "Drums").string();
    }()) {
    formatManager.registerBasicFormats();
    for (auto& ptr : rtSamplePointers) {
        ptr.store(nullptr, std::memory_order_relaxed);
    }
}

const std::string& DrumSampleLibrary::getGlobalSampleDirectory() const {
    return globalSampleDirectory;
}

bool DrumSampleLibrary::preloadEmbeddedDrumKits(std::string& error) {
    error.clear();

    bool hadFailure = false;
    std::ostringstream failures;

    for (const auto& sample : DrumKitPresetCatalog::kEmbeddedSamples) {
        int dataSize = 0;
        const void* data = DrumKitBinaryData::getNamedResource(sample.resourceName.data(), dataSize);
        if (data == nullptr || dataSize <= 0) {
            hadFailure = true;
            failures << "Missing embedded resource: " << sample.resourceName << "\n";
            continue;
        }

        std::string loadError;
        const std::string sourcePath = "embedded://" + std::string(sample.resourceName);
        if (!registerEmbeddedSample(sample.sampleId,
                                    data,
                                    static_cast<size_t>(dataSize),
                                    sourcePath,
                                    std::string(sample.displayName),
                                    loadError)) {
            hadFailure = true;
            failures << "Failed to load embedded resource " << sample.resourceName
                     << ": " << loadError << "\n";
        }
    }

    if (hadFailure) {
        error = failures.str();
    }

    return !hadFailure;
}

bool DrumSampleLibrary::ensureGlobalSampleDirectory(std::string& error) const {
    error.clear();
    try {
        std::filesystem::create_directories(std::filesystem::path(globalSampleDirectory));
    } catch (const std::exception& ex) {
        error = std::string("Unable to create drum sample directory: ") + ex.what();
        return false;
    }

    return true;
}

bool DrumSampleLibrary::isSupportedAudioFile(const std::string& extension) {
    std::string lower;
    lower.reserve(extension.size());
    for (const unsigned char ch : extension) {
        lower.push_back(static_cast<char>(std::tolower(ch)));
    }

    return lower == ".wav" || lower == ".aif" || lower == ".aiff" || lower == ".flac" || lower == ".ogg";
}

std::string DrumSampleLibrary::normalizePath(const std::string& path) {
    std::filesystem::path p(path);
    if (p.is_relative()) {
        p = std::filesystem::absolute(p);
    }

    std::error_code ec;
    const std::filesystem::path weak = std::filesystem::weakly_canonical(p, ec);
    if (!ec) {
        return weak.lexically_normal().string();
    }

    return p.lexically_normal().string();
}

std::string DrumSampleLibrary::sanitizeStem(const std::string& stem) {
    std::string out;
    out.reserve(stem.size());
    for (const unsigned char ch : stem) {
        if (std::isalnum(ch) || ch == '-' || ch == '_') {
            out.push_back(static_cast<char>(ch));
        } else {
            out.push_back('_');
        }
    }

    if (out.empty()) {
        out = "sample";
    }

    return out;
}

uint16_t DrumSampleLibrary::computeStableUserSampleIdLocked(const std::string& normalizedPath) const {
    const uint32_t hash = fnv1a32(normalizedPath);
    const uint16_t mask = Command::kDrumSampleIdMask;
    uint16_t start = static_cast<uint16_t>(hash & mask);
    if (start == 0) {
        start = 1;
    }

    for (uint16_t offset = 0; offset < mask; ++offset) {
        const uint16_t candidate = static_cast<uint16_t>(((start - 1u + offset) % mask) + 1u);
        if (DrumKitPresetCatalog::isEmbeddedSampleId(candidate)) {
            continue;
        }

        if (sampleStorage[candidate] == nullptr || samplePathById[candidate] == normalizedPath) {
            return candidate;
        }
    }

    return 0;
}

bool DrumSampleLibrary::loadSampleFromReader(juce::AudioFormatReader& reader,
                                             const std::string& sourcePath,
                                             const std::string& displayName,
                                             std::shared_ptr<DrumSampleData>& outData,
                                             std::string& error) const {
    error.clear();

    if (reader.lengthInSamples <= 0) {
        error = "Sample source is empty.";
        return false;
    }

    const int numChannels = std::max(1, std::min<int>(2, static_cast<int>(reader.numChannels)));
    const int numSamples = static_cast<int>(
        std::min<int64_t>(reader.lengthInSamples, static_cast<int64_t>(10 * reader.sampleRate)));

    auto sampleData = std::make_shared<DrumSampleData>();
    sampleData->audio.setSize(numChannels, numSamples, false, true, true);
    if (!reader.read(&sampleData->audio, 0, numSamples, 0, true, true)) {
        error = "Failed reading sample data.";
        return false;
    }

    sampleData->sourceSampleRate = reader.sampleRate;
    sampleData->path = sourcePath;
    sampleData->name = displayName.empty()
        ? std::filesystem::path(sourcePath).stem().string()
        : displayName;
    outData = std::move(sampleData);
    return true;
}

bool DrumSampleLibrary::loadSampleFile(const std::string& normalizedPath,
                                       std::shared_ptr<DrumSampleData>& outData,
                                       std::string& error) {
    error.clear();
    juce::File file(normalizedPath);
    if (!file.existsAsFile()) {
        error = "Sample file does not exist.";
        return false;
    }

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader) {
        error = "Unsupported or unreadable audio file.";
        return false;
    }

    return loadSampleFromReader(*reader,
                                normalizedPath,
                                std::filesystem::path(normalizedPath).stem().string(),
                                outData,
                                error);
}

bool DrumSampleLibrary::loadEmbeddedSample(const void* data,
                                           size_t sizeBytes,
                                           const std::string& sourcePath,
                                           const std::string& displayName,
                                           std::shared_ptr<DrumSampleData>& outData,
                                           std::string& error) {
    error.clear();

    if (data == nullptr || sizeBytes == 0) {
        error = "Embedded sample data is empty.";
        return false;
    }

    auto memoryStream = std::make_unique<juce::MemoryInputStream>(data, sizeBytes, false);
    juce::OggVorbisAudioFormat oggFormat;
    std::unique_ptr<juce::AudioFormatReader> reader(oggFormat.createReaderFor(memoryStream.release(), true));
    if (!reader) {
        error = "Failed to decode embedded OGG sample.";
        return false;
    }

    return loadSampleFromReader(*reader, sourcePath, displayName, outData, error);
}

bool DrumSampleLibrary::registerLoadedSampleLocked(uint16_t sampleId,
                                                   const std::shared_ptr<DrumSampleData>& sampleData,
                                                   const std::string& normalizedPath,
                                                   const std::string& displayName,
                                                   std::string& error) {
    error.clear();
    if (sampleId == 0 || sampleId >= kMaxSampleIds) {
        error = "Sample ID is out of range.";
        return false;
    }

    sampleStorage[sampleId] = sampleData;
    samplePathById[sampleId] = normalizedPath;
    sampleNameById[sampleId] = displayName;
    rtSamplePointers[sampleId].store(sampleData.get(), std::memory_order_release);
    return true;
}

bool DrumSampleLibrary::registerSamplePath(const std::string& path,
                                           uint16_t& outSampleId,
                                           std::string& error) {
    error.clear();
    outSampleId = 0;

    const std::string normalizedPath = normalizePath(path);
    const std::filesystem::path fsPath(normalizedPath);
    if (!std::filesystem::exists(fsPath) || !std::filesystem::is_regular_file(fsPath)) {
        error = "Sample file does not exist.";
        return false;
    }

    if (!isSupportedAudioFile(fsPath.extension().string())) {
        error = "Only wav/aiff/flac/ogg drum samples are supported.";
        return false;
    }

    std::shared_ptr<DrumSampleData> loaded;
    if (!loadSampleFile(normalizedPath, loaded, error)) {
        return false;
    }

    std::scoped_lock<std::mutex> lock(sampleMutex);
    const uint16_t sampleId = computeStableUserSampleIdLocked(normalizedPath);
    if (sampleId == 0) {
        error = "Drum sample registry is full.";
        return false;
    }

    if (!registerLoadedSampleLocked(sampleId, loaded, normalizedPath, loaded->name, error)) {
        return false;
    }

    outSampleId = sampleId;
    return true;
}

bool DrumSampleLibrary::registerEmbeddedSample(uint16_t sampleId,
                                               const void* data,
                                               size_t sizeBytes,
                                               const std::string& sourcePath,
                                               const std::string& displayName,
                                               std::string& error) {
    error.clear();
    if (!DrumKitPresetCatalog::isEmbeddedSampleId(sampleId)) {
        error = "Embedded sample ID is outside reserved range.";
        return false;
    }

    std::shared_ptr<DrumSampleData> loaded;
    if (!loadEmbeddedSample(data, sizeBytes, sourcePath, displayName, loaded, error)) {
        return false;
    }

    std::scoped_lock<std::mutex> lock(sampleMutex);
    return registerLoadedSampleLocked(sampleId, loaded, sourcePath, displayName, error);
}

bool DrumSampleLibrary::rescanGlobalDirectory(std::string& error) {
    error.clear();
    if (!ensureGlobalSampleDirectory(error)) {
        return false;
    }

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(globalSampleDirectory, ec)) {
        if (ec) {
            error = std::string("Failed while scanning drum sample directory: ") + ec.message();
            return false;
        }

        if (!entry.is_regular_file()) {
            continue;
        }

        if (!isSupportedAudioFile(entry.path().extension().string())) {
            continue;
        }

        uint16_t ignoredId = 0;
        std::string ignoredError;
        registerSamplePath(entry.path().string(), ignoredId, ignoredError);
    }

    return true;
}

bool DrumSampleLibrary::importSampleFromPath(const std::string& sourcePath,
                                             uint16_t& outSampleId,
                                             std::string& error) {
    error.clear();
    outSampleId = 0;

    if (!ensureGlobalSampleDirectory(error)) {
        return false;
    }

    const std::string normalizedSource = normalizePath(sourcePath);
    const std::filesystem::path source(normalizedSource);
    if (!std::filesystem::exists(source) || !std::filesystem::is_regular_file(source)) {
        error = "Source sample path does not exist.";
        return false;
    }

    if (!isSupportedAudioFile(source.extension().string())) {
        error = "Only wav/aiff/flac/ogg drum samples are supported.";
        return false;
    }

    const std::filesystem::path globalDir(globalSampleDirectory);
    std::filesystem::path destination = globalDir / source.filename();
    if (!std::filesystem::equivalent(source.parent_path(), globalDir)) {
        std::string stem = sanitizeStem(source.stem().string());
        const std::string extension = source.extension().string();
        int suffix = 1;
        while (std::filesystem::exists(destination)) {
            destination = globalDir / (stem + "_" + std::to_string(suffix) + extension);
            ++suffix;
        }

        std::error_code copyError;
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, copyError);
        if (copyError) {
            error = std::string("Failed to copy sample into library: ") + copyError.message();
            return false;
        }
    }

    return registerSamplePath(destination.string(), outSampleId, error);
}

std::vector<DrumSampleLibrary::SampleRecord> DrumSampleLibrary::listSamples() const {
    std::vector<SampleRecord> records;
    std::scoped_lock<std::mutex> lock(sampleMutex);
    for (uint16_t id = 1; id < kMaxSampleIds; ++id) {
        if (sampleStorage[id] == nullptr) {
            continue;
        }

        records.push_back(SampleRecord{
            id,
            sampleNameById[id],
            samplePathById[id],
            sampleStorage[id]->audio.getNumSamples(),
            sampleStorage[id]->sourceSampleRate,
        });
    }

    std::sort(records.begin(), records.end(), [](const SampleRecord& lhs, const SampleRecord& rhs) {
        return lhs.id < rhs.id;
    });
    return records;
}

std::string DrumSampleLibrary::getSampleName(uint16_t sampleId) const {
    if (sampleId == 0 || sampleId >= kMaxSampleIds) {
        return "";
    }

    std::scoped_lock<std::mutex> lock(sampleMutex);
    return sampleNameById[sampleId];
}

std::string DrumSampleLibrary::getSamplePath(uint16_t sampleId) const {
    if (sampleId == 0 || sampleId >= kMaxSampleIds) {
        return "";
    }

    std::scoped_lock<std::mutex> lock(sampleMutex);
    return samplePathById[sampleId];
}

bool DrumSampleLibrary::hasSample(uint16_t sampleId) const {
    if (sampleId == 0 || sampleId >= kMaxSampleIds) {
        return false;
    }

    return rtSamplePointers[sampleId].load(std::memory_order_acquire) != nullptr;
}

const DrumSampleData* DrumSampleLibrary::getRtSample(uint16_t sampleId) const {
    if (sampleId == 0 || sampleId >= kMaxSampleIds) {
        return nullptr;
    }

    return rtSamplePointers[sampleId].load(std::memory_order_acquire);
}
