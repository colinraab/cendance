#include "AgentCommandTrack.h"

#include "AlgorithmCatalog.h"
#include "EffectPresetCatalog.h"
#include "SynthCatalog.h"

namespace AgentCommand {
namespace {

bool parseTrackNumber(const std::string& token, uint8_t& trackIndex) {
    uint16_t display = 0;
    if (!parseUInt(token, display) || display < 1 || display > AppState::kTrackCount) {
        return false;
    }
    trackIndex = static_cast<uint8_t>(display - 1);
    return true;
}

} // namespace

Response executeTrack(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        return makeResponse(false, "Usage: track <1-4> <parameter> <value>.");
    }

    uint8_t track = 0;
    if (!parseTrackNumber(tokens[1], track)) {
        return makeResponse(false, "Track must be 1-4.");
    }

    const std::string field = lowerCopy(tokens[2]);
    std::string error;

    if (field == "density" || field == "complexity" || field == "tone" ||
        field == "motion" || field == "gain") {
        if (tokens.size() != 4) {
            return makeResponse(false, "Track parameter command takes one value.");
        }
        float value = 0.0f;
        if (!parseFloat(tokens[3], value)) {
            return makeResponse(false, "Track parameter value must be numeric.");
        }

        Command::Type type = Command::Type::SetDensityAbsolute;
        float previous = 0.0f;
        float maxValue = 1.0f;
        if (field == "density") {
            previous = context.appState.tracks[track].density.load(std::memory_order_relaxed);
            type = Command::Type::SetDensityAbsolute;
        } else if (field == "complexity") {
            previous = context.appState.tracks[track].complexity.load(std::memory_order_relaxed);
            type = Command::Type::SetComplexityAbsolute;
        } else if (field == "tone") {
            previous = context.appState.tracks[track].tone.load(std::memory_order_relaxed);
            type = Command::Type::SetToneAbsolute;
        } else if (field == "motion") {
            previous = context.appState.tracks[track].motion.load(std::memory_order_relaxed);
            type = Command::Type::SetMotionAbsolute;
        } else {
            previous = context.appState.tracks[track].gain.load(std::memory_order_relaxed);
            type = Command::Type::SetTrackGainAbsolute;
            maxValue = 2.0f;
        }

        if (value < 0.0f || value > maxValue) {
            return makeResponse(false, "Track parameter value is out of range.");
        }

        const Command command{type, track, 0, value};
        const Command undo{type, track, 0, previous};
        if (!dispatch(context, command, "Agent Track " + std::to_string(track + 1) + " " + field, undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Track parameter command accepted.");
    }

    if (field == "algorithm") {
        if (tokens.size() != 4) {
            return makeResponse(false, "Usage: track <1-4> algorithm <id>.");
        }
        uint16_t displayId = 0;
        if (!parseUInt(tokens[3], displayId) ||
            !AlgorithmCatalog::isValidDisplayIdForTrack(track, displayId)) {
            return makeResponse(false, "Algorithm id is out of range for this track.");
        }
        const uint8_t previous = context.appState.tracks[track].algorithmId.load(std::memory_order_relaxed);
        const Command command{Command::Type::SetAlgorithm, track,
                              AlgorithmCatalog::displayIdToAlgorithmId(displayId), 0.0f};
        const Command undo{Command::Type::SetAlgorithm, track, previous, 0.0f};
        if (!dispatch(context, command, "Agent Track " + std::to_string(track + 1) + " Algorithm", undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Algorithm command accepted.");
    }

    if (field == "sound") {
        if (tokens.size() != 4) {
            return makeResponse(false, "Usage: track <1-4> sound <id>.");
        }
        uint16_t displayId = 0;
        if (!parseUInt(tokens[3], displayId) ||
            !SynthCatalog::isValidDisplayIdForTrack(track, displayId)) {
            return makeResponse(false, "Sound id is out of range for this track.");
        }
        const uint8_t previous = context.appState.tracks[track].synthPreset.load(std::memory_order_relaxed);
        const Command command{Command::Type::SetSynthPreset, track,
                              SynthCatalog::displayIdToPresetId(displayId), 0.0f};
        const Command undo{Command::Type::SetSynthPreset, track, previous, 0.0f};
        if (!dispatch(context, command, "Agent Track " + std::to_string(track + 1) + " Sound", undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Sound command accepted.");
    }

    if (field == "mute") {
        if (tokens.size() != 4) {
            return makeResponse(false, "Usage: track <1-4> mute on|off.");
        }
        const bool target = lowerCopy(tokens[3]) == "on" || lowerCopy(tokens[3]) == "true" || tokens[3] == "1";
        const bool targetOff = lowerCopy(tokens[3]) == "off" || lowerCopy(tokens[3]) == "false" || tokens[3] == "0";
        if (!target && !targetOff) {
            return makeResponse(false, "Mute value must be on or off.");
        }
        const bool current = context.appState.tracks[track].muted.load(std::memory_order_relaxed);
        if (current == target) {
            return makeResponse(true, "Mute state already matches.");
        }
        const Command command{Command::Type::ToggleTrackMute, track, 0, 0.0f};
        const Command undo{Command::Type::ToggleTrackMute, track, 0, 0.0f};
        if (!dispatch(context, command, "Agent Track " + std::to_string(track + 1) + " Mute", undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Mute command accepted.");
    }

    if (field == "fx") {
        if (tokens.size() != 5) {
            return makeResponse(false, "Usage: track <1-4> fx <slot 1-3> <effect id>.");
        }
        uint16_t slotDisplay = 0;
        uint16_t effectDisplay = 0;
        if (!parseUInt(tokens[3], slotDisplay) || slotDisplay < 1 || slotDisplay > 3) {
            return makeResponse(false, "FX slot must be 1-3.");
        }
        if (!parseUInt(tokens[4], effectDisplay) ||
            !EffectPresetCatalog::isValidDisplayId(effectDisplay)) {
            return makeResponse(false, "Effect id is out of range.");
        }
        const uint8_t slot = static_cast<uint8_t>(slotDisplay - 1);
        const uint16_t preset = EffectPresetCatalog::displayIdToPresetId(effectDisplay);
        const uint16_t previous = context.appState.tracks[track].getEffectPresetSlot(slot);
        const Command command{Command::Type::SetTrackEffectPreset, track,
                              Command::encodeEffectSlotPreset(slot, preset), 0.0f};
        const Command undo{Command::Type::SetTrackEffectPreset, track,
                           Command::encodeEffectSlotPreset(slot, previous), 0.0f};
        if (!dispatch(context, command, "Agent Track " + std::to_string(track + 1) + " FX", undo, error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Track FX command accepted.");
    }

    return makeResponse(false, "Unknown track parameter.");
}

} // namespace AgentCommand
