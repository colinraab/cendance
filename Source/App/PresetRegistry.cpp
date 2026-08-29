#include "PresetRegistry.h"

#include "DrumKitPresetCatalog.h"
#include "SynthCatalog.h"

#include <algorithm>
#include <mutex>
#include <sstream>

namespace PresetRegistry {
namespace {

std::mutex gCustomEffectsMutex;
std::vector<ResolvedEffectPreset> gPublishedCustomEffects;

std::string quoted(const std::string& text) {
    std::ostringstream out;
    out << "\"";
    for (const char ch : text) {
        if (ch == '"' || ch == '\\') {
            out << '\\';
        }
        out << ch;
    }
    out << "\"";
    return out.str();
}

void appendTags(std::ostringstream& out, const std::vector<std::string>& tags) {
    out << "[";
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) out << ",";
        out << quoted(tags[i]);
    }
    out << "]";
}

void appendEntry(std::ostringstream& out, const CatalogEntry& entry) {
    out << "{\"ref\":" << PresetRefs::toJson(entry.ref)
        << ",\"displayLabel\":" << quoted(entry.displayLabel)
        << ",\"name\":" << quoted(entry.name)
        << ",\"description\":" << quoted(entry.description)
        << ",\"source\":" << quoted(PresetRefs::sourceToString(entry.ref.source))
        << ",\"track\":" << static_cast<int>(entry.trackIndex) + 1
        << ",\"runtimeId\":" << entry.runtimeId;
    if (!entry.ref.packageId.empty()) {
        out << ",\"packageId\":" << quoted(entry.ref.packageId)
            << ",\"itemId\":" << quoted(entry.ref.itemId)
            << ",\"packageName\":" << quoted(entry.packageName);
    }
    bool hasFx = (entry.fxPresetIds[0] != 0 || entry.fxPresetIds[1] != 0 || entry.fxPresetIds[2] != 0);
    if (hasFx) {
        out << ",\"fx\":[";
        for (size_t i = 0; i < entry.fxPresetIds.size(); ++i) {
            if (i > 0) out << ",";
            out << entry.fxPresetIds[i];
        }
        out << "]";
    }
    out << ",\"tags\":";
    appendTags(out, entry.tags);
    out << "}";
}

void appendEntries(std::ostringstream& out, const char* key, const std::vector<CatalogEntry>& entries) {
    out << "\"" << key << "\":[";
    for (size_t i = 0; i < entries.size(); ++i) {
        if (i > 0) out << ",";
        appendEntry(out, entries[i]);
    }
    out << "]";
}

} // namespace

PresetRefs::PresetRef builtinEffectRef(uint16_t presetId) {
    return PresetRefs::builtin(PresetRefs::Domain::Effect,
                               "effect-" + std::to_string(EffectPresetCatalog::presetIdToDisplayId(presetId))
                                   + "-" + PresetRefs::slugify(std::string(EffectPresetCatalog::getPresetName(presetId))));
}

PresetRefs::PresetRef builtinSoundRef(uint8_t trackIndex, uint8_t presetId) {
    return PresetRefs::builtin(PresetRefs::Domain::Sound,
                               "track-" + std::to_string(trackIndex + 1)
                                   + "-sound-" + std::to_string(presetId + 1)
                                   + "-" + PresetRefs::slugify(std::string(SynthCatalog::getPresetName(trackIndex, presetId))));
}

PresetRefs::PresetRef builtinDrumKitRef(uint8_t presetId) {
    return PresetRefs::builtin(PresetRefs::Domain::DrumKit,
                               "drumkit-" + std::to_string(presetId + 1)
                                   + "-" + PresetRefs::slugify(std::string(DrumKitPresetCatalog::getPreset(presetId).name)));
}

std::optional<uint16_t> builtinEffectIdFromSlug(const std::string& slug) {
    for (uint16_t id = 0; id < EffectPresetCatalog::getTotalPresetCount(); ++id) {
        const uint16_t presetId = EffectPresetCatalog::displayIdToPresetId(static_cast<uint16_t>(id + 1));
        if (builtinEffectRef(presetId).builtinId == slug) {
            return presetId;
        }
    }
    return std::nullopt;
}

std::optional<std::pair<uint8_t, uint8_t>> builtinSoundIdFromSlug(const std::string& slug) {
    for (uint8_t track = 0; track < SynthCatalog::kTrackCount; ++track) {
        const uint16_t count = SynthCatalog::getPresetCountForTrack(track);
        for (uint16_t id = 0; id < count; ++id) {
            if (builtinSoundRef(track, static_cast<uint8_t>(id)).builtinId == slug) {
                return std::make_pair(track, static_cast<uint8_t>(id));
            }
        }
    }
    return std::nullopt;
}

void Registry::rebuild(const ContributionPackage::Library* contributionLibrary) {
    library = contributionLibrary;
    effectEntries.clear();
    soundEntries.clear();
    drumKitEntries.clear();
    arrangementEntries.clear();
    sceneEntries.clear();

    for (uint16_t display = 1; display <= EffectPresetCatalog::getTotalPresetCount(); ++display) {
        const uint16_t presetId = EffectPresetCatalog::displayIdToPresetId(display);
        effectEntries.push_back({builtinEffectRef(presetId),
                                 std::to_string(display),
                                 std::string(EffectPresetCatalog::getPresetName(presetId)),
                                 "Built-in effect preset.",
                                 0,
                                 presetId,
                                 {},
                                 {"builtin"}});
    }

    for (uint8_t track = 0; track < SynthCatalog::kTrackCount; ++track) {
        const uint16_t count = SynthCatalog::getPresetCountForTrack(track);
        for (uint16_t id = 0; id < count; ++id) {
            soundEntries.push_back({builtinSoundRef(track, static_cast<uint8_t>(id)),
                                    std::to_string(id + 1),
                                    std::string(SynthCatalog::getPresetName(track, static_cast<uint8_t>(id))),
                                    "Built-in sound preset.",
                                    track,
                                    id,
                                    {},
                                    {"builtin"}});
        }
    }

    for (uint8_t id = 0; id < DrumKitPresetCatalog::getPresetCount(); ++id) {
        auto preset = DrumKitPresetCatalog::getPreset(id);
        auto fxSlots = DrumKitPresetCatalog::getPresetEffectSlots(id);
        std::array<uint16_t, 3> fxDisplayIds{{0, 0, 0}};
        for (uint8_t s = 0; s < 3; ++s) {
            if (fxSlots[s] != 0)
                fxDisplayIds[s] = EffectPresetCatalog::presetIdToDisplayId(fxSlots[s]);
        }
        drumKitEntries.push_back({builtinDrumKitRef(id),
                                  std::to_string(id + 1),
                                  std::string(preset.name),
                                  "Built-in drum kit preset.",
                                  0,
                                  id,
                                  {},
                                  {"builtin"},
                                  fxDisplayIds});
    }

    if (library == nullptr) {
        return;
    }

    uint16_t nextCustomEffectId = kCustomEffectPresetIdBase;
    for (const auto& package : library->installedPackages()) {
        for (const auto& item : package.effectPresets) {
            effectEntries.push_back({PresetRefs::package(PresetRefs::Domain::Effect, package.packageId, item.itemId, package.version),
                                     "C" + std::to_string(nextCustomEffectId - kCustomEffectPresetIdBase + 1),
                                     item.name,
                                     item.description,
                                     0,
                                     nextCustomEffectId++,
                                     package.name,
                                     item.tags});
        }
        for (const auto& item : package.soundPresets) {
            soundEntries.push_back({PresetRefs::package(PresetRefs::Domain::Sound, package.packageId, item.itemId, package.version),
                                    "C" + std::to_string(soundEntries.size() + 1),
                                    item.name,
                                    item.description,
                                    item.trackIndex,
                                    item.synthPresetId,
                                    package.name,
                                    item.tags});
        }
        for (const auto& item : package.drumKitPresets) {
            std::array<uint16_t, 3> fxDisplayIds{{0, 0, 0}};
            for (uint8_t s = 0; s < 3; ++s) {
                if (item.fxPresetIds[s] != 0)
                    fxDisplayIds[s] = EffectPresetCatalog::presetIdToDisplayId(item.fxPresetIds[s]);
            }
            drumKitEntries.push_back({PresetRefs::package(PresetRefs::Domain::DrumKit, package.packageId, item.itemId, package.version),
                                      "C" + std::to_string(drumKitEntries.size() + 1),
                                      item.name,
                                      item.description,
                                      0,
                                      0,
                                      package.name,
                                      item.tags,
                                      fxDisplayIds});
        }
        for (const auto& item : package.arrangementPresets) {
            arrangementEntries.push_back({PresetRefs::package(PresetRefs::Domain::Arrangement, package.packageId, item.itemId, package.version),
                                          "C" + std::to_string(arrangementEntries.size() + 1),
                                          item.name,
                                          item.description,
                                          0,
                                          0,
                                          package.name,
                                          item.tags});
        }
        for (const auto& item : package.scenePresets) {
            sceneEntries.push_back({PresetRefs::package(PresetRefs::Domain::Scene, package.packageId, item.itemId, package.version),
                                    "C" + std::to_string(sceneEntries.size() + 1),
                                    item.name,
                                    item.description,
                                    0,
                                    0,
                                    package.name,
                                    item.tags});
        }
    }
}

std::vector<CatalogEntry> Registry::soundsForTrack(uint8_t trackIndex) const {
    std::vector<CatalogEntry> result;
    for (const auto& entry : soundEntries) {
        if (entry.trackIndex == trackIndex) {
            result.push_back(entry);
        }
    }
    return result;
}

std::optional<CatalogEntry> Registry::findEntry(const PresetRefs::PresetRef& ref) const {
    const auto findIn = [&](const std::vector<CatalogEntry>& entries) -> std::optional<CatalogEntry> {
        for (const auto& entry : entries) {
            if (entry.ref == ref) {
                return entry;
            }
        }
        return std::nullopt;
    };
    if (auto found = findIn(effectEntries)) return found;
    if (auto found = findIn(soundEntries)) return found;
    if (auto found = findIn(drumKitEntries)) return found;
    if (auto found = findIn(arrangementEntries)) return found;
    return findIn(sceneEntries);
}

std::optional<ResolvedEffectPreset> Registry::resolveEffect(const PresetRefs::PresetRef& ref) const {
    if (ref.domain != PresetRefs::Domain::Effect) {
        return std::nullopt;
    }
    if (ref.source == PresetRefs::Source::Builtin) {
        const auto presetId = builtinEffectIdFromSlug(ref.builtinId);
        if (!presetId.has_value()) {
            return std::nullopt;
        }
        const auto preset = EffectPresetCatalog::getPresetById(*presetId);
        return ResolvedEffectPreset{ref, std::string(preset.name), preset.type, preset.paramA, preset.paramB, preset.paramC, *presetId};
    }
    if (library == nullptr) {
        return std::nullopt;
    }
    const auto* item = library->findEffectPreset(ref.packageId, ref.itemId);
    if (item == nullptr) {
        return std::nullopt;
    }
    const auto runtimeId = runtimeEffectIdForRef(ref);
    return ResolvedEffectPreset{ref, item->name, item->type, item->paramA, item->paramB, item->paramC, runtimeId.value_or(0)};
}

std::optional<ResolvedSoundPreset> Registry::resolveSound(const PresetRefs::PresetRef& ref) const {
    if (ref.domain != PresetRefs::Domain::Sound) {
        return std::nullopt;
    }
    if (ref.source == PresetRefs::Source::Builtin) {
        const auto ids = builtinSoundIdFromSlug(ref.builtinId);
        if (!ids.has_value()) {
            return std::nullopt;
        }
        const uint8_t track = ids->first;
        const uint8_t preset = ids->second;
        return ResolvedSoundPreset{ref,
                                   std::string(SynthCatalog::getPresetName(track, preset)),
                                   track,
                                   preset,
                                   SynthCatalog::getPresetEffectSlots(track, preset),
                                   {}};
    }
    if (library == nullptr) {
        return std::nullopt;
    }
    const auto* item = library->findSoundPreset(ref.packageId, ref.itemId);
    if (item == nullptr) {
        return std::nullopt;
    }
    uint8_t synthPresetId = item->synthPresetId;
    if (item->soundRef.has_value()) {
        if (const auto nested = resolveSound(*item->soundRef)) {
            synthPresetId = nested->synthPresetId;
        }
    }
    auto fxPresetIds = item->fxPresetIds;
    for (uint8_t slot = 0; slot < fxPresetIds.size(); ++slot) {
        if (item->fxPresetRefs[slot].has_value()) {
            if (const auto runtimeId = runtimeEffectIdForRef(*item->fxPresetRefs[slot])) {
                fxPresetIds[slot] = *runtimeId;
            }
        }
    }
    return ResolvedSoundPreset{ref, item->name, item->trackIndex, synthPresetId, fxPresetIds, item->macros};
}

std::optional<uint16_t> Registry::runtimeEffectIdForRef(const PresetRefs::PresetRef& ref) const {
    for (const auto& entry : effectEntries) {
        if (entry.ref == ref) {
            return entry.runtimeId;
        }
    }
    return std::nullopt;
}

std::optional<PresetRefs::PresetRef> Registry::refForRuntimeEffectId(uint16_t presetId) const {
    for (const auto& entry : effectEntries) {
        if (entry.runtimeId == presetId) {
            return entry.ref;
        }
    }
    return std::nullopt;
}

std::string Registry::catalogJson() const {
    std::ostringstream out;
    out << "\"presetCatalog\":{";
    appendEntries(out, "effects", effectEntries);
    out << ",";
    appendEntries(out, "sounds", soundEntries);
    out << ",";
    appendEntries(out, "drumKits", drumKitEntries);
    out << ",";
    appendEntries(out, "arrangements", arrangementEntries);
    out << ",";
    appendEntries(out, "scenes", sceneEntries);
    out << "}";
    return out.str();
}

Registry& globalRegistry() {
    static Registry registry;
    return registry;
}

void publishCustomEffectsToAudio(const Registry& registry) {
    std::vector<ResolvedEffectPreset> next;
    for (const auto& entry : registry.effects()) {
        if (entry.ref.source != PresetRefs::Source::Package) {
            continue;
        }
        if (auto resolved = registry.resolveEffect(entry.ref)) {
            next.push_back(*resolved);
        }
    }
    std::lock_guard<std::mutex> lock(gCustomEffectsMutex);
    gPublishedCustomEffects = std::move(next);
}

std::optional<ResolvedEffectPreset> resolvePublishedCustomEffect(uint16_t presetId) {
    std::lock_guard<std::mutex> lock(gCustomEffectsMutex);
    for (const auto& effect : gPublishedCustomEffects) {
        if (effect.runtimePresetId == presetId) {
            return effect;
        }
    }
    return std::nullopt;
}

} // namespace PresetRegistry
