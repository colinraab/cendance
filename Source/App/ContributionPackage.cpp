#include "ContributionPackage.h"
#include "ContributionPackageParse.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace ContributionPackage {

std::string kindToString(Kind kind) {
    switch (kind) {
    case Kind::EffectPresetPack: return "effectPresetPack";
    case Kind::SoundPresetPack: return "soundPresetPack";
    case Kind::DrumKitPresetPack: return "drumKitPresetPack";
    case Kind::ScenePresetPack: return "scenePresetPack";
    case Kind::ArrangementPresetPack: return "arrangementPresetPack";
    case Kind::SamplePack: return "samplePack";
    }
    return "unknown";
}

std::optional<Kind> kindFromString(const std::string& text) {
    const std::string lower = lowerCopy(text);
    if (lower == "effectpresetpack") return Kind::EffectPresetPack;
    if (lower == "soundpresetpack") return Kind::SoundPresetPack;
    if (lower == "drumkitpresetpack") return Kind::DrumKitPresetPack;
    if (lower == "scenepresetpack") return Kind::ScenePresetPack;
    if (lower == "arrangementpresetpack") return Kind::ArrangementPresetPack;
    if (lower == "samplepack") return Kind::SamplePack;
    return std::nullopt;
}

Library::Library()
    : rootDirectory(defaultRootDirectory()),
      stagingDirectory((std::filesystem::path(rootDirectory) / "Staging").string()),
      installedDirectory((std::filesystem::path(rootDirectory) / "Installed").string()),
      payloadDirectory((std::filesystem::path(rootDirectory) / "Payloads").string()) {}

bool Library::ensureDirectories(std::string& error) const {
    try {
        std::filesystem::create_directories(stagingDirectory);
        std::filesystem::create_directories(installedDirectory);
        std::filesystem::create_directories(payloadDirectory);
        return true;
    } catch (const std::exception& ex) {
        error = std::string("Unable to create contribution directories: ") + ex.what();
        return false;
    }
}

Preview Library::previewFile(const std::string& path) const {
    Preview preview;
    std::string error;
    const std::string text = readFile(path, error);
    if (!error.empty()) {
        preview.errors.push_back(error);
        return preview;
    }
    parsePackage(text, path, preview);
    return preview;
}

bool Library::installFile(const std::string& path, Preview& preview, std::string& error) {
    preview = previewFile(path);
    if (!preview.ok) {
        error = preview.errors.empty() ? "Package preview failed." : preview.errors.front();
        return false;
    }
    if (!ensureDirectories(error)) {
        return false;
    }
    const std::filesystem::path destination = std::filesystem::path(installedDirectory)
        / packageFileName(preview.package.packageId);
    try {
        if (preview.package.kind == Kind::SamplePack) {
            const std::filesystem::path sourceDir = std::filesystem::path(path).parent_path();
            const std::filesystem::path payloadRoot = std::filesystem::path(payloadDirectory) / preview.package.packageId;
            for (const auto& item : preview.package.samplePacks) {
                const std::filesystem::path relative(item.filePath);
                const std::filesystem::path destinationPayload = payloadRoot / relative;
                std::filesystem::create_directories(destinationPayload.parent_path());
                std::filesystem::copy_file(sourceDir / relative,
                                           destinationPayload,
                                           std::filesystem::copy_options::overwrite_existing);
            }
        }
        std::filesystem::copy_file(path, destination, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception& ex) {
        error = std::string("Package install failed: ") + ex.what();
        return false;
    }
    return reloadInstalled(error);
}

bool Library::removePackage(const std::string& packageId, std::string& error) {
    if (!validateIdText(packageId, "package id", error)) {
        return false;
    }
    const std::filesystem::path path = std::filesystem::path(installedDirectory) / packageFileName(packageId);
    try {
        if (!std::filesystem::exists(path)) {
            error = "Package is not installed.";
            return false;
        }
        std::filesystem::remove(path);
        std::filesystem::remove_all(std::filesystem::path(payloadDirectory) / packageId);
    } catch (const std::exception& ex) {
        error = std::string("Package removal failed: ") + ex.what();
        return false;
    }
    return reloadInstalled(error);
}

bool Library::reloadInstalled(std::string& error) {
    if (!ensureDirectories(error)) {
        return false;
    }
    std::vector<Package> loaded;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(installedDirectory)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto path = entry.path();
            if (path.extension() != ".json") {
                continue;
            }
            auto preview = previewFile(path.string());
            if (!preview.ok) {
                continue;
            }
            preview.package.installed = true;
            preview.package.installedPath = path.string();
            loaded.push_back(preview.package);
        }
    } catch (const std::exception& ex) {
        error = std::string("Package reload failed: ") + ex.what();
        return false;
    }
    std::sort(loaded.begin(), loaded.end(), [](const Package& a, const Package& b) {
        return a.packageId < b.packageId;
    });
    packages = std::move(loaded);
    return true;
}

bool Library::exportPackageTemplate(const std::string& path,
                                    Kind kind,
                                    const std::string& packageId,
                                    const std::string& name,
                                    std::string& error) const {
    if (!validateIdText(packageId, "package id", error)) {
        return false;
    }
    const std::string itemKind = kindToString(kind);
    std::ostringstream json;
    json << "{\n"
         << "  \"schema\": \"cendancePackage.v1\",\n"
         << "  \"id\": " << quoted(packageId) << ",\n"
         << "  \"version\": \"0.1.0\",\n"
         << "  \"kind\": " << quoted(itemKind) << ",\n"
         << "  \"name\": " << quoted(name) << ",\n"
         << "  \"description\": \"Exported cendance contribution package.\",\n"
         << "  \"authorAgent\": \"local-agent\",\n"
         << "  \"createdAt\": " << quoted(juce::Time::getCurrentTime().toISO8601(true).toStdString()) << ",\n"
         << "  \"license\": \"UNSPECIFIED\",\n"
         << "  \"compatibility\": {\n"
         << "    \"minPackageSchema\": \"cendancePackage.v1\",\n"
         << "    \"maxPackageSchema\": \"cendancePackage.v1\"\n"
         << "  },\n"
         << "  \"contentHash\": \"\",\n"
         << "  \"signature\": \"\",\n"
         << "  \"dependencies\": [],\n"
         << "  \"tags\": [],\n";
    if (kind == Kind::SamplePack) {
        json << "  \"payloads\": [\"Samples/example.wav\"],\n"
             << "  \"items\": [\n"
             << "    {\n"
             << "      \"id\": \"example-sample\",\n"
             << "      \"name\": \"Example Sample\",\n"
             << "      \"description\": \"A WAV or FLAC sample payload.\",\n"
             << "      \"filePath\": \"Samples/example.wav\",\n"
             << "      \"format\": \"wav\",\n"
             << "      \"sampleRate\": 44100,\n"
             << "      \"channels\": 2,\n"
             << "      \"duration\": 1.0,\n"
             << "      \"sha256\": \"replace-with-audio-file-sha256\",\n"
             << "      \"tags\": []\n"
             << "    }\n"
             << "  ]\n";
    } else {
        json << "  \"items\": []\n";
    }
    json << "}\n";
    try {
        const std::filesystem::path output(path);
        if (output.has_parent_path()) {
            std::filesystem::create_directories(output.parent_path());
        }
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            error = "Could not open export path.";
            return false;
        }
        out << json.str();
    } catch (const std::exception& ex) {
        error = std::string("Package export failed: ") + ex.what();
        return false;
    }
    return true;
}

const Package* Library::findPackage(const std::string& packageId) const {
    for (const auto& package : packages) {
        if (package.packageId == packageId) {
            return &package;
        }
    }
    return nullptr;
}

const SoundPresetItem* Library::findSoundPreset(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->soundPresets) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

const EffectPresetItem* Library::findEffectPreset(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->effectPresets) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

const DrumKitPresetItem* Library::findDrumKitPreset(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->drumKitPresets) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

const ArrangementPresetItem* Library::findArrangementPreset(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->arrangementPresets) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

const ScenePresetItem* Library::findScenePreset(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->scenePresets) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

const SamplePackItem* Library::findSamplePackItem(const std::string& packageId, const std::string& itemId) const {
    if (const auto* package = findPackage(packageId)) {
        for (const auto& item : package->samplePacks) {
            if (item.itemId == itemId) return &item;
        }
    }
    return nullptr;
}

std::string Library::packagesJson(bool includeItems) const {
    std::ostringstream out;
    out << "\"packages\":[";
    for (size_t i = 0; i < packages.size(); ++i) {
        if (i > 0) out << ",";
        appendPackageSummary(out, packages[i], includeItems);
    }
    out << "]";
    return out.str();
}

std::string Library::contributionCatalogJson() const {
    std::ostringstream out;
    out << "\"contributionCatalog\":{";
    bool firstKind = true;
    const auto beginKind = [&](const char* name) {
        if (!firstKind) out << ",";
        firstKind = false;
        out << quoted(name) << ":[";
    };
    const auto appendBase = [&](bool& first, const Package& package, const std::string& id, const std::string& name, const std::string& description) {
        if (!first) out << ",";
        first = false;
        out << "{\"packageId\":" << quoted(package.packageId)
            << ",\"itemId\":" << quoted(id)
            << ",\"name\":" << quoted(name)
            << ",\"description\":" << quoted(description)
            << ",\"packageName\":" << quoted(package.name)
            << "}";
    };
    beginKind("effectPresetPack");
    bool first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.effectPresets) appendBase(first, package, item.itemId, item.name, item.description);
    }
    out << "]";
    beginKind("soundPresetPack");
    first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.soundPresets) appendBase(first, package, item.itemId, item.name, item.description);
    }
    out << "]";
    beginKind("drumKitPresetPack");
    first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.drumKitPresets) appendBase(first, package, item.itemId, item.name, item.description);
    }
    out << "]";
    beginKind("arrangementPresetPack");
    first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.arrangementPresets) appendBase(first, package, item.itemId, item.name, item.description);
    }
    out << "]";
    beginKind("scenePresetPack");
    first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.scenePresets) appendBase(first, package, item.itemId, item.name, item.description);
    }
    out << "]";
    beginKind("samplePack");
    first = true;
    for (const auto& package : packages) {
        for (const auto& item : package.samplePacks) {
            if (!first) out << ",";
            first = false;
            out << "{\"packageId\":" << quoted(package.packageId)
                << ",\"itemId\":" << quoted(item.itemId)
                << ",\"name\":" << quoted(item.name)
                << ",\"description\":" << quoted(item.description)
                << ",\"packageName\":" << quoted(package.name)
                << ",\"filePath\":" << quoted(item.filePath)
                << ",\"format\":" << quoted(item.format)
                << ",\"sampleRate\":" << item.sampleRate
                << ",\"channels\":" << item.channels
                << ",\"duration\":" << item.duration
                << ",\"sha256\":" << quoted(item.sha256)
                << "}";
        }
    }
    out << "]}";
    return out.str();
}

std::string Library::previewJson(const Preview& preview) const {
    std::ostringstream out;
    out << "\"preview\":{";
    out << "\"valid\":" << (preview.ok ? "true" : "false")
        << ",\"warnings\":";
    appendJsonStringArray(out, preview.warnings);
    out << ",\"errors\":";
    appendJsonStringArray(out, preview.errors);
    if (preview.ok) {
        out << ",\"package\":";
        appendPackageSummary(out, preview.package, true);
    }
    out << "}";
    return out.str();
}

} // namespace ContributionPackage
