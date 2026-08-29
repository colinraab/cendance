#include "AgentCommandTransport.h"

namespace AgentCommand {
namespace {

Response executePlayPause(ExecutionContext& context, bool targetPlaying) {
    const bool current = context.appState.playing.load(std::memory_order_relaxed);
    if (current == targetPlaying) {
        return makeResponse(true, targetPlaying ? "Already playing." : "Already paused.");
    }

    std::string error;
    const Command command{Command::Type::PlayStop, 0, 0, 0.0f};
    const Command undo{Command::Type::PlayStop, 0, 0, 0.0f};
    if (!dispatch(context, command, targetPlaying ? "Agent Play" : "Agent Pause", undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, targetPlaying ? "Playback started." : "Playback paused.");
}

} // namespace

Response executePlay(ExecutionContext& context) {
    return executePlayPause(context, true);
}

Response executePause(ExecutionContext& context) {
    return executePlayPause(context, false);
}

Response executeStop(ExecutionContext& context) {
    std::string error;
    const uint8_t previousSection =
        context.appState.arrangementCurrentSection.load(std::memory_order_relaxed);
    const Command command{Command::Type::Stop, 0, 0, 0.0f};
    const Command undo{Command::Type::SetArrangementSection, 0, previousSection, 0.0f};
    if (!dispatch(context, command, "Agent Stop", undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Playback stopped.");
}

Response executeTempo(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() != 2 && tokens.size() != 3) {
        return makeResponse(false, "Usage: tempo set 128 or tempo +2.");
    }

    float value = 0.0f;
    Command command{Command::Type::SetTempo, 0, 0, 0.0f};
    std::string description;
    if (lowerCopy(tokens[1]) == "set") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: tempo set 128.");
        }
        if (!parseFloat(tokens[2], value)) {
            return makeResponse(false, "Tempo must be numeric.");
        }
        if (value < 20.0f || value > 260.0f) {
            return makeResponse(false, "Tempo must be between 20 and 260 BPM.");
        }
        command = Command{Command::Type::SetTempoAbsolute, 0, 0, value};
        description = "Agent Tempo Set " + std::to_string(static_cast<int>(value));
    } else {
        if (tokens.size() != 2) {
            return makeResponse(false, "Usage: tempo +2.");
        }
        if (!parseFloat(tokens[1], value)) {
            return makeResponse(false, "Tempo delta must be numeric.");
        }
        command = Command{Command::Type::SetTempo, 0, 0, value};
        description = "Agent Tempo " + tokens[1];
    }

    std::string error;
    const float previous = context.appState.bpm.load(std::memory_order_relaxed);
    const Command undo{Command::Type::SetTempoAbsolute, 0, 0, previous};
    if (!dispatch(context, command, description, undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Tempo command accepted.");
}

} // namespace AgentCommand
