#include "../Source/App/AppState.h"
#include "../Source/App/AlgorithmCatalog.h"
#include "../Source/App/EffectPresetCatalog.h"
#include "../Source/App/ProjectKey.h"
#include "../Source/App/StartupProjectInitializer.h"
#include "../Source/App/SynthCatalog.h"
#include "../Source/App/SpotEffectCatalog.h"
#include "../Source/Audio/Transport.h"
#include "../Source/Audio/Effects/Spot/BeatRepeat.h"
#include "../Source/Audio/Generators/BassStyleAlgorithms.h"
#include "../Source/Audio/Generators/ChordStyleAlgorithms.h"
#include "../Source/Audio/Generators/DrumStyleAlgorithms.h"
#include "../Source/Audio/Generators/LeadStyleAlgorithms.h"
#include "../Source/Audio/Generators/BlockChords.h"
#include "../Source/Audio/Generators/CustomAlgorithmInstance.h"
#include "../Source/Audio/Generators/SyncBass.h"
#include "../Source/Audio/Generators/SyncStabs.h"
#include "../Source/App/CustomAlgorithmPreset.h"
#include "../Source/Audio/Harmony/Scale.h"
#include "../Source/Audio/Harmony/ChordProgression.h"
#include "../Source/Audio/Effects/6_Degrade/ReduxCrush.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

namespace {

void testAppStateDefaultsAndSetters() {
    AppState state;

    assert(state.bpm.load() == 120.0f);
    assert(state.playing.load() == false);
    assert(state.metronomeEnabled.load() == false);
    assert(state.chordProgression.load() == 0);
    assert(state.projectKeyRoot.load() == 0);
    assert(state.projectKeyMode.load() == AppState::kProjectKeyModeNaturalMinor);
    assert(state.arrangementSectionCount.load() == 4);
    assert(state.arrangementCurrentSection.load() == 0);
    assert(state.arrangementMode.load() == AppState::kArrangementModeMixed);
    assert(state.arrangementChainEnabled.load() == false);
    assert(state.getArrangementChainLength() == AppState::kArrangementDefaultChainLength);
    assert(state.activeSpotEffects.load() == 0);

    for (uint8_t section = 0; section < AppState::kArrangementMaxSections; ++section) {
        assert(state.getArrangementSectionLength(section) == AppState::kArrangementDefaultSectionLengthBars);
        assert(state.getArrangementSectionProgression(section) == AppState::kArrangementProgressionFollowGlobal);
        assert(state.getArrangementSectionTrackMask(section) == AppState::kArrangementTrackMaskAll);
        assert(state.getArrangementChainStep(section) == section);
    }

    for (int i = 0; i < 4; ++i) {
        assert(state.tracks[i].algorithmId.load() == 0);
        assert(state.tracks[i].synthPreset.load() == 0);
        assert(state.tracks[i].density.load() == 0.5f);
        assert(state.tracks[i].complexity.load() == 0.5f);
        assert(state.tracks[i].tone.load() == 0.5f);
        assert(state.tracks[i].motion.load() == 0.5f);
        assert(state.tracks[i].muted.load() == false);
        assert(state.tracks[i].synthManualOverride.load() == false);
        assert(state.tracks[i].gain.load() == 1.0f);
    }

    for (uint8_t slot = 0; slot < 3; ++slot) {
        assert(state.master.getEffectPresetSlot(slot) == 0);
    }
    assert(state.master.gain.load() == 2.0f);

    state.setBpm(132.0f);
    state.setPlaying(true);
    state.setMetronomeEnabled(false);
    state.setChordProgression(3);
    state.setProjectKey(10, AppState::kProjectKeyModeMajor);
    state.setArrangementSectionCount(3);
    state.setArrangementCurrentSection(2);
    state.setArrangementMode(AppState::kArrangementModeAuto);
    state.setArrangementSectionLength(1, 7);
    state.setArrangementSectionProgression(1, 5);
    state.setArrangementSectionTrackMask(1, 0b0110);
    state.setArrangementChainEnabled(true);
    state.setArrangementChainLength(4);
    state.setArrangementChainStep(0, 0);
    state.setArrangementChainStep(1, 2);
    state.setArrangementChainStep(2, 1);
    state.setArrangementChainStep(3, 2);
    state.setActiveSpotEffects(0b0101);
    state.tracks[2].setAlgorithmId(4);
    state.tracks[2].setSynthPreset(2);
    state.tracks[2].setDensity(0.75f);
    state.tracks[2].setComplexity(0.25f);
    state.tracks[2].setTone(0.9f);
    state.tracks[2].setMotion(0.1f);
    state.tracks[2].setMuted(true);
    state.tracks[2].setSynthManualOverride(true);
    state.tracks[2].setGain(0.6f);
    state.master.setEffectPresetSlot(0, 5);
    state.master.setEffectPresetSlot(1, 9);
    state.master.setEffectPresetSlot(2, 12);

    assert(state.bpm.load() == 132.0f);
    assert(state.playing.load() == true);
    assert(state.metronomeEnabled.load() == false);
    assert(state.chordProgression.load() == 3);
    assert(state.projectKeyRoot.load() == 10);
    assert(state.projectKeyMode.load() == AppState::kProjectKeyModeMajor);
    assert(state.arrangementSectionCount.load() == 3);
    assert(state.arrangementCurrentSection.load() == 2);
    assert(state.arrangementMode.load() == AppState::kArrangementModeAuto);
    assert(state.getArrangementSectionLength(1) == 7);
    assert(state.getArrangementSectionProgression(1) == 5);
    assert(state.getArrangementSectionTrackMask(1) == 0b0110);
    assert(state.arrangementChainEnabled.load() == true);
    assert(state.getArrangementChainLength() == 4);
    assert(state.getArrangementChainStep(0) == 0);
    assert(state.getArrangementChainStep(1) == 2);
    assert(state.getArrangementChainStep(2) == 1);
    assert(state.getArrangementChainStep(3) == 2);
    assert(state.activeSpotEffects.load() == 0b0101);
    assert(state.tracks[2].algorithmId.load() == 4);
    assert(state.tracks[2].synthPreset.load() == 2);
    assert(state.tracks[2].density.load() == 0.75f);
    assert(state.tracks[2].complexity.load() == 0.25f);
    assert(state.tracks[2].tone.load() == 0.9f);
    assert(state.tracks[2].motion.load() == 0.1f);
    assert(state.tracks[2].muted.load() == true);
    assert(state.tracks[2].synthManualOverride.load() == true);
    assert(state.tracks[2].gain.load() == 0.6f);
    assert(state.master.getEffectPresetSlot(0) == 5);
    assert(state.master.getEffectPresetSlot(1) == 9);
    assert(state.master.getEffectPresetSlot(2) == 12);
}

void testSpotStutterDefaultsAndWrappedRepeat() {
    assert(SpotEffectCatalog::getStutterDefaultRepeatDivision() == 0.5f);
    assert(SpotEffectCatalog::getStutterDefaultMix() == 1.0f);

    BeatRepeat repeat;
    repeat.prepare(1000.0, 64);
    repeat.setBpm(120.0f);
    repeat.setRepeatDivision(SpotEffectCatalog::getStutterDefaultRepeatDivision());
    repeat.setMix(SpotEffectCatalog::getStutterDefaultMix());

    juce::AudioBuffer<float> fill(2, 4050);
    for (int s = 0; s < fill.getNumSamples(); ++s) {
        fill.setSample(0, s, static_cast<float>(s));
        fill.setSample(1, s, static_cast<float>(s));
    }
    repeat.processBlock(fill, fill.getNumSamples());

    repeat.setActive(true);
    juce::AudioBuffer<float> stuttered(2, 260);
    stuttered.clear();
    repeat.processBlock(stuttered, stuttered.getNumSamples());

    assert(stuttered.getSample(0, 0) == 0.0f);
    assert(stuttered.getSample(0, 5) == 3805.0f);
    assert(stuttered.getSample(0, 199) == 3999.0f);
    assert(stuttered.getSample(0, 200) == 4000.0f);
    assert(stuttered.getSample(1, 200) == 4000.0f);
}

void testTransportBeatBarTransitions() {
    Transport transport;
    transport.prepare(48000.0);
    transport.setBpm(120.0f);

    constexpr int samplesPerBeatAt120 = 24000;
    transport.advance(samplesPerBeatAt120);
    assert(std::fabs(transport.getPlayheadPosition() - 1.0) < 1e-6);
    assert(transport.getCurrentBeat() == 1);
    assert(transport.getCurrentBar() == 0);
    assert(transport.isNewBeat() == false);
    assert(transport.isNewBar() == false);

    transport.advance(samplesPerBeatAt120);
    assert(transport.getCurrentBeat() == 2);
    assert(transport.getCurrentBar() == 0);
    assert(transport.isNewBeat() == true);
    assert(transport.isNewBar() == false);

    transport.advance(samplesPerBeatAt120);
    assert(transport.getCurrentBeat() == 3);
    assert(transport.getCurrentBar() == 0);
    assert(transport.isNewBeat() == true);
    assert(transport.isNewBar() == false);

    transport.advance(samplesPerBeatAt120);
    assert(transport.getCurrentBeat() == 0);
    assert(transport.getCurrentBar() == 1);
    assert(transport.isNewBeat() == true);
    assert(transport.isNewBar() == true);
}

void testTransportInvalidBpmFallback() {
    Transport transport;
    transport.prepare(44100.0);
    transport.setBpm(0.0f);

    transport.advance(44100);
    assert(std::fabs(transport.getPlayheadPosition() - 1.0) < 1e-6);
}

void testTransportReset() {
    Transport transport;
    transport.prepare(48000.0);
    transport.setBpm(120.0f);

    transport.advance(24000);
    transport.advance(24000);
    assert(transport.getCurrentBeat() == 2);

    transport.reset();
    assert(std::fabs(transport.getPlayheadPosition()) < 1e-6);
    assert(transport.getCurrentBeat() == 0);
    assert(transport.getCurrentBar() == 0);
    assert(transport.isNewBeat() == false);
    assert(transport.isNewBar() == false);
}

void testScaleDegreeAndChordHelpers() {
    Scale cMajor(0, Scale::Type::Major);

    assert(cMajor.getDegree(0, 4) == 48);
    assert(cMajor.getDegree(6, 4) == 59);
    assert(cMajor.getDegree(7, 4) == 60);
    assert(cMajor.getDegree(-1, 4) == 47);

    auto triad = cMajor.getChordTones(0, 4, false);
    assert(triad.size() == 3);
    assert(triad[0] == 48);
    assert(triad[1] == 52);
    assert(triad[2] == 55);

    auto seventh = cMajor.getChordTones(0, 4, true);
    assert(seventh.size() == 4);
    assert(seventh[0] == 48);
    assert(seventh[1] == 52);
    assert(seventh[2] == 55);
    assert(seventh[3] == 59);
}

void testChordProgressionBounds() {
    assert(ChordProgression::getNumProgressions() == 15);
    assert(ChordProgression::isValidDisplayId(1));
    assert(ChordProgression::isValidDisplayId(15));
    assert(!ChordProgression::isValidDisplayId(0));
    assert(!ChordProgression::isValidDisplayId(16));
    assert(!ChordProgression::isValidDisplayId(999));
    assert(ChordProgression::displayIdToProgressionIndex(1) == 0);
    assert(ChordProgression::displayIdToProgressionIndex(15) == 14);
    assert(ChordProgression::getNameByDisplayId(1) == "Trance/Pop");
    assert(ChordProgression::getNameByDisplayId(15) == "Tension Arc");

    const auto& first = ChordProgression::get(0);
    assert(first.name == "Trance/Pop");

    const auto& negativeFallback = ChordProgression::get(-1);
    assert(negativeFallback.name == "Trance/Pop");

    const auto& highFallback = ChordProgression::get(100);
    assert(highFallback.name == "Trance/Pop");
}

void testProjectKeyParseAndFormat() {
    ProjectKey::ParsedValue parsed;

    assert(ProjectKey::parse("A", parsed));
    assert(parsed.root == 9);
    assert(parsed.mode == ProjectKey::kModeMajor);

    assert(ProjectKey::parse("a", parsed));
    assert(parsed.root == 9);
    assert(parsed.mode == ProjectKey::kModeNaturalMinor);

    assert(ProjectKey::parse("A#", parsed));
    assert(parsed.root == 10);
    assert(parsed.mode == ProjectKey::kModeMajor);

    assert(ProjectKey::parse("bb", parsed));
    assert(parsed.root == 10);
    assert(parsed.mode == ProjectKey::kModeNaturalMinor);

    assert(ProjectKey::parse("Db major", parsed));
    assert(parsed.root == 1);
    assert(parsed.mode == ProjectKey::kModeMajor);

    assert(ProjectKey::parse("c#min", parsed));
    assert(parsed.root == 1);
    assert(parsed.mode == ProjectKey::kModeNaturalMinor);

    assert(ProjectKey::parse("Am", parsed));
    assert(parsed.mode == ProjectKey::kModeNaturalMinor);

    assert(ProjectKey::parse("aM", parsed));
    assert(parsed.mode == ProjectKey::kModeMajor);

    assert(!ProjectKey::parse("H", parsed));
    assert(!ProjectKey::parse("", parsed));

    assert(ProjectKey::format(10, ProjectKey::kModeNaturalMinor) == "Bb Minor");
    assert(ProjectKey::format(1, ProjectKey::kModeMajor) == "Db Major");
}

void testEffectPresetCategoryMapping() {
    assert(EffectPresetCatalog::getCategoryName(0) == "Spectral/Resonators");
    assert(EffectPresetCatalog::getCategoryName(1) == "Dynamics");
    assert(EffectPresetCatalog::getCategoryName(2) == "Space");
    assert(EffectPresetCatalog::getCategoryName(3) == "Distortion");
    assert(EffectPresetCatalog::getCategoryName(4) == "Filters");
    assert(EffectPresetCatalog::getCategoryName(5) == "Modulation");
    assert(EffectPresetCatalog::getCategoryName(6) == "Pitch");
    assert(EffectPresetCatalog::getCategoryName(7) == "Degrade");
    assert(EffectPresetCatalog::getCategoryName(8) == "Rhythm");
    assert(EffectPresetCatalog::getCategoryName(9) == "Granular");

    assert(EffectPresetCatalog::getCategoryPresetCount(0) == 14);
    assert(EffectPresetCatalog::getCategoryPresetCount(1) == 18);
    assert(EffectPresetCatalog::getCategoryPresetCount(2) == 37);
    assert(EffectPresetCatalog::getCategoryPresetCount(3) == 19);
    assert(EffectPresetCatalog::getCategoryPresetCount(4) == 23);
    assert(EffectPresetCatalog::getCategoryPresetCount(5) == 16);
    assert(EffectPresetCatalog::getCategoryPresetCount(6) == 12);
    assert(EffectPresetCatalog::getCategoryPresetCount(7) == 15);
    assert(EffectPresetCatalog::getCategoryPresetCount(8) == 20);
    assert(EffectPresetCatalog::getCategoryPresetCount(9) == 13);
    assert(EffectPresetCatalog::getCategoryMappedPresetCount() == 187);

    for (uint8_t category = 0; category <= 9; ++category) {
        assert(EffectPresetCatalog::isCategoryImplemented(category));
        assert(EffectPresetCatalog::isValidCategoryPresetDisplayId(category, 1));
        assert(EffectPresetCatalog::isValidCategoryPresetDisplayId(category, EffectPresetCatalog::getCategoryPresetCount(category)));
        assert(!EffectPresetCatalog::isValidCategoryPresetDisplayId(category, 0));
        assert(!EffectPresetCatalog::isValidCategoryPresetDisplayId(category, static_cast<uint16_t>(EffectPresetCatalog::getCategoryPresetCount(category) + 1)));
    }

    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(0, 1) == "Resonator Body");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(1, 1) == "Comp Tight");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(1, 9) == "Transient Tight");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(2, 1) == "Room Glue");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(2, 35) == "Drum Tight Room");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(2, 36) == "Drum Soft Plate");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(2, 37) == "Drum Dark Room");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(3, 15) == "Asym Scream");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(4, 14) == "EQ Drum Clean");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(4, 19) == "Formant Sweep");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(5, 1) == "Ring Gentle");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(5, 16) == "Flange Metal");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(6, 4) == "Freq Shift Wide ++");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(6, 12) == "Harmony Stack");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(7, 11) == "Erode Void");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(8, 16) == "Repeat Melt 1/4");
    assert(EffectPresetCatalog::getCategoryPresetNameByDisplayId(9, 8) == "Grain Cloud");

    const auto firstCompositePresetId = EffectPresetCatalog::categoryPresetDisplayIdToPresetId(1, 15);
    const auto lastRhythmCompositePresetId = EffectPresetCatalog::categoryPresetDisplayIdToPresetId(8, 20);
    assert(EffectPresetCatalog::isValidCompositePresetId(firstCompositePresetId));
    assert(EffectPresetCatalog::isValidCompositePresetId(lastRhythmCompositePresetId));
    assert(EffectPresetCatalog::getPresetName(firstCompositePresetId) == "Dyn Glue Serial");
    assert(EffectPresetCatalog::getPresetName(lastRhythmCompositePresetId) == "Rhythm Stutter Parallel");
}

void testReduxCrushMix() {
    juce::AudioBuffer<float> dryBuffer(1, 4);
    dryBuffer.setSample(0, 0, -0.75f);
    dryBuffer.setSample(0, 1, -0.25f);
    dryBuffer.setSample(0, 2, 0.25f);
    dryBuffer.setSample(0, 3, 0.75f);

    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(dryBuffer);

    ReduxCrush crusher;
    crusher.prepare(48000.0, 4);
    crusher.setBitDepth(2.0f);
    crusher.setDownsampleFactor(1.0f);
    crusher.setActive(true);

    crusher.setMix(0.0f);
    crusher.processBlock(dryBuffer, dryBuffer.getNumSamples());
    assert(std::fabs(dryBuffer.getSample(0, 0) - -0.75f) < 1.0e-6f);
    assert(std::fabs(dryBuffer.getSample(0, 1) - -0.25f) < 1.0e-6f);
    assert(std::fabs(dryBuffer.getSample(0, 2) - 0.25f) < 1.0e-6f);
    assert(std::fabs(dryBuffer.getSample(0, 3) - 0.75f) < 1.0e-6f);

    crusher.reset();
    crusher.setMix(1.0f);
    crusher.processBlock(wetBuffer, wetBuffer.getNumSamples());
    assert(std::fabs(wetBuffer.getSample(0, 0) - -1.0f) < 1.0e-6f);
    assert(std::fabs(wetBuffer.getSample(0, 1) - -1.0f / 3.0f) < 1.0e-6f);
    assert(std::fabs(wetBuffer.getSample(0, 2) - 1.0f / 3.0f) < 1.0e-6f);
    assert(std::fabs(wetBuffer.getSample(0, 3) - 1.0f) < 1.0e-6f);
}

void testDrumKitReverbsUseGentlePresets() {
    const auto tightRoomId = EffectPresetCatalog::displayIdToPresetId(FxDisplayId::DrumTightRoom);
    const auto softPlateId = EffectPresetCatalog::displayIdToPresetId(FxDisplayId::DrumSoftPlate);
    const auto darkRoomId = EffectPresetCatalog::displayIdToPresetId(FxDisplayId::DrumDarkRoom);
    const auto& tightRoom = EffectPresetCatalog::getCompositePresetById(tightRoomId);
    const auto& softPlate = EffectPresetCatalog::getCompositePresetById(softPlateId);
    const auto& darkRoom = EffectPresetCatalog::getCompositePresetById(darkRoomId);

    assert(tightRoom.name == "Drum Tight Room");
    assert(softPlate.name == "Drum Soft Plate");
    assert(darkRoom.name == "Drum Dark Room");
    assert(tightRoom.componentCount == 1);
    assert(softPlate.componentCount == 1);
    assert(darkRoom.componentCount == 1);
    assert(tightRoom.components[0].type == EffectPresetCatalog::EffectType::ReverbWash);
    assert(softPlate.components[0].type == EffectPresetCatalog::EffectType::ReverbWash);
    assert(darkRoom.components[0].type == EffectPresetCatalog::EffectType::ReverbWash);
    assert(tightRoom.components[0].paramA <= 0.20f);
    assert(softPlate.components[0].paramA <= 0.20f);
    assert(darkRoom.components[0].paramA <= 0.20f);

    constexpr std::array<uint16_t, 5> broadReverbDisplayIds{
        FxDisplayId::RoomGlue,
        FxDisplayId::HallBloom,
        FxDisplayId::InfiniteWash,
        FxDisplayId::PlateBright,
        FxDisplayId::DarkChamber,
    };

    uint8_t gentleReverbUseCount = 0;
    for (uint8_t presetId = 0; presetId < DrumKitPresetCatalog::getPresetCount(); ++presetId) {
        const auto displayIds = DrumKitPresetCatalog::getPreset(presetId).effectSlots.slots;
        for (const uint16_t displayId : displayIds) {
            for (const uint16_t broadReverbDisplayId : broadReverbDisplayIds) {
                assert(displayId != broadReverbDisplayId);
            }

            if (displayId == FxDisplayId::DrumTightRoom
                || displayId == FxDisplayId::DrumSoftPlate
                || displayId == FxDisplayId::DrumDarkRoom) {
                ++gentleReverbUseCount;
            }
        }
    }
    assert(gentleReverbUseCount == 9);
}

void testAlgorithmCatalogExpandedToTwentyPerTrack() {
    for (uint8_t track = 0; track < 4; ++track) {
        assert(AlgorithmCatalog::getAlgorithmCountForTrack(track) == 20);
        assert(AlgorithmCatalog::getAlgorithmName(track, 0) != "Unknown");
        assert(AlgorithmCatalog::getAlgorithmName(track, 19) != "Unknown");
        assert(AlgorithmCatalog::getAlgorithmName(track, 20) == "Unknown");
    }
}

void testSynthCatalogExpandedForMelodicTracks() {
    assert(SynthCatalog::getPresetCountForTrack(0) == DrumKitPresetCatalog::getPresetCount());
    assert(DrumKitPresetCatalog::getPresetCount() == 24);
    assert(SynthCatalog::getPresetCountForTrack(1) == 38);
    assert(SynthCatalog::getPresetCountForTrack(2) == 38);
    assert(SynthCatalog::getPresetCountForTrack(3) == 38);
    assert(SynthCatalog::getAutoSelectablePresetCountForTrack(0) == DrumKitPresetCatalog::getPresetCount());
    assert(SynthCatalog::getAutoSelectablePresetCountForTrack(1) == SynthCatalog::kMelodicSamplerPresetStart);
    assert(SynthCatalog::getAutoSelectablePresetCountForTrack(2) == SynthCatalog::kMelodicSamplerPresetStart);
    assert(SynthCatalog::getAutoSelectablePresetCountForTrack(3) == SynthCatalog::kMelodicSamplerPresetStart);
    assert(SynthCatalog::getPresetName(0, 7) == "Pop Slam");
    assert(SynthCatalog::getPresetName(0, 23) == "Tight Stutter");
    assert(SynthCatalog::getProceduralPresetCountForTrack(1) == SynthCatalog::kMelodicSamplerPresetStart);
    assert(SynthCatalog::getPresetName(1, 37) == "Tonal Drive");
    assert(SynthCatalog::getPresetName(2, 37) == "Supersaw Lift");
    assert(SynthCatalog::getPresetName(3, 37) == "Tube Delay Bloom");
    assert(SynthCatalog::isMelodicSamplerPreset(1, 20));
    assert(!SynthCatalog::isMelodicSamplerPreset(1, 19));
    assert(SynthCatalog::getSoundGenreMask(4, 0) == 0);
    assert(SynthCatalog::getSoundGenreMask(1, 99) == 0);
    assert(!SynthCatalog::soundHasGenre(0, 0, 0));
    assert(!SynthCatalog::soundHasGenre(0, 0, 32));

    const auto& punch = DrumKitPresetCatalog::getPreset(0);
    const auto& subterraneanBloom = DrumKitPresetCatalog::getPreset(17);
    assert(punch.name == "Modular Punch");
    assert(subterraneanBloom.name == "Subterranean Bloom");
    assert(punch.slots[0].sampleId != subterraneanBloom.slots[0].sampleId);
    assert(SynthCatalog::getPresetEffectSlot(0, 0, 0) != SynthCatalog::getPresetEffectSlot(0, 17, 0));
    assert(SynthCatalog::getPresetEffectSlot(0, 17, 1) != 0);

    for (uint8_t track = 0; track < 4; ++track) {
        const uint16_t count = SynthCatalog::getPresetCountForTrack(track);
        assert(count > 0);
        std::array<uint16_t, SynthCatalog::kSoundEffectSlotCount + 1> chainLengthCounts{0, 0, 0, 0};
        std::array<uint16_t, 8> genreCounts{0, 0, 0, 0, 0, 0, 0, 0};
        for (uint16_t displayId = 1; displayId <= count; ++displayId) {
            assert(SynthCatalog::isValidDisplayIdForTrack(track, displayId));
            assert(SynthCatalog::getPresetNameByDisplayId(track, displayId) != "Invalid");
            const uint8_t presetId = static_cast<uint8_t>(displayId - 1);
            assert(SynthCatalog::getSoundGenreMask(track, presetId) != 0);
            for (uint8_t genreId = 1; genreId <= 8; ++genreId) {
                if (SynthCatalog::soundHasGenre(track, presetId, genreId)) {
                    ++genreCounts[genreId - 1];
                }
            }
            uint8_t filledSlots = 0;
            for (uint8_t slot = 0; slot < SynthCatalog::kSoundEffectSlotCount; ++slot) {
                const uint16_t effectPreset = SynthCatalog::getPresetEffectSlot(
                    track,
                    presetId,
                    slot);
                assert(effectPreset == 0 || EffectPresetCatalog::isPresetAssignableToSlot(effectPreset));
                if (effectPreset != 0) {
                    ++filledSlots;
                }
            }
            ++chainLengthCounts[filledSlots];
        }
        assert(!SynthCatalog::isValidDisplayIdForTrack(track, static_cast<uint16_t>(count + 1)));
        assert(chainLengthCounts[0] > 0);
        assert(chainLengthCounts[1] > 0);
        assert(chainLengthCounts[2] > 0);
        assert(chainLengthCounts[3] > 0);
        for (uint8_t genre = 0; genre < genreCounts.size(); ++genre) {
            assert(genreCounts[genre] > 0);
        }
    }

    assert(SynthCatalog::getPresetEffectSlot(0, 0, 0) == EffectPresetCatalog::displayIdToPresetId(32));
    assert(SynthCatalog::getPresetEffectSlot(1, 20, 0) == EffectPresetCatalog::displayIdToPresetId(26));
    assert(SynthCatalog::getPresetEffectSlot(2, 24, 0) == EffectPresetCatalog::displayIdToPresetId(83));
    assert(SynthCatalog::getPresetEffectSlot(3, 31, 2) == EffectPresetCatalog::displayIdToPresetId(11));
}

void testRandomizedStartupProjectSetsNormalizedParameters() {
    AppState state;
    std::mt19937 rng(1337u);

    PresetRegistry::Registry emptyRegistry;
    StartupProjectInitializer::applyRandomizedStartupProject(state, emptyRegistry, rng);

    const float bpm = state.bpm.load();
    assert(bpm >= 80.0f && bpm <= 160.0f);
    assert(bpm != 120.0f);

    for (uint8_t track = 0; track < 4; ++track) {
        const auto& trackState = state.tracks[track];

        assert(trackState.algorithmId.load() < AlgorithmCatalog::getAlgorithmCountForTrack(track));
        assert(trackState.synthPreset.load() < SynthCatalog::getPresetCountForTrack(track));
        assert(trackState.density.load() >= 0.0f && trackState.density.load() <= 1.0f);
        assert(trackState.complexity.load() >= 0.0f && trackState.complexity.load() <= 1.0f);
        assert(trackState.tone.load() >= 0.0f && trackState.tone.load() <= 1.0f);
        assert(trackState.motion.load() >= 0.0f && trackState.motion.load() <= 1.0f);

        const auto expectedEffectSlots = SynthCatalog::getPresetEffectSlots(track, trackState.synthPreset.load());
        for (uint8_t slot = 0; slot < 3; ++slot) {
            assert(trackState.getEffectPresetSlot(slot) == expectedEffectSlots[slot]);
        }

        if (track > 0) {
            assert(trackState.synthManualOverride.load() == true);
        }
    }

    const auto& drumPreset = DrumKitPresetCatalog::getPreset(state.tracks[0].synthPreset.load());
    for (uint8_t slot = 0; slot < AppState::TrackState::DrumSampleSlotCount; ++slot) {
        assert(state.tracks[0].getDrumSampleSlotSampleId(slot) == drumPreset.slots[slot].sampleId);
        assert(state.tracks[0].getDrumSampleSlotVolume(slot) == drumPreset.slots[slot].volume);
        assert(state.tracks[0].getDrumSampleSlotTuneSemitones(slot) == drumPreset.slots[slot].tuneSemitones);
        assert(state.tracks[0].getDrumSampleSlotStartOffset(slot) == drumPreset.slots[slot].startOffset);
        assert(state.tracks[0].getDrumSampleSlotDecay(slot) == drumPreset.slots[slot].decay);
        assert(state.tracks[0].getDrumSampleSlotVelocitySensitivity(slot) == drumPreset.slots[slot].velocitySensitivity);
    }

    assert(state.master.getEffectPresetSlot(0) == 0);
    assert(state.master.getEffectPresetSlot(1) == 0);
    assert(state.master.getEffectPresetSlot(2) == EffectPresetCatalog::kDefaultMasterLimiterPresetId);
    const auto& masterLimiter = EffectPresetCatalog::getPresetById(state.master.getEffectPresetSlot(2));
    assert(masterLimiter.type == EffectPresetCatalog::EffectType::PeakLimiter);
    assert(masterLimiter.name == "Limiter Transparent");
    assert(state.projectKeyRoot.load() < 12);
    const uint8_t keyMode = state.projectKeyMode.load();
    assert(keyMode == AppState::kProjectKeyModeMajor || keyMode == AppState::kProjectKeyModeNaturalMinor);
    assert(state.activeSpotEffects.load() == 0);
}

bool producesNoteOnAtLowDensity(GenerativeAlgorithm& algorithm, float density = 0.1f) {
    juce::MidiBuffer midi;
    const Scale scale(0, Scale::Type::NaturalMinor);
    algorithm.reset();
    algorithm.processMidi(midi,
                          0.0,
                          16.0,
                          4096,
                          scale,
                          scale.getDegree(0, 3),
                          density,
                          0.5f);

    for (const auto meta : midi) {
        if (meta.getMessage().isNoteOn()) {
            return true;
        }
    }

    return false;
}

void runGeneratorBlock(GenerativeAlgorithm& algorithm,
                       juce::MidiBuffer& midi,
                       double playheadBeats,
                       double blockLengthBeats,
                       int blockSamples,
                       const Scale& scale,
                       float density = 0.7f,
                       float complexity = 0.5f) {
    midi.clear();
    algorithm.flushPendingNoteOffs(midi, playheadBeats, blockLengthBeats, blockSamples);
    algorithm.processMidi(midi,
                          playheadBeats,
                          blockLengthBeats,
                          blockSamples,
                          scale,
                          scale.getDegree(0, 3),
                          density,
                          complexity);
}

int countMessages(const juce::MidiBuffer& midi, bool noteOn) {
    int count = 0;
    for (const auto meta : midi) {
        const auto msg = meta.getMessage();
        if ((noteOn && msg.isNoteOn()) || (!noteOn && msg.isNoteOff())) {
            ++count;
        }
    }
    return count;
}

void testSubBassAlgorithmsStayInSampleRange() {
    std::vector<std::unique_ptr<GenerativeAlgorithm>> algorithms;
    algorithms.push_back(std::make_unique<Sub808Bass>());
    algorithms.push_back(std::make_unique<DubPedalBass>());
    algorithms.push_back(std::make_unique<ReggaetonSubBass>());
    algorithms.push_back(std::make_unique<MinimalDroneBass>());
    algorithms.push_back(std::make_unique<StepperDubBass>());
    algorithms.push_back(std::make_unique<NeuroWobbleBass>());

    for (int root = 0; root < 12; ++root) {
        const Scale scale(root, Scale::Type::NaturalMinor);
        for (auto& algorithm : algorithms) {
            juce::MidiBuffer midi;
            runGeneratorBlock(*algorithm, midi, 0.0, 4.0, 4096, scale, 0.8f, 0.8f);

            bool producedNote = false;
            for (const auto meta : midi) {
                const auto message = meta.getMessage();
                if (message.isNoteOn()) {
                    producedNote = true;
                    assert(message.getNoteNumber() >= 24);
                }
            }
            assert(producedNote);
        }
    }
}

void testBeatScheduledSustainsAcrossBlocks() {
    const Scale scale(0, Scale::Type::NaturalMinor);
    BlockChords chords;
    juce::MidiBuffer midi;

    runGeneratorBlock(chords, midi, 0.0, 0.25, 256, scale);
    assert(countMessages(midi, true) > 0);
    assert(countMessages(midi, false) == 0);

    runGeneratorBlock(chords, midi, 3.75, 0.25, 256, scale);
    assert(countMessages(midi, false) > 0);
}

void testShortBeatScheduledNotesEndInSameBlock() {
    const Scale scale(0, Scale::Type::NaturalMinor);
    MinimalPluckChords pluck;
    juce::MidiBuffer midi;

    runGeneratorBlock(pluck, midi, 0.0, 0.5, 512, scale);
    assert(countMessages(midi, true) > 0);
    assert(countMessages(midi, false) > 0);
}

void testCustomAlgorithmNoteLengthCanCrossBlocks() {
    CustomAlgorithmPreset preset;
    preset.trackIndex = 1;
    preset.stepCount = 4;
    preset.noteLength = 1.0f;
    preset.rhythmicPattern = {1, 0, 0, 0};
    preset.melodicPattern = {0, 0, 0, 0};
    preset.densityCurve = {1.0f, 1.0f, 1.0f, 1.0f};
    preset.complexityCurve = {0.0f, 0.0f, 0.0f, 0.0f};

    const Scale scale(0, Scale::Type::NaturalMinor);
    CustomAlgorithmInstance custom(preset);
    juce::MidiBuffer midi;

    runGeneratorBlock(custom, midi, 0.0, 0.25, 256, scale);
    assert(countMessages(midi, true) == 1);
    assert(countMessages(midi, false) == 0);

    runGeneratorBlock(custom, midi, 0.75, 0.25, 256, scale);
    assert(countMessages(midi, false) == 1);
}

void testResetClearsPendingNoteOffs() {
    const Scale scale(0, Scale::Type::NaturalMinor);
    BlockChords chords;
    juce::MidiBuffer midi;

    runGeneratorBlock(chords, midi, 0.0, 0.25, 256, scale);
    chords.reset();
    runGeneratorBlock(chords, midi, 3.75, 0.25, 256, scale);
    assert(countMessages(midi, false) == 0);
}

void testLowDensityAlgorithmsStillProduceMidi() {
    std::vector<std::unique_ptr<GenerativeAlgorithm>> algorithms;
    algorithms.push_back(std::make_unique<SyncBass>());
    algorithms.push_back(std::make_unique<UKGarageBass>());
    algorithms.push_back(std::make_unique<TumbaoBass>());
    algorithms.push_back(std::make_unique<MotifBass>());
    algorithms.push_back(std::make_unique<AcidTripletBass>());
    algorithms.push_back(std::make_unique<GlideCounterBass>());
    algorithms.push_back(std::make_unique<PulseChopBass>());
    algorithms.push_back(std::make_unique<OctaveBounceBass>());
    algorithms.push_back(std::make_unique<SyncStabs>());
    algorithms.push_back(std::make_unique<HousePianoStabs>());
    algorithms.push_back(std::make_unique<QuartalComping>());
    algorithms.push_back(std::make_unique<BrokenStrumChords>());
    algorithms.push_back(std::make_unique<PulseClusterChords>());
    algorithms.push_back(std::make_unique<AfroCallResponseLead>());
    algorithms.push_back(std::make_unique<GlideRunLead>());
    algorithms.push_back(std::make_unique<WideIntervalLead>());
    algorithms.push_back(std::make_unique<TripletRushLead>());
    algorithms.push_back(std::make_unique<ElectroBreaksGroove>());
    algorithms.push_back(std::make_unique<GarageSwingGroove>());
    algorithms.push_back(std::make_unique<LatinPercGroove>());
    algorithms.push_back(std::make_unique<MinimalClicksGroove>());
    algorithms.push_back(std::make_unique<DubSkankGroove>());
    algorithms.push_back(std::make_unique<Footwork160Groove>());
    algorithms.push_back(std::make_unique<HalfstepGroove>());
    algorithms.push_back(std::make_unique<IndustrialGroove>());
    algorithms.push_back(std::make_unique<ReggaetonSubBass>());
    algorithms.push_back(std::make_unique<ElectroFunkBass>());
    algorithms.push_back(std::make_unique<MinimalDroneBass>());
    algorithms.push_back(std::make_unique<BrokenOctaveBass>());
    algorithms.push_back(std::make_unique<StepperDubBass>());
    algorithms.push_back(std::make_unique<FunkPopBass>());
    algorithms.push_back(std::make_unique<NeuroWobbleBass>());
    algorithms.push_back(std::make_unique<ClaveBass>());
    algorithms.push_back(std::make_unique<DubSkankChords>());
    algorithms.push_back(std::make_unique<MinimalPluckChords>());
    algorithms.push_back(std::make_unique<RNBKeyChords>());
    algorithms.push_back(std::make_unique<SuspendedPadChords>());
    algorithms.push_back(std::make_unique<CinematicHitChords>());
    algorithms.push_back(std::make_unique<FifthDroneChords>());
    algorithms.push_back(std::make_unique<GarageOrganChords>());
    algorithms.push_back(std::make_unique<PolychordChords>());
    algorithms.push_back(std::make_unique<PentatonicHookLead>());
    algorithms.push_back(std::make_unique<AcidLineLead>());
    algorithms.push_back(std::make_unique<DubEchoLead>());
    algorithms.push_back(std::make_unique<GarageVoxLead>());
    algorithms.push_back(std::make_unique<MinimalPingLead>());
    algorithms.push_back(std::make_unique<OrnamentRunLead>());
    algorithms.push_back(std::make_unique<SyncopatedPluckLead>());
    algorithms.push_back(std::make_unique<LydianFloatLead>());

    for (const auto& algorithm : algorithms) {
        assert(producesNoteOnAtLowDensity(*algorithm));
    }
}

} // namespace

int main() {
    testAppStateDefaultsAndSetters();
    testTransportBeatBarTransitions();
    testTransportInvalidBpmFallback();
    testTransportReset();
    testScaleDegreeAndChordHelpers();
    testChordProgressionBounds();
    testProjectKeyParseAndFormat();
    testEffectPresetCategoryMapping();
    testReduxCrushMix();
    testSpotStutterDefaultsAndWrappedRepeat();
    testDrumKitReverbsUseGentlePresets();
    testAlgorithmCatalogExpandedToTwentyPerTrack();
    testSynthCatalogExpandedForMelodicTracks();
    testRandomizedStartupProjectSetsNormalizedParameters();
    testLowDensityAlgorithmsStillProduceMidi();
    testSubBassAlgorithmsStayInSampleRange();
    testBeatScheduledSustainsAcrossBlocks();
    testShortBeatScheduledNotesEndInSameBlock();
    testCustomAlgorithmNoteLengthCanCrossBlocks();
    testResetClearsPendingNoteOffs();

    std::cout << "Core logic tests passed!\n";
    return 0;
}
