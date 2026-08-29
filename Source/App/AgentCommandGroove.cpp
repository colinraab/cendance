#include "AgentCommandGroove.h"
#include "AppState.h"
#include "CommandQueue.h"
#include "MeterQueue.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

namespace {

std::string floatToStr(float val) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << val;
    return oss.str();
}

} // namespace

namespace AgentCommand {

Response executeSwing(ExecutionContext& context, const std::vector<std::string>& tokens) {
    // Usage: swing set <0-100> | swing get
    if (tokens.size() < 2) {
        const float current = context.appState.getSwingAmount() * 100.0f;
        return {true, "swing: " + floatToStr(current) + "%", ""};
    }

    const std::string& sub = tokens[1];
    if (sub == "get") {
        const float current = context.appState.getSwingAmount() * 100.0f;
        return {true, "swing: " + floatToStr(current) + "%", ""};
    }

    if (sub == "set") {
        if (tokens.size() < 3) {
            return {false, "Usage: swing set <0-100>", ""};
        }
        float value = 0.0f;
        try {
            value = std::stof(tokens[2]);
        } catch (...) {
            return {false, "Invalid swing value: " + tokens[2], ""};
        }
        value = std::clamp(value, 0.0f, 100.0f);
        const float normalized = value / 100.0f;
        context.appState.setSwingAmount(normalized);
        return {true, "swing set to " + floatToStr(value) + "%", ""};
    }

    return {false, "Unknown swing subcommand: " + sub + ". Use: swing set <0-100> | swing get", ""};
}

Response executeHumanize(ExecutionContext& context, const std::vector<std::string>& tokens) {
    // Usage: humanize set <vel:jitter> | humanize get
    // vel and jitter are both 0-100
    if (tokens.size() < 2) {
        const float vel = context.appState.getVelocityHumanize() * 100.0f;
        const float jitter = context.appState.getTimingJitter() * 100.0f;
        return {true, "humanize velocity: " + floatToStr(vel) + "%, timing jitter: " + floatToStr(jitter) + "%", ""};
    }

    const std::string& sub = tokens[1];
    if (sub == "get") {
        const float vel = context.appState.getVelocityHumanize() * 100.0f;
        const float jitter = context.appState.getTimingJitter() * 100.0f;
        return {true, "humanize velocity: " + floatToStr(vel) + "%, timing jitter: " + floatToStr(jitter) + "%", ""};
    }

    if (sub == "set") {
        // Parse: humanize set <vel> [jitter]
        if (tokens.size() < 3) {
            return {false, "Usage: humanize set <vel 0-100> [jitter 0-100]", ""};
        }
        float velValue = 0.0f;
        float jitterValue = 0.0f;
        try {
            velValue = std::stof(tokens[2]);
        } catch (...) {
            return {false, "Invalid velocity humanize value: " + tokens[2], ""};
        }
        velValue = std::clamp(velValue, 0.0f, 100.0f);

        if (tokens.size() >= 4) {
            try {
                jitterValue = std::stof(tokens[3]);
            } catch (...) {
                return {false, "Invalid timing jitter value: " + tokens[3], ""};
            }
            jitterValue = std::clamp(jitterValue, 0.0f, 100.0f);
        } else {
            // If only vel specified, set jitter to same value
            jitterValue = velValue;
        }

        context.appState.setVelocityHumanize(velValue / 100.0f);
        context.appState.setTimingJitter(jitterValue / 100.0f);
        return {true, "humanize set: velocity " + floatToStr(velValue) + "%, timing jitter " + floatToStr(jitterValue) + "%", ""};
    }

    return {false, "Unknown humanize subcommand: " + sub + ". Use: humanize set <vel> [jitter] | humanize get", ""};
}

} // namespace AgentCommand
