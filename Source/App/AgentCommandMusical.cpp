#include "AgentCommandMusical.h"

#include "ArrangementPresetManager.h"
#include "GenreCatalog.h"
#include "ProjectIO.h"
#include "ProjectIOLoad.h"
#include "ProjectKey.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <cctype>

namespace AgentCommand {
namespace {

std::string joinTokens(const std::vector<std::string>& tokens, size_t begin) {
    std::string text;
    for (size_t i = begin; i < tokens.size(); ++i) {
        if (!text.empty()) {
            text += " ";
        }
        text += tokens[i];
    }
    return text;
}

std::string normalizeGenreName(std::string text) {
    text = lowerCopy(text);
    std::string normalized;
    for (const char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            normalized.push_back(ch);
        }
    }
    return normalized;
}

bool parseGenreSelection(const std::string& text, bool allowNone, uint8_t& genreId) {
    uint16_t numeric = 0;
    if (parseUInt(text, numeric)) {
        if (numeric == 0 && allowNone) {
            genreId = 0;
            return true;
        }
        if (numeric >= 1 && numeric <= GenreCatalog::kGenreCount) {
            genreId = static_cast<uint8_t>(numeric);
            return true;
        }
        return false;
    }

    const std::string normalized = normalizeGenreName(text);
    if (allowNone && (normalized == "none" || normalized == "random" || normalized == "free")) {
        genreId = 0;
        return true;
    }

    for (uint8_t i = 0; i < GenreCatalog::kGenreCount; ++i) {
        if (normalizeGenreName(std::string(GenreCatalog::getGenreName(i))) == normalized) {
            genreId = static_cast<uint8_t>(i + 1);
            return true;
        }
    }

    return false;
}

} // namespace

Response executeKey(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        return makeResponse(false, "Usage: key \"A minor\".");
    }
    std::string keyText = tokens[1];
    for (size_t i = 2; i < tokens.size(); ++i) {
        keyText += " " + tokens[i];
    }
    ProjectKey::ParsedValue parsed;
    if (!ProjectKey::parse(keyText, parsed)) {
        return makeResponse(false, "Invalid key format.");
    }

    const uint8_t previousRoot = context.appState.projectKeyRoot.load(std::memory_order_relaxed);
    const uint8_t previousMode = context.appState.projectKeyMode.load(std::memory_order_relaxed);
    const Command command{Command::Type::SetProjectKey, 0,
                          Command::encodeProjectKey(parsed.root, parsed.mode), 0.0f};
    const Command undo{Command::Type::SetProjectKey, 0,
                       Command::encodeProjectKey(previousRoot, previousMode), 0.0f};
    std::string error;
    if (!dispatch(context, command, "Agent Key " + ProjectKey::format(parsed.root, parsed.mode), undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Key command accepted.");
}

Response executeProgression(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        return makeResponse(false, "Usage: progression <id>.");
    }
    uint16_t displayId = 0;
    if (!parseUInt(tokens[1], displayId) || !ChordProgression::isValidDisplayId(displayId)) {
        return makeResponse(false, "Progression id is out of range.");
    }
    const uint8_t previous = context.appState.chordProgression.load(std::memory_order_relaxed);
    const Command command{Command::Type::SetChordProg, 0,
                          static_cast<uint16_t>(ChordProgression::displayIdToProgressionIndex(displayId)), 0.0f};
    const Command undo{Command::Type::SetChordProg, 0, previous, 0.0f};
    std::string error;
    if (!dispatch(context, command, "Agent Progression", undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Progression command accepted.");
}

Response executeGenre(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        return makeResponse(false, "Usage: genre <id|name|none> or genre randomize <id|name|none>.");
    }

    const bool randomize = lowerCopy(tokens[1]) == "randomize";
    const size_t genreTokenStart = randomize ? 2u : 1u;
    if (tokens.size() <= genreTokenStart) {
        return makeResponse(false, randomize
            ? "Usage: genre randomize <id|name|none>."
            : "Usage: genre <id|name|none>.");
    }

    uint8_t genreId = 0;
    if (!parseGenreSelection(joinTokens(tokens, genreTokenStart), true, genreId)) {
        return makeResponse(false, randomize
            ? "Genre must be 0-8, none, or a known genre name."
            : "Genre must be 0-8, none, or a known genre name.");
    }

    const uint8_t previous = context.appState.getGenre();
    const Command::Type type = randomize
        ? Command::Type::RandomizeForGenre
        : Command::Type::SetGenre;
    const Command command{type, 0, genreId, 0.0f};
    const Command undo{Command::Type::SetGenre, 0, previous, 0.0f};
    std::string error;
    if (!dispatch(context, command, randomize ? "Agent Genre Randomize" : "Agent Genre", undo, error)) {
        return makeResponse(false, error);
    }

    return makeResponse(true, randomize ? "Genre randomize command accepted." : "Genre command accepted.");
}

Response executeArrangement(ExecutionContext& context, const std::vector<std::string>& tokens) {
    // Usage:
    //   arrangement section <id>
    //   arrangement save <name>
    //   arrangement load <preset_id>
    //   arrangement list
    //   arrangement delete <preset_id>
    if (tokens.size() < 2) {
        return makeResponse(false, "Usage: arrangement section <id> | save <name> | load <id> | list | delete <id>.");
    }

    const std::string sub = lowerCopy(tokens[1]);

    if (sub == "section") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: arrangement section <id>.");
        }
        uint16_t sectionDisplay = 0;
        const uint8_t sectionCount = context.appState.arrangementSectionCount.load(std::memory_order_relaxed);
        if (!parseUInt(tokens[2], sectionDisplay) || sectionDisplay < 1 || sectionDisplay > sectionCount) {
            return makeResponse(false, "Arrangement section is out of range.");
        }
        const uint8_t previous = context.appState.arrangementCurrentSection.load(std::memory_order_relaxed);
        const Command command{Command::Type::SetArrangementSection, 0,
                              static_cast<uint16_t>(sectionDisplay - 1), 0.0f};
        const Command undo{Command::Type::SetArrangementSection, 0, previous, 0.0f};
        std::string error;
        if (!dispatch(context, command, "Agent Arrangement Section", undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Arrangement section command accepted.");
    }

    if (sub == "save") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: arrangement save <name>.");
        }
        std::string name = tokens[2];
        for (size_t i = 3; i < tokens.size(); ++i) {
            name += " " + tokens[i];
        }
        auto& mgr = globalArrangementPresetManager();
        std::string error;
        std::string presetId = mgr.savePreset(context.appState, name, error);
        if (presetId.empty()) {
            return makeResponse(false, "Failed to save arrangement preset: " + error);
        }
        return makeResponse(true, "Arrangement preset saved: " + presetId);
    }

    if (sub == "load") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: arrangement load <preset_id>.");
        }
        const std::string& presetId = tokens[2];
        auto& mgr = globalArrangementPresetManager();
        std::string error;

        // Reload to make sure we have the latest
        mgr.reload(error);

        const auto* preset = mgr.findPreset(presetId);
        if (preset == nullptr) {
            return makeResponse(false, "Arrangement preset not found: " + presetId);
        }

        // Apply via command queue using the existing arrangement command builders
        std::vector<std::pair<Command, Command>> commands;
        appendArrangementCommands(*preset, context.appState, commands);
        if (!dispatchCommandList(context, commands, "Agent Arrangement Load " + preset->name, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Arrangement preset loaded: " + preset->name);
    }

    if (sub == "list") {
        auto& mgr = globalArrangementPresetManager();
        std::string error;
        mgr.reload(error);
        return makeResponse(true, "Saved arrangement presets.", mgr.presetsJson());
    }

    if (sub == "delete") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: arrangement delete <preset_id>.");
        }
        const std::string& presetId = tokens[2];
        auto& mgr = globalArrangementPresetManager();
        std::string error;
        if (!mgr.deletePreset(presetId, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Arrangement preset deleted: " + presetId);
    }

    return makeResponse(false, "Unknown arrangement subcommand: " + sub + ". Use: section | save | load | list | delete.");
}

Response executeProject(ExecutionContext& context, const std::vector<std::string>& tokens) {
    // Usage:
    //   project save <name>
    //   project load <path>
    //   project list
    //   project delete <path>
    if (tokens.size() < 2) {
        return makeResponse(false, "Usage: project save <name> | load <path> | list | delete <path>.");
    }

    const std::string sub = lowerCopy(tokens[1]);

    if (sub == "save") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: project save <name>.");
        }
        std::string name = tokens[2];
        for (size_t i = 3; i < tokens.size(); ++i) {
            name += " " + tokens[i];
        }

        // Snapshot and save
        ProjectIO::ProjectSnapshot snapshot = ProjectIO::snapshotFromState(context.appState);
        snapshot.projectName = name;

        // Build path in the default projects directory
        std::string dir = ProjectIO::getDefaultProjectsDirectory();
        std::string safeName;
        for (const unsigned char ch : name) {
            safeName.push_back((std::isalnum(ch) || ch == '-' || ch == '_') ? static_cast<char>(ch) : '_');
        }
        if (safeName.empty()) safeName = "project";
        std::string path = dir + "/" + safeName + ".cendance";

        std::string error;
        if (!ProjectIO::saveProjectFile(snapshot, path, error)) {
            return makeResponse(false, "Failed to save project: " + error);
        }
        return makeResponse(true, "Project saved: " + path);
    }

    if (sub == "load") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: project load <path>.");
        }
        const std::string& path = tokens[2];

        ProjectIO::ProjectSnapshot snapshot;
        std::string error;
        if (!ProjectIO::loadProjectFile(path, snapshot, error)) {
            return makeResponse(false, "Failed to load project: " + error);
        }

        if (!context.commandQueue) {
            return makeResponse(false, "Command queue not available — cannot apply project.");
        }

        if (!ProjectIO::applySnapshotToCommandQueue(snapshot, context.appState, *context.commandQueue, error)) {
            return makeResponse(false, "Failed to apply project: " + error);
        }
        return makeResponse(true, "Project loaded: " + path);
    }

    if (sub == "list") {
        std::string dir = ProjectIO::getDefaultProjectsDirectory();

        // Scan the default directory
        juce::File dirFile(dir);
        juce::Array<juce::File> files;
        if (dirFile.exists() && dirFile.isDirectory()) {
            files = dirFile.findChildFiles(juce::File::findFiles, false, "*.cendance");
        }

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
        return makeResponse(true, "Saved projects.", juce::JSON::toString(juce::var(res), false).toStdString());
    }

    if (sub == "delete") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: project delete <path>.");
        }
        const std::string& path = tokens[2];
        juce::File file(path);
        if (!file.existsAsFile()) {
            return makeResponse(false, "Project file not found: " + path);
        }
        if (!file.deleteFile()) {
            return makeResponse(false, "Failed to delete project file: " + path);
        }
        return makeResponse(true, "Project deleted: " + path);
    }

    return makeResponse(false, "Unknown project subcommand: " + sub + ". Use: save | load | list | delete.");
}

} // namespace AgentCommand
