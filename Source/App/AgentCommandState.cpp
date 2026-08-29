#include "AgentCommandState.h"

#include "AlgorithmCatalog.h"
#include "AlgorithmPresetRegistry.h"
#include "EffectPresetCatalog.h"
#include "PresetRegistry.h"
#include "ProjectKey.h"
#include "SynthCatalog.h"
#include "../Audio/Harmony/ChordProgression.h"

#include <sstream>

namespace AgentCommand {
namespace {

std::string stateJson(const AppState& state, bool full) {
    std::ostringstream out;
    out << "\"state\":{";
    out << "\"playing\":" << (state.playing.load(std::memory_order_relaxed) ? "true" : "false")
        << ",\"bpm\":" << state.bpm.load(std::memory_order_relaxed)
        << ",\"metronome\":" << (state.metronomeEnabled.load(std::memory_order_relaxed) ? "true" : "false")
        << ",\"key\":" << quoted(ProjectKey::format(
               state.projectKeyRoot.load(std::memory_order_relaxed),
               state.projectKeyMode.load(std::memory_order_relaxed)))
        << ",\"progression\":{\"id\":" << static_cast<int>(state.chordProgression.load(std::memory_order_relaxed)) + 1
        << ",\"name\":" << quoted(std::string(ChordProgression::getNameByDisplayId(
               static_cast<uint16_t>(state.chordProgression.load(std::memory_order_relaxed)) + 1)))
        << ",\"genre_tags\":" << ChordProgression::getGenreMask(state.chordProgression.load(std::memory_order_relaxed)) << "}"
        << ",\"arrangement\":{\"mode\":" << quoted(arrangementModeName(state.arrangementMode.load(std::memory_order_relaxed)))
        << ",\"section\":" << static_cast<int>(state.arrangementCurrentSection.load(std::memory_order_relaxed)) + 1
        << ",\"sectionCount\":" << static_cast<int>(state.arrangementSectionCount.load(std::memory_order_relaxed))
        << "}";

    out << ",\"tracks\":[";
    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
        if (track > 0) {
            out << ",";
        }
        const auto& t = state.tracks[track];
        const uint8_t algorithm = t.algorithmId.load(std::memory_order_relaxed);
        const uint8_t sound = t.synthPreset.load(std::memory_order_relaxed);
        out << "{\"id\":" << static_cast<int>(track) + 1
            << ",\"name\":" << quoted(trackName(track))
            << ",\"algorithm\":{\"id\":" << static_cast<int>(algorithm) + 1
            << ",\"name\":" << quoted(std::string(AlgorithmCatalog::getAlgorithmName(track, algorithm))) << "}"
            << ",\"sound\":{\"id\":" << static_cast<int>(sound) + 1
            << ",\"name\":" << quoted(std::string(SynthCatalog::getPresetName(track, sound))) << "}"
            << ",\"density\":" << t.density.load(std::memory_order_relaxed)
            << ",\"complexity\":" << t.complexity.load(std::memory_order_relaxed)
            << ",\"tone\":" << t.tone.load(std::memory_order_relaxed)
            << ",\"motion\":" << t.motion.load(std::memory_order_relaxed)
            << ",\"gain\":" << t.gain.load(std::memory_order_relaxed)
            << ",\"muted\":" << (t.muted.load(std::memory_order_relaxed) ? "true" : "false");
        if (full) {
            out << ",\"fx\":[";
            for (uint8_t slot = 0; slot < 3; ++slot) {
                if (slot > 0) {
                    out << ",";
                }
                const uint16_t preset = t.getEffectPresetSlot(slot);
                out << "{\"slot\":" << static_cast<int>(slot) + 1
                    << ",\"id\":" << EffectPresetCatalog::presetIdToDisplayId(preset)
                    << ",\"name\":" << quoted(std::string(EffectPresetCatalog::getPresetName(preset))) << "}";
            }
            out << "]";
        }
        out << "}";
    }
    out << "]";

    if (full) {
        out << ",\"master\":{\"fx\":[";
        for (uint8_t slot = 0; slot < 3; ++slot) {
            if (slot > 0) {
                out << ",";
            }
            const uint16_t preset = state.master.getEffectPresetSlot(slot);
            out << "{\"slot\":" << static_cast<int>(slot) + 1
                << ",\"id\":" << EffectPresetCatalog::presetIdToDisplayId(preset)
                << ",\"name\":" << quoted(std::string(EffectPresetCatalog::getPresetName(preset))) << "}";
        }
        out << "]}";
    }

    out << "}";
    return out.str();
}

std::string catalogAlgorithmsJson() {
    std::ostringstream out;
    out << "\"catalog\":{\"algorithms\":[";
    for (uint8_t track = 0; track < AlgorithmCatalog::kTrackCount; ++track) {
        if (track > 0) {
            out << ",";
        }
        out << "{\"track\":" << static_cast<int>(track) + 1
            << ",\"name\":" << quoted(trackName(track))
            << ",\"items\":[";
        const uint16_t count = AlgorithmCatalog::getAlgorithmCountForTrack(track);
        for (uint16_t i = 0; i < count; ++i) {
            if (i > 0) {
                out << ",";
            }
            out << "{\"id\":" << i + 1
                << ",\"name\":" << quoted(std::string(AlgorithmCatalog::getAlgorithmName(track, i))) << "}";
        }

        // Append custom algorithms
        auto& registry = globalAlgorithmPresetRegistry();
        auto customPresets = registry.listForTrack(track);
        for (const auto& preset : customPresets) {
            if (!preset.id.empty()) {
                auto runtimeId = registry.runtimeIdForPresetId(track, preset.id);
                uint16_t rid = runtimeId.value_or(0);
                out << ",{\"id\":" << rid
                    << ",\"runtime_algorithm_id\":" << rid
                    << ",\"name\":" << quoted(preset.name)
                    << ",\"builtin\":false"
                    << ",\"preset_ref\":" << quoted("local.algorithms/" + preset.id + "/" + preset.version)
                    << ",\"author\":" << quoted(preset.author)
                    << ",\"genre_tags\":" << preset.genreTags
                    << "}";
            }
        }

        out << "]}";
    }
    out << "]}";
    return out.str();
}

std::string catalogSoundsJson() {
    std::ostringstream out;
    out << "\"catalog\":{\"sounds\":[";
    for (uint8_t track = 0; track < SynthCatalog::kTrackCount; ++track) {
        if (track > 0) {
            out << ",";
        }
        out << "{\"track\":" << static_cast<int>(track) + 1
            << ",\"name\":" << quoted(trackName(track))
            << ",\"items\":[";
        const uint16_t count = SynthCatalog::getPresetCountForTrack(track);
        for (uint16_t i = 0; i < count; ++i) {
            if (i > 0) {
                out << ",";
            }
            out << "{\"id\":" << i + 1
                << ",\"name\":" << quoted(std::string(SynthCatalog::getPresetName(track, static_cast<uint8_t>(i))))
                << ",\"genre_tags\":" << SynthCatalog::getSoundGenreMask(track, static_cast<uint8_t>(i))
                << "}";
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}

std::string catalogEffectsJson() {
    std::ostringstream out;
    out << "\"catalog\":{\"effects\":[";
    for (uint16_t i = 0; i < EffectPresetCatalog::getTotalPresetCount(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << "{\"id\":" << i + 1
            << ",\"name\":" << quoted(std::string(EffectPresetCatalog::getPresetNameByDisplayId(static_cast<uint16_t>(i + 1)))) << "}";
    }
    out << "]}";
    return out.str();
}

std::string catalogProgressionsJson() {
    std::ostringstream out;
    out << "\"catalog\":{\"progressions\":[";
    const int count = ChordProgression::getNumProgressions();
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            out << ",";
        }
        const auto& progression = ChordProgression::get(i);
        out << "{\"id\":" << i + 1
            << ",\"name\":" << quoted(progression.name)
            << ",\"genre_tags\":" << progression.genreTags
            << ",\"degrees\":[";
        for (size_t degreeIndex = 0; degreeIndex < progression.degrees.size(); ++degreeIndex) {
            if (degreeIndex > 0) {
                out << ",";
            }
            out << progression.degrees[degreeIndex] + 1;
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}

std::string metersJson(const MeterData& meter) {
    std::ostringstream out;
    out << "\"meters\":{"
        << "\"playing\":" << (meter.isPlaying ? "true" : "false")
        << ",\"bar\":" << meter.barNumber + 1
        << ",\"beat\":" << meter.beatPosition + 1
        << ",\"master\":" << meter.masterLevel
        << ",\"tracks\":[";
    for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
        if (track > 0) {
            out << ",";
        }
        out << "{\"track\":" << static_cast<int>(track) + 1
            << ",\"level\":" << meter.trackLevels[track]
            << ",\"algorithm\":" << static_cast<int>(meter.activeAlgorithm[track]) + 1
            << "}";
    }
    out << "],\"spectrum\":[";
    for (size_t i = 0; i < meter.spectrumBins.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << meter.spectrumBins[i];
    }
    out << "]}";
    return out.str();
}

} // namespace

Response executeHelp() {
    return makeResponse(true, "Available agent commands.", commandHelpJson());
}

Response executeState(ExecutionContext& context, bool full) {
    return makeResponse(true, "State snapshot.", stateJson(context.appState, full));
}

Response executeCatalog(ExecutionContext& context, const std::string& domain) {
    if (domain == "algorithms") {
        return makeResponse(true, "Algorithm catalog.", catalogAlgorithmsJson());
    }
    if (domain == "sounds") {
        return makeResponse(true, "Sound catalog.", catalogSoundsJson());
    }
    if (domain == "effects") {
        return makeResponse(true, "Effect catalog.", catalogEffectsJson());
    }
    if (domain == "progressions") {
        return makeResponse(true, "Progression catalog.", catalogProgressionsJson());
    }
    if (domain == "presets") {
        refreshPresetRegistry(context);
        return makeResponse(true, "Merged preset catalog.", PresetRegistry::globalRegistry().catalogJson());
    }
    return makeResponse(false, "Unknown catalog domain.");
}

Response executeMeters(ExecutionContext& context) {
    return makeResponse(true, "Current meters.", metersJson(context.currentMeters));
}

Response executeListen(ExecutionContext& context, double seconds) {
    return makeResponse(true, "Listening heuristic snapshot.", listenJson(context.meterHistory, seconds));
}

} // namespace AgentCommand
