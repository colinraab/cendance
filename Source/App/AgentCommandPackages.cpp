#include "AgentCommandPackages.h"

#include "PresetRegistry.h"
#include "PresetRef.h"

namespace AgentCommand {

Response executePresets(ExecutionContext& context, const std::vector<std::string>& tokens) {
    refreshPresetRegistry(context);
    auto& registry = PresetRegistry::globalRegistry();
    if (tokens.size() == 2 && lowerCopy(tokens[1]) == "catalog") {
        return makeResponse(true, "Merged preset catalog.", registry.catalogJson());
    }
    if (tokens.size() < 3 || lowerCopy(tokens[1]) != "apply") {
        return makeResponse(false, "Usage: presets catalog or presets apply <ref> [track 1-5] [slot 1-3].");
    }
    const auto ref = PresetRefs::parseStableString(tokens[2]);
    if (!ref.has_value()) {
        return makeResponse(false, "Preset ref is invalid.");
    }
    std::vector<std::pair<Command, Command>> commands;
    if (ref->domain == PresetRefs::Domain::Sound) {
        const auto sound = registry.resolveSound(*ref);
        if (!sound.has_value()) {
            return makeResponse(false, "Sound preset ref could not be resolved.");
        }
        const uint8_t track = sound->trackIndex;
        commands.push_back({Command{Command::Type::SetSynthPreset, track, sound->synthPresetId, 0.0f},
                            Command{Command::Type::SetSynthPreset, track,
                                    context.appState.tracks[track].synthPreset.load(std::memory_order_relaxed), 0.0f}});
        appendTrackFxCommands(sound->fxPresetIds, track, context.appState, commands);
        appendMacroCommands(sound->macros, track, context.appState, commands);
    } else if (ref->domain == PresetRefs::Domain::Effect) {
        if (tokens.size() != 5) {
            return makeResponse(false, "Usage: presets apply <effect ref> <track 1-5> <slot 1-3>.");
        }
        uint16_t trackDisplay = 0;
        uint16_t slotDisplay = 0;
        if (!parseUInt(tokens[3], trackDisplay) || !parseUInt(tokens[4], slotDisplay)
            || trackDisplay < 1 || trackDisplay > 5 || slotDisplay < 1 || slotDisplay > 3) {
            return makeResponse(false, "Effect target must be track 1-5 and slot 1-3.");
        }
        const auto runtimePresetId = registry.runtimeEffectIdForRef(*ref);
        if (!runtimePresetId.has_value()) {
            return makeResponse(false, "Effect preset ref could not be resolved.");
        }
        const uint8_t slot = static_cast<uint8_t>(slotDisplay - 1);
        if (trackDisplay == 5) {
            const uint16_t previous = context.appState.master.getEffectPresetSlot(slot);
            commands.push_back({Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                                        Command::encodeEffectSlotPreset(slot, *runtimePresetId), 0.0f},
                                Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                                        Command::encodeEffectSlotPreset(slot, previous), 0.0f}});
        } else {
            const uint8_t track = static_cast<uint8_t>(trackDisplay - 1);
            const uint16_t previous = context.appState.tracks[track].getEffectPresetSlot(slot);
            commands.push_back({Command{Command::Type::SetTrackEffectPreset, track,
                                        Command::encodeEffectSlotPreset(slot, *runtimePresetId), 0.0f},
                                Command{Command::Type::SetTrackEffectPreset, track,
                                        Command::encodeEffectSlotPreset(slot, previous), 0.0f}});
        }
    } else if (ref->domain == PresetRefs::Domain::DrumKit) {
        if (ref->source == PresetRefs::Source::Builtin) {
            std::optional<uint8_t> foundPresetId;
            for (uint8_t id = 0; id < DrumKitPresetCatalog::getPresetCount(); ++id) {
                if (PresetRegistry::builtinDrumKitRef(id).builtinId == ref->builtinId) {
                    foundPresetId = id;
                    break;
                }
            }
            if (!foundPresetId.has_value()) {
                return makeResponse(false, "Built-in drum kit preset ref could not be resolved.");
            }
            const uint8_t presetId = *foundPresetId;
            const auto& preset = DrumKitPresetCatalog::getPreset(presetId);
            const auto& state = context.appState.tracks[0];
            for (uint8_t slot = 0; slot < preset.slots.size(); ++slot) {
                const auto& drumSlot = preset.slots[slot];
                commands.push_back({
                    Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(slot, drumSlot.sampleId), 0.0f},
                    Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(slot, state.getDrumSampleSlotSampleId(slot)), 0.0f}
                });
                commands.push_back({Command{Command::Type::SetDrumSampleVolume, 0, slot, drumSlot.volume},
                                    Command{Command::Type::SetDrumSampleVolume, 0, slot, state.getDrumSampleSlotVolume(slot)}});
                commands.push_back({Command{Command::Type::SetDrumSampleTune, 0, slot, drumSlot.tuneSemitones},
                                    Command{Command::Type::SetDrumSampleTune, 0, slot, state.getDrumSampleSlotTuneSemitones(slot)}});
                commands.push_back({Command{Command::Type::SetDrumSampleStartOffset, 0, slot, drumSlot.startOffset},
                                    Command{Command::Type::SetDrumSampleStartOffset, 0, slot, state.getDrumSampleSlotStartOffset(slot)}});
                commands.push_back({Command{Command::Type::SetDrumSampleDecay, 0, slot, drumSlot.decay},
                                    Command{Command::Type::SetDrumSampleDecay, 0, slot, state.getDrumSampleSlotDecay(slot)}});
                commands.push_back({Command{Command::Type::SetDrumSampleVelocitySensitivity, 0, slot, drumSlot.velocitySensitivity},
                                    Command{Command::Type::SetDrumSampleVelocitySensitivity, 0, slot, state.getDrumSampleSlotVelocitySensitivity(slot)}});
            }
            auto fxSlots = DrumKitPresetCatalog::getPresetEffectSlots(presetId);
            appendTrackFxCommands(fxSlots, 0, context.appState, commands);
        } else {
            return makeResponse(false, "Package drum kit refs must be applied via packages apply drumkit.");
        }
    } else {
        return makeResponse(false, "This preset ref domain is cataloged but not directly applyable yet.");
    }
    std::string error;
    if (!dispatchCommandList(context, commands, "Agent PresetRef Apply", error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Preset ref applied.");
}

namespace {

Response applyContribution(const std::vector<std::string>& tokens,
                           ExecutionContext& context) {
    if (context.contributionLibrary == nullptr) {
        return makeResponse(false, "Contribution package library is unavailable.");
    }
    if (tokens.size() != 5 && tokens.size() != 7) {
        return makeResponse(false, "Usage: packages apply sound|effect|drumkit|arrangement|scene <packageId> <itemId>.");
    }
    const std::string kind = lowerCopy(tokens[2]);
    const std::string& packageId = tokens[3];
    const std::string& itemId = tokens[4];
    std::vector<std::pair<Command, Command>> commands;
    std::string description = "Agent Contribution";

    if (kind == "sound") {
        const auto* item = context.contributionLibrary->findSoundPreset(packageId, itemId);
        if (item == nullptr) {
            return makeResponse(false, "Sound contribution not found.");
        }
        const uint8_t track = item->trackIndex;
        commands.push_back({
            Command{Command::Type::SetSynthPreset, track, item->synthPresetId, 0.0f},
            Command{Command::Type::SetSynthPreset, track,
                    context.appState.tracks[track].synthPreset.load(std::memory_order_relaxed), 0.0f}
        });
        appendTrackFxCommands(item->fxPresetIds, track, context.appState, commands);
        appendMacroCommands(item->macros, track, context.appState, commands);
        description = "Agent Sound Contribution " + item->name;
    } else if (kind == "effect") {
        const auto* item = context.contributionLibrary->findEffectPreset(packageId, itemId);
        if (item == nullptr) {
            return makeResponse(false, "Effect contribution not found.");
        }
        refreshPresetRegistry(context);
        const auto ref = PresetRefs::package(PresetRefs::Domain::Effect, packageId, itemId);
        const auto runtimePresetId = PresetRegistry::globalRegistry().runtimeEffectIdForRef(ref);
        if (!runtimePresetId.has_value()) {
            return makeResponse(false, "Effect contribution could not be assigned a runtime preset id.");
        }
        uint16_t trackDisplay = 0;
        uint16_t slotDisplay = 0;
        if (tokens.size() != 7 || !parseUInt(tokens[5], trackDisplay) || !parseUInt(tokens[6], slotDisplay)
            || trackDisplay < 1 || trackDisplay > 5 || slotDisplay < 1 || slotDisplay > 3) {
            return makeResponse(false, "Usage for effects: packages apply effect <packageId> <itemId> <track 1-5> <slot 1-3>.");
        }
        const uint8_t slot = static_cast<uint8_t>(slotDisplay - 1);
        if (trackDisplay == 5) {
            const uint16_t previous = context.appState.master.getEffectPresetSlot(slot);
            commands.push_back({
                Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                        Command::encodeEffectSlotPreset(slot, *runtimePresetId), 0.0f},
                Command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                        Command::encodeEffectSlotPreset(slot, previous), 0.0f}
            });
        } else {
            const uint8_t track = static_cast<uint8_t>(trackDisplay - 1);
            const uint16_t previous = context.appState.tracks[track].getEffectPresetSlot(slot);
            commands.push_back({
                Command{Command::Type::SetTrackEffectPreset, track,
                        Command::encodeEffectSlotPreset(slot, *runtimePresetId), 0.0f},
                Command{Command::Type::SetTrackEffectPreset, track,
                        Command::encodeEffectSlotPreset(slot, previous), 0.0f}
            });
        }
        description = "Agent Effect Contribution " + item->name;
    } else if (kind == "drumkit") {
        const auto* item = context.contributionLibrary->findDrumKitPreset(packageId, itemId);
        if (item == nullptr) {
            return makeResponse(false, "Drum kit contribution not found.");
        }
        for (uint8_t slot = 0; slot < item->slots.size(); ++slot) {
            const auto& drumSlot = item->slots[slot];
            const auto& state = context.appState.tracks[0];
            commands.push_back({
                Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(slot, drumSlot.sampleId), 0.0f},
                Command{Command::Type::SetDrumSampleAssignment, 0, Command::encodeDrumSlotSampleId(slot, state.getDrumSampleSlotSampleId(slot)), 0.0f}
            });
            commands.push_back({Command{Command::Type::SetDrumSampleVolume, 0, slot, drumSlot.volume},
                                Command{Command::Type::SetDrumSampleVolume, 0, slot, state.getDrumSampleSlotVolume(slot)}});
            commands.push_back({Command{Command::Type::SetDrumSampleTune, 0, slot, drumSlot.tuneSemitones},
                                Command{Command::Type::SetDrumSampleTune, 0, slot, state.getDrumSampleSlotTuneSemitones(slot)}});
            commands.push_back({Command{Command::Type::SetDrumSampleStartOffset, 0, slot, drumSlot.startOffset},
                                Command{Command::Type::SetDrumSampleStartOffset, 0, slot, state.getDrumSampleSlotStartOffset(slot)}});
            commands.push_back({Command{Command::Type::SetDrumSampleDecay, 0, slot, drumSlot.decay},
                                Command{Command::Type::SetDrumSampleDecay, 0, slot, state.getDrumSampleSlotDecay(slot)}});
            commands.push_back({Command{Command::Type::SetDrumSampleVelocitySensitivity, 0, slot, drumSlot.velocitySensitivity},
                                Command{Command::Type::SetDrumSampleVelocitySensitivity, 0, slot, state.getDrumSampleSlotVelocitySensitivity(slot)}});
        }
        {
            auto fxPresetIds = item->fxPresetIds;
            for (uint8_t s = 0; s < 3; ++s) {
                if (item->fxPresetRefs[s].has_value()) {
                    refreshPresetRegistry(context);
                    const auto runtimeId = PresetRegistry::globalRegistry().runtimeEffectIdForRef(*item->fxPresetRefs[s]);
                    if (runtimeId.has_value()) {
                        fxPresetIds[s] = *runtimeId;
                    }
                }
            }
            appendTrackFxCommands(fxPresetIds, 0, context.appState, commands);
        }
        description = "Agent Drum Kit Contribution " + item->name;
    } else if (kind == "arrangement") {
        const auto* item = context.contributionLibrary->findArrangementPreset(packageId, itemId);
        if (item == nullptr) {
            return makeResponse(false, "Arrangement contribution not found.");
        }
        appendArrangementCommands(*item, context.appState, commands);
        description = "Agent Arrangement Contribution " + item->name;
    } else if (kind == "scene") {
        const auto* item = context.contributionLibrary->findScenePreset(packageId, itemId);
        if (item == nullptr) {
            return makeResponse(false, "Scene contribution not found.");
        }
        if (item->bpm.has_value()) {
            commands.push_back({Command{Command::Type::SetTempoAbsolute, 0, 0, *item->bpm},
                                Command{Command::Type::SetTempoAbsolute, 0, 0,
                                        context.appState.bpm.load(std::memory_order_relaxed)}});
        }
        if (item->chordProgression.has_value()) {
            commands.push_back({Command{Command::Type::SetChordProg, 0, *item->chordProgression, 0.0f},
                                Command{Command::Type::SetChordProg, 0,
                                        context.appState.chordProgression.load(std::memory_order_relaxed), 0.0f}});
        }
        if (item->projectKeyRoot.has_value() && item->projectKeyMode.has_value()) {
            commands.push_back({Command{Command::Type::SetProjectKey, 0,
                                        Command::encodeProjectKey(*item->projectKeyRoot, *item->projectKeyMode), 0.0f},
                                Command{Command::Type::SetProjectKey, 0,
                                        Command::encodeProjectKey(context.appState.projectKeyRoot.load(std::memory_order_relaxed),
                                                                  context.appState.projectKeyMode.load(std::memory_order_relaxed)), 0.0f}});
        }
        for (uint8_t track = 0; track < AppState::kTrackCount; ++track) {
            if (!item->hasTrack[track]) {
                continue;
            }
            const auto& trackItem = item->tracks[track];
            commands.push_back({Command{Command::Type::SetAlgorithm, track, trackItem.algorithmId, 0.0f},
                                Command{Command::Type::SetAlgorithm, track,
                                        context.appState.tracks[track].algorithmId.load(std::memory_order_relaxed), 0.0f}});
            commands.push_back({Command{Command::Type::SetSynthPreset, track, trackItem.synthPresetId, 0.0f},
                                Command{Command::Type::SetSynthPreset, track,
                                        context.appState.tracks[track].synthPreset.load(std::memory_order_relaxed), 0.0f}});
            appendTrackFxCommands(trackItem.fxPresetIds, track, context.appState, commands);
            appendMacroCommands(trackItem.macros, track, context.appState, commands);
            if (trackItem.hasMuted && context.appState.tracks[track].muted.load(std::memory_order_relaxed) != trackItem.muted) {
                commands.push_back({Command{Command::Type::ToggleTrackMute, track, 0, 0.0f},
                                    Command{Command::Type::ToggleTrackMute, track, 0, 0.0f}});
            }
        }
        if (item->hasMasterFx) {
            appendMasterFxCommands(item->masterFxPresetIds, context.appState, commands);
        }
        if (item->masterGain.has_value()) {
            commands.push_back({Command{Command::Type::SetTrackGainAbsolute, kMasterTrackIndex, 0, *item->masterGain},
                                Command{Command::Type::SetTrackGainAbsolute, kMasterTrackIndex, 0,
                                        context.appState.master.gain.load(std::memory_order_relaxed)}});
        }
        if (item->arrangement.has_value()) {
            appendArrangementCommands(*item->arrangement, context.appState, commands);
        }
        description = "Agent Scene Contribution " + item->name;
    } else {
        return makeResponse(false, "Unknown contribution kind.");
    }

    std::string error;
    if (!dispatchCommandList(context, commands, description, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Contribution applied.");
}

} // namespace

Response executePackages(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (context.contributionLibrary == nullptr) {
        return makeResponse(false, "Contribution package library is unavailable.");
    }
    if (tokens.size() < 2) {
        return makeResponse(false, "Usage: packages list|preview|install|remove|catalog|export|apply.");
    }
    const std::string action = lowerCopy(tokens[1]);
    if (action == "list") {
        if (tokens.size() != 2) {
            return makeResponse(false, "Usage: packages list.");
        }
        std::string error;
        if (!context.contributionLibrary->reloadInstalled(error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Installed contribution packages.", context.contributionLibrary->packagesJson(true));
    }
    if (action == "catalog") {
        if (tokens.size() != 2) {
            return makeResponse(false, "Usage: packages catalog.");
        }
        std::string error;
        if (!context.contributionLibrary->reloadInstalled(error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Contribution catalog.", context.contributionLibrary->contributionCatalogJson());
    }
    if (action == "preview") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: packages preview <path>.");
        }
        const auto preview = context.contributionLibrary->previewFile(tokens[2]);
        return makeResponse(preview.ok, preview.ok ? "Package preview passed." : "Package preview failed.",
                            context.contributionLibrary->previewJson(preview));
    }
    if (action == "install") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: packages install <path>.");
        }
        ContributionPackage::Preview preview;
        std::string error;
        if (!context.contributionLibrary->installFile(tokens[2], preview, error)) {
            return makeResponse(false, error, context.contributionLibrary->previewJson(preview));
        }
        return makeResponse(true, "Package installed.", context.contributionLibrary->previewJson(preview));
    }
    if (action == "remove") {
        if (tokens.size() != 3) {
            return makeResponse(false, "Usage: packages remove <packageId>.");
        }
        std::string error;
        if (!context.contributionLibrary->removePackage(tokens[2], error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Package removed.");
    }
    if (action == "export") {
        if (tokens.size() != 6) {
            return makeResponse(false, "Usage: packages export <path> <kind> <packageId> <name>.");
        }
        const auto kind = ContributionPackage::kindFromString(tokens[3]);
        if (!kind.has_value()) {
            return makeResponse(false, "Unsupported package kind.");
        }
        std::string error;
        if (!context.contributionLibrary->exportPackageTemplate(tokens[2], *kind, tokens[4], tokens[5], error)) {
            return makeResponse(false, error);
        }
        return makeResponse(true, "Package template exported.");
    }
    if (action == "apply") {
        return applyContribution(tokens, context);
    }
    return makeResponse(false, "Unknown packages action.");
}

} // namespace AgentCommand
