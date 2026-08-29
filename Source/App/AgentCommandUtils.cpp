#include "AgentCommandUtils.h"

#include "AlgorithmCatalog.h"
#include "AlgorithmPresetRegistry.h"
#include "EffectPresetCatalog.h"
#include "PresetRegistry.h"
#include "ProjectKey.h"
#include "SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>

namespace AgentCommand {

// --- String utilities ---

std::string trimCopy(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, (end - begin) + 1);
}

std::string lowerCopy(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string jsonEscape(const std::string& text) {
    std::ostringstream out;
    for (const char ch : text) {
        switch (ch) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(ch))
                    << std::dec << std::setfill(' ');
            } else {
                out << ch;
            }
            break;
        }
    }
    return out.str();
}

std::string quoted(const std::string& text) {
    return "\"" + jsonEscape(text) + "\"";
}

// --- Response builder ---

Response makeResponse(bool ok, const std::string& message,
                      const std::string& dataJson) {
    std::ostringstream json;
    json << "{\"ok\":" << (ok ? "true" : "false")
         << ",\"message\":" << quoted(message);
    if (!dataJson.empty()) {
        json << "," << dataJson;
    }
    json << "}";
    return Response{ok, message, json.str()};
}

// --- Tokenizer & parsers ---

std::vector<std::string> tokenize(const std::string& input,
                                  std::string& error) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuote = false;
    char quoteChar = '\0';
    bool escaping = false;

    for (const char ch : input) {
        if (escaping) {
            current.push_back(ch);
            escaping = false;
            continue;
        }

        if (inQuote && ch == '\\') {
            escaping = true;
            continue;
        }

        if (inQuote) {
            if (ch == quoteChar) {
                inQuote = false;
            } else {
                current.push_back(ch);
            }
            continue;
        }

        if (ch == '"' || ch == '\'') {
            inQuote = true;
            quoteChar = ch;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (escaping) {
        current.push_back('\\');
    }
    if (inQuote) {
        error = "Unterminated quoted string.";
        return {};
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

bool parseFloat(const std::string& text, float& out) {
    try {
        size_t consumed = 0;
        const float value = std::stof(text, &consumed);
        if (consumed != text.size() || !std::isfinite(value)) {
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool parseUInt(const std::string& text, uint16_t& out) {
    if (text.empty()) {
        return false;
    }
    uint32_t value = 0;
    for (const char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        value = value * 10u + static_cast<uint32_t>(ch - '0');
        if (value > std::numeric_limits<uint16_t>::max()) {
            return false;
        }
    }
    out = static_cast<uint16_t>(value);
    return true;
}

bool parseDurationSeconds(const std::string& token, double& seconds) {
    std::string number = token;
    if (!number.empty() && (number.back() == 's' || number.back() == 'S')) {
        number.pop_back();
    }
    float parsed = 0.0f;
    if (!parseFloat(number, parsed) || parsed <= 0.0f) {
        return false;
    }
    seconds = std::clamp<double>(parsed, 0.25, 60.0);
    return true;
}

// --- Display helpers ---

const char* trackName(uint8_t track) {
    switch (track) {
    case 0: return "Drums";
    case 1: return "Bass";
    case 2: return "Chords";
    case 3: return "Lead";
    case 4: return "Master";
    default: return "Unknown";
    }
}

const char* arrangementModeName(uint8_t mode) {
    switch (mode) {
    case AppState::kArrangementModeManual: return "manual";
    case AppState::kArrangementModeAuto: return "auto";
    case AppState::kArrangementModeMixed: return "mixed";
    default: return "unknown";
    }
}

std::string commandHelpJson() {
    return "\"commands\":["
           "\"help\",\"state\",\"state full\",\"catalog algorithms\","
           "\"catalog sounds\",\"catalog effects\",\"catalog progressions\",\"meters\",\"listen 8s\","
           "\"play\",\"pause\",\"stop\",\"tempo set 128\",\"tempo +2\","
           "\"track 2 density 0.75\",\"track 4 algorithm 13\","
           "\"track 3 sound 8\",\"track 1 mute on\","
           "\"track 2 fx 1 43\",\"master fx 2 30\",\"key \\\"A minor\\\"\","
           "\"progression 4\",\"genre House\",\"genre randomize Techno\","
           "\"arrangement section 2\","
           "\"packages list\",\"packages preview /path/pkg.json\","
           "\"packages install /path/pkg.json\",\"packages catalog\","
           "\"packages apply sound pkg.id item.id\","
           "\"presets catalog\",\"presets apply effect:<source>:<id> 2 1\","
           "\"presets apply sound:<source>:<id>]\"";
}

// --- Command dispatch ---

bool dispatch(ExecutionContext& context, const Command& command,
              const std::string& description, const Command& undoCommand,
              std::string& error) {
    if (!context.dispatchCommand) {
        error = "No command dispatcher is available.";
        return false;
    }
    if (!context.dispatchCommand(command, description, undoCommand)) {
        error = "Command queue full. Try again.";
        return false;
    }
    return true;
}

bool dispatchCommandList(ExecutionContext& context,
                         const std::vector<std::pair<Command, Command>>& commands,
                         const std::string& description,
                         std::string& error) {
    for (const auto& pair : commands) {
        if (!dispatch(context, pair.first, description, pair.second, error)) {
            return false;
        }
    }
    return true;
}

// --- Listen analysis ---

static float average(const std::vector<float>& values) {
    if (values.empty()) {
        return 0.0f;
    }
    return std::accumulate(values.begin(), values.end(), 0.0f) /
           static_cast<float>(values.size());
}

static float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

ListenScores analyzeMeterHistory(const std::vector<MeterData>& history,
                                 double seconds) {
    ListenScores scores;
    if (history.empty()) {
        scores.silenceRisk = 1.0f;
        scores.notes.push_back("No meter history is available yet.");
        return scores;
    }

    const size_t desiredSamples = std::max<size_t>(1, static_cast<size_t>(std::ceil(seconds * 30.0)));
    const size_t start = history.size() > desiredSamples ? history.size() - desiredSamples : 0;

    std::vector<float> masterLevels;
    std::array<std::vector<float>, AppState::kTrackCount> trackLevels;
    std::vector<float> lows;
    std::vector<float> mids;
    std::vector<float> highs;
    uint16_t firstBar = history[start].barNumber;
    uint16_t lastBar = firstBar;
    uint32_t firstBeat = history[start].beatPosition;
    uint32_t lastBeat = firstBeat;
    size_t hotFrames = 0;
    size_t silentFrames = 0;
    size_t noteFrames = 0;

    for (size_t i = start; i < history.size(); ++i) {
        const MeterData& m = history[i];
        masterLevels.push_back(m.masterLevel);
        if (m.masterLevel > 0.92f) {
            ++hotFrames;
        }
        if (m.masterLevel < 0.03f) {
            ++silentFrames;
        }
        uint64_t noteMask = 0;
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            trackLevels[track].push_back(m.trackLevels[track]);
            noteMask |= m.activeNotes[track][0] | m.activeNotes[track][1];
        }
        if (noteMask != 0) {
            ++noteFrames;
        }

        float low = 0.0f;
        float mid = 0.0f;
        float high = 0.0f;
        for (size_t bin = 0; bin < m.spectrumBins.size(); ++bin) {
            if (bin < 8) {
                low += m.spectrumBins[bin];
            } else if (bin < 22) {
                mid += m.spectrumBins[bin];
            } else {
                high += m.spectrumBins[bin];
            }
        }
        lows.push_back(low / 8.0f);
        mids.push_back(mid / 14.0f);
        highs.push_back(high / 10.0f);
        lastBar = m.barNumber;
        lastBeat = m.beatPosition;
    }

    const float avgMaster = average(masterLevels);
    scores.energy = clamp01(avgMaster / 0.55f);
    scores.clippingRisk = clamp01(static_cast<float>(hotFrames) /
                                  static_cast<float>(masterLevels.size()));
    scores.silenceRisk = clamp01(static_cast<float>(silentFrames) /
                                 static_cast<float>(masterLevels.size()));

    std::array<float, AppState::kTrackCount> trackAverages{};
    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
        trackAverages[track] = average(trackLevels[track]);
    }
    const float trackMean = std::accumulate(trackAverages.begin(), trackAverages.end(), 0.0f) /
                            static_cast<float>(trackAverages.size());
    float imbalance = 0.0f;
    for (const float value : trackAverages) {
        imbalance += std::abs(value - trackMean);
    }
    scores.balance = clamp01(1.0f - (imbalance / 1.4f));

    const float low = average(lows);
    const float mid = average(mids);
    const float high = average(highs);
    const float spectralSum = std::max(0.0001f, low + mid + high);
    const float lowShare = low / spectralSum;
    const float midShare = mid / spectralSum;
    const float highShare = high / spectralSum;
    const float spectralDistance =
        std::abs(lowShare - 0.34f) + std::abs(midShare - 0.38f) +
        std::abs(highShare - 0.28f);
    scores.lowMidHigh = clamp01(1.0f - spectralDistance);

    float movement = 0.0f;
    for (size_t i = 1; i < masterLevels.size(); ++i) {
        movement += std::abs(masterLevels[i] - masterLevels[i - 1]);
    }
    scores.variation = clamp01((movement / std::max<size_t>(1, masterLevels.size() - 1)) * 6.0f);
    scores.variation = std::max(scores.variation,
                                clamp01(static_cast<float>(noteFrames) /
                                        static_cast<float>(masterLevels.size())));

    const int barDelta = static_cast<int>(lastBar) - static_cast<int>(firstBar);
    const int beatDelta = static_cast<int>(lastBeat) - static_cast<int>(firstBeat);
    scores.arrangementMotion = clamp01((static_cast<float>(std::max(0, barDelta)) * 4.0f +
                                        static_cast<float>(std::max(0, beatDelta))) / 16.0f);

    scores.overall = clamp01((scores.energy * 0.22f) +
                             (scores.balance * 0.18f) +
                             (scores.lowMidHigh * 0.16f) +
                             (scores.variation * 0.18f) +
                             ((1.0f - scores.clippingRisk) * 0.12f) +
                             ((1.0f - scores.silenceRisk) * 0.10f) +
                             (scores.arrangementMotion * 0.04f));

    if (scores.silenceRisk > 0.5f) {
        scores.notes.push_back("Much of the sampled window is quiet or silent.");
    }
    if (scores.clippingRisk > 0.2f) {
        scores.notes.push_back("Master level is frequently hot; check limiting or gains.");
    }
    if (scores.balance < 0.45f) {
        scores.notes.push_back("Track energy appears uneven.");
    }
    if (scores.variation < 0.25f) {
        scores.notes.push_back("The sampled window is fairly static.");
    }
    if (scores.notes.empty()) {
        scores.notes.push_back("Meter heuristics look active and reasonably balanced.");
    }

    return scores;
}

std::string listenJson(const std::vector<MeterData>& history, double seconds) {
    const ListenScores scores = analyzeMeterHistory(history, seconds);
    std::ostringstream out;
    out << "\"listen\":{"
        << "\"seconds\":" << seconds
        << ",\"energy\":" << scores.energy
        << ",\"balance\":" << scores.balance
        << ",\"lowMidHigh\":" << scores.lowMidHigh
        << ",\"variation\":" << scores.variation
        << ",\"clippingRisk\":" << scores.clippingRisk
        << ",\"silenceRisk\":" << scores.silenceRisk
        << ",\"arrangementMotion\":" << scores.arrangementMotion
        << ",\"overall\":" << scores.overall
        << ",\"notes\":[";
    for (size_t i = 0; i < scores.notes.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << quoted(scores.notes[i]);
    }
    out << "]}";
    return out.str();
}

// --- Preset registry refresh ---

void refreshPresetRegistry(ExecutionContext& context) {
    if (context.contributionLibrary != nullptr) {
        std::string ignored;
        context.contributionLibrary->reloadInstalled(ignored);
    }
    PresetRegistry::globalRegistry().rebuild(context.contributionLibrary);
    PresetRegistry::publishCustomEffectsToAudio(PresetRegistry::globalRegistry());
}

// --- Macro/FX/Arrangement command builders ---

void appendMacroCommands(const ContributionPackage::MacroDefaults& macros,
                         uint8_t track,
                         const AppState& appState,
                         std::vector<std::pair<Command, Command>>& commands) {
    const auto add = [&](Command::Type type, float value, float previous) {
        commands.push_back({Command{type, track, 0, value},
                            Command{type, track, 0, previous}});
    };
    const auto& state = appState.tracks[track];
    if (macros.density.has_value()) {
        add(Command::Type::SetDensityAbsolute, *macros.density, state.density.load(std::memory_order_relaxed));
    }
    if (macros.complexity.has_value()) {
        add(Command::Type::SetComplexityAbsolute, *macros.complexity, state.complexity.load(std::memory_order_relaxed));
    }
    if (macros.tone.has_value()) {
        add(Command::Type::SetToneAbsolute, *macros.tone, state.tone.load(std::memory_order_relaxed));
    }
    if (macros.motion.has_value()) {
        add(Command::Type::SetMotionAbsolute, *macros.motion, state.motion.load(std::memory_order_relaxed));
    }
    if (macros.gain.has_value()) {
        add(Command::Type::SetTrackGainAbsolute, *macros.gain, state.gain.load(std::memory_order_relaxed));
    }
}

void appendTrackFxCommands(const std::array<uint16_t, 3>& fxPresetIds,
                           uint8_t track,
                           const AppState& appState,
                           std::vector<std::pair<Command, Command>>& commands) {
    for (uint8_t slot = 0; slot < 3; ++slot) {
        const uint16_t preset = fxPresetIds[slot];
        if (preset == 0) {
            continue;
        }
        const uint16_t previous = appState.tracks[track].getEffectPresetSlot(slot);
        commands.push_back({
            Command{Command::Type::SetTrackEffectPreset, track, Command::encodeEffectSlotPreset(slot, preset), 0.0f},
            Command{Command::Type::SetTrackEffectPreset, track, Command::encodeEffectSlotPreset(slot, previous), 0.0f}
        });
    }
}

void appendMasterFxCommands(const std::array<uint16_t, 3>& fxPresetIds,
                            const AppState& appState,
                            std::vector<std::pair<Command, Command>>& commands) {
    for (uint8_t slot = 0; slot < 3; ++slot) {
        const uint16_t preset = fxPresetIds[slot];
        if (preset == 0) {
            continue;
        }
        const uint16_t previous = appState.master.getEffectPresetSlot(slot);
        commands.push_back({
            Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex, Command::encodeEffectSlotPreset(slot, preset), 0.0f},
            Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex, Command::encodeEffectSlotPreset(slot, previous), 0.0f}
        });
    }
}

void appendArrangementCommands(const ContributionPackage::ArrangementPresetItem& item,
                               const AppState& appState,
                               std::vector<std::pair<Command, Command>>& commands) {
    commands.push_back({
        Command{Command::Type::SetArrangementSectionCount, 0, item.sectionCount, 0.0f},
        Command{Command::Type::SetArrangementSectionCount, 0,
                appState.arrangementSectionCount.load(std::memory_order_relaxed), 0.0f}
    });
    commands.push_back({
        Command{Command::Type::SetArrangementMode, 0, item.mode, 0.0f},
        Command{Command::Type::SetArrangementMode, 0,
                appState.arrangementMode.load(std::memory_order_relaxed), 0.0f}
    });
    commands.push_back({
        Command{Command::Type::SetArrangementSection, 0, item.currentSection, 0.0f},
        Command{Command::Type::SetArrangementSection, 0,
                appState.arrangementCurrentSection.load(std::memory_order_relaxed), 0.0f}
    });
    commands.push_back({
        Command{Command::Type::SetArrangementChainEnabled, 0, static_cast<uint16_t>(item.chainEnabled ? 1 : 0), 0.0f},
        Command{Command::Type::SetArrangementChainEnabled, 0,
                static_cast<uint16_t>(appState.arrangementChainEnabled.load(std::memory_order_relaxed) ? 1 : 0), 0.0f}
    });
    commands.push_back({
        Command{Command::Type::SetArrangementChainLength, 0, item.chainLength, 0.0f},
        Command{Command::Type::SetArrangementChainLength, 0, appState.getArrangementChainLength(), 0.0f}
    });
    for (uint8_t section = 0; section < AppState::kArrangementMaxSections; ++section) {
        commands.push_back({
            Command{Command::Type::SetArrangementSectionLength, 0,
                    Command::encodeArrangementSectionValue(section, item.sectionLengths[section]), 0.0f},
            Command{Command::Type::SetArrangementSectionLength, 0,
                    Command::encodeArrangementSectionValue(section, appState.getArrangementSectionLength(section)), 0.0f}
        });
        commands.push_back({
            Command{Command::Type::SetArrangementSectionProgression, 0,
                    Command::encodeArrangementSectionValue(section, item.sectionProgressions[section]), 0.0f},
            Command{Command::Type::SetArrangementSectionProgression, 0,
                    Command::encodeArrangementSectionValue(section, appState.getArrangementSectionProgression(section)), 0.0f}
        });
        commands.push_back({
            Command{Command::Type::SetArrangementSectionTrackMask, 0,
                    Command::encodeArrangementSectionValue(section, item.trackMasks[section]), 0.0f},
            Command{Command::Type::SetArrangementSectionTrackMask, 0,
                    Command::encodeArrangementSectionValue(section, appState.getArrangementSectionTrackMask(section)), 0.0f}
        });
        commands.push_back({
            Command{Command::Type::SetArrangementChainStep, 0,
                    Command::encodeArrangementSectionValue(section, item.chainSequence[section]), 0.0f},
            Command{Command::Type::SetArrangementChainStep, 0,
                    Command::encodeArrangementSectionValue(section, appState.getArrangementChainStep(section)), 0.0f}
        });
    }
}

} // namespace AgentCommand
