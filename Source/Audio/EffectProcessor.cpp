#include "EffectProcessor.h"

#include "../App/PresetRegistry.h"
#include "../App/SpotEffectCatalog.h"

#include <algorithm>
#include <limits>

EffectProcessor::EffectProcessor(AppState& appState)
    : appState(appState)
{
}

void EffectProcessor::prepare(double sampleRate, int maximumBlockSize)
{
    for (uint8_t track = 0; track < TrackCount; ++track)
    {
        for (uint8_t slot = 0; slot < EffectSlotCount; ++slot)
        {
            tapeStops[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            beatRepeats[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            highPassSweeps[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            reverbWashes[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            reduxCrushers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            delayEchos[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            saturators[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            softHardClips[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            wavefolders[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            asymShapers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            compressors[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            limiters[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            transientShapers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            combFilters[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            multiModeEqs[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            formantFilters[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            autopans[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            ringModulators[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            choruses[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            phasers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            flangers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            jitterDegrades[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            erosionDegrades[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            tranceGates[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            sidechainDuckers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            beatRepeatInserts[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            frequencyShifters[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            pitchShifters[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            harmonizers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            timeFreezers[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            grainDelays[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            physicalModelingResonators[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            multibandOtts[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            convolutionReverbs[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            tapeDelays[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            pingPongDelays[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            cloudGenerators[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            spectralBlurs[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
            spectralDelays[track][slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));

            cachedTrackEffectPresets[track][slot] = std::numeric_limits<uint16_t>::max();
            const uint16_t presetId = appState.tracks[track].getEffectPresetSlot(slot);
            applyTrackEffectPreset(track, slot, presetId);
        }
    }

    for (uint8_t slot = 0; slot < EffectSlotCount; ++slot)
    {
        masterTapeStops[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterBeatRepeats[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterHighPassSweeps[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterReverbWashes[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterReduxCrushers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterDelayEchos[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterSaturators[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterSoftHardClips[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterWavefolders[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterAsymShapers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterCompressors[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterLimiters[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterTransientShapers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterCombFilters[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterMultiModeEqs[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterFormantFilters[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterAutopans[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterRingModulators[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterChoruses[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterPhasers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterFlangers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterJitterDegrades[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterErosionDegrades[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterTranceGates[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterSidechainDuckers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterBeatRepeatInserts[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterFrequencyShifters[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterPitchShifters[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterHarmonizers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterTimeFreezers[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterGrainDelays[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterPhysicalModelingResonators[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterMultibandOtts[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterConvolutionReverbs[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterTapeDelays[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterPingPongDelays[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterCloudGenerators[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterSpectralBlurs[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));
        masterSpectralDelays[slot].prepare(sampleRate, static_cast<int>(maximumBlockSize));

        cachedMasterEffectPresets[slot] = std::numeric_limits<uint16_t>::max();
        const uint16_t presetId = appState.master.getEffectPresetSlot(slot);
        applyMasterEffectPreset(slot, presetId);
    }

    spotTapeStop.prepare(sampleRate, static_cast<int>(maximumBlockSize));
    spotTapeStop.setStopTimeSeconds(SpotEffectCatalog::getTapeBrakeDefaultStopTimeSeconds());
    spotBeatRepeat.prepare(sampleRate, static_cast<int>(maximumBlockSize));
    spotBeatRepeat.setRepeatDivision(SpotEffectCatalog::getStutterDefaultRepeatDivision());
    spotBeatRepeat.setMix(SpotEffectCatalog::getStutterDefaultMix());
    cachedSpotEffectsMask = 0xFFu;
    applySpotEffectsBitmask(appState.activeSpotEffects.load(std::memory_order_relaxed));

    for (auto& workBuffer : compositeWorkBuffers)
    {
        workBuffer.setSize(2, static_cast<int>(maximumBlockSize), false, false, true);
        workBuffer.clear();
    }
}

void EffectProcessor::configureTrackEffectFromParams(uint8_t trackIndex, uint8_t slotIndex, EffectPresetCatalog::EffectType type, float paramA, float paramB, float paramC, float inputGainDb, float outputGainDb, std::string_view irResourceName)
{
    switch (type)
    {
        case EffectPresetCatalog::EffectType::None:
            break;
        case EffectPresetCatalog::EffectType::TapeStop:
            tapeStops[trackIndex][slotIndex].setStopTimeSeconds(paramA);
            tapeStops[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeat:
            beatRepeats[trackIndex][slotIndex].setRepeatDivision(paramA);
            beatRepeats[trackIndex][slotIndex].setMix(paramB);
            beatRepeats[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::HighPassSweep:
            highPassSweeps[trackIndex][slotIndex].setTargetCutoff(paramA);
            highPassSweeps[trackIndex][slotIndex].setSweepRate(paramB);
            highPassSweeps[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ReverbWash:
            reverbWashes[trackIndex][slotIndex].setTargetMix(paramA);
            reverbWashes[trackIndex][slotIndex].setRoomSize(paramB);
            reverbWashes[trackIndex][slotIndex].setDamping(paramC);
            reverbWashes[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ReduxCrush:
            reduxCrushers[trackIndex][slotIndex].setBitDepth(paramA);
            reduxCrushers[trackIndex][slotIndex].setDownsampleFactor(paramB);
            reduxCrushers[trackIndex][slotIndex].setMix(paramC);
            reduxCrushers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::DelayEcho:
            delayEchos[trackIndex][slotIndex].setDelayMs(paramA);
            delayEchos[trackIndex][slotIndex].setFeedback(paramB);
            delayEchos[trackIndex][slotIndex].setMix(paramC);
            delayEchos[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SaturationWaveshaper:
            saturators[trackIndex][slotIndex].setDrive(paramA);
            saturators[trackIndex][slotIndex].setMix(paramB);
            saturators[trackIndex][slotIndex].setInputGainDb(inputGainDb);
            saturators[trackIndex][slotIndex].setOutputTrimDb(paramC + outputGainDb);
            saturators[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SoftHardClip:
            softHardClips[trackIndex][slotIndex].setDrive(paramA);
            softHardClips[trackIndex][slotIndex].setHardness(paramB);
            softHardClips[trackIndex][slotIndex].setMix(paramC);
            softHardClips[trackIndex][slotIndex].setInputGainDb(inputGainDb);
            softHardClips[trackIndex][slotIndex].setOutputGainDb(outputGainDb);
            softHardClips[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Wavefolder:
            wavefolders[trackIndex][slotIndex].setDrive(paramA);
            wavefolders[trackIndex][slotIndex].setFoldAmount(paramB);
            wavefolders[trackIndex][slotIndex].setMix(paramC);
            wavefolders[trackIndex][slotIndex].setInputGainDb(inputGainDb);
            wavefolders[trackIndex][slotIndex].setOutputGainDb(outputGainDb);
            wavefolders[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::AsymShaper:
            asymShapers[trackIndex][slotIndex].setDrive(paramA);
            asymShapers[trackIndex][slotIndex].setAsymmetry(paramB);
            asymShapers[trackIndex][slotIndex].setMix(paramC);
            asymShapers[trackIndex][slotIndex].setInputGainDb(inputGainDb);
            asymShapers[trackIndex][slotIndex].setOutputGainDb(outputGainDb);
            asymShapers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CompressorGlue:
            compressors[trackIndex][slotIndex].setThresholdDb(paramA);
            compressors[trackIndex][slotIndex].setRatio(paramB);
            compressors[trackIndex][slotIndex].setMakeupDb(paramC);
            compressors[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PeakLimiter:
            limiters[trackIndex][slotIndex].setCeilingDb(paramA);
            limiters[trackIndex][slotIndex].setReleaseMs(paramB);
            limiters[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TransientShaper:
            transientShapers[trackIndex][slotIndex].setAttack(paramA);
            transientShapers[trackIndex][slotIndex].setSustain(paramB);
            transientShapers[trackIndex][slotIndex].setMix(paramC);
            transientShapers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CombFilter:
            combFilters[trackIndex][slotIndex].setDelayMs(paramA);
            combFilters[trackIndex][slotIndex].setFeedback(paramB);
            combFilters[trackIndex][slotIndex].setMix(paramC);
            combFilters[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::MultiModeEQ:
            multiModeEqs[trackIndex][slotIndex].setModeSelector(paramC);
            multiModeEqs[trackIndex][slotIndex].setFrequencyHz(paramA);
            multiModeEqs[trackIndex][slotIndex].setShape(paramB);
            multiModeEqs[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::FormantFilter:
            formantFilters[trackIndex][slotIndex].setVowel(paramA);
            formantFilters[trackIndex][slotIndex].setResonance(paramB);
            formantFilters[trackIndex][slotIndex].setMix(paramC);
            formantFilters[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Autopan:
            autopans[trackIndex][slotIndex].setRateHz(paramA);
            autopans[trackIndex][slotIndex].setDepth(paramB);
            autopans[trackIndex][slotIndex].setWidth(paramC);
            autopans[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::RingModulator:
            ringModulators[trackIndex][slotIndex].setRateHz(paramA);
            ringModulators[trackIndex][slotIndex].setDepth(paramB);
            ringModulators[trackIndex][slotIndex].setMix(paramC);
            ringModulators[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Chorus:
            choruses[trackIndex][slotIndex].setRateHz(paramA);
            choruses[trackIndex][slotIndex].setDepthMs(paramB);
            choruses[trackIndex][slotIndex].setMix(paramC);
            choruses[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Phaser:
            phasers[trackIndex][slotIndex].setRateHz(paramA);
            phasers[trackIndex][slotIndex].setDepth(paramB);
            phasers[trackIndex][slotIndex].setMix(paramC);
            phasers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Flanger:
            flangers[trackIndex][slotIndex].setRateHz(paramA);
            flangers[trackIndex][slotIndex].setFeedback(paramB);
            flangers[trackIndex][slotIndex].setMix(paramC);
            flangers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::JitterDegrade:
            jitterDegrades[trackIndex][slotIndex].setBaseDelayMs(paramA);
            jitterDegrades[trackIndex][slotIndex].setJitterMs(paramB);
            jitterDegrades[trackIndex][slotIndex].setMix(paramC);
            jitterDegrades[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ErosionDegrade:
            erosionDegrades[trackIndex][slotIndex].setDropProbability(paramA);
            erosionDegrades[trackIndex][slotIndex].setHoldMs(paramB);
            erosionDegrades[trackIndex][slotIndex].setMix(paramC);
            erosionDegrades[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TranceGate:
            tranceGates[trackIndex][slotIndex].setRepeatDivision(paramA);
            tranceGates[trackIndex][slotIndex].setDepth(paramB);
            tranceGates[trackIndex][slotIndex].setDutyCycle(paramC);
            tranceGates[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SidechainDucker:
            sidechainDuckers[trackIndex][slotIndex].setRepeatDivision(paramA);
            sidechainDuckers[trackIndex][slotIndex].setDepth(paramB);
            sidechainDuckers[trackIndex][slotIndex].setCurve(paramC);
            sidechainDuckers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeatInsert:
            beatRepeatInserts[trackIndex][slotIndex].setRepeatDivision(paramA);
            beatRepeatInserts[trackIndex][slotIndex].setMix(paramB);
            beatRepeatInserts[trackIndex][slotIndex].setFeedback(paramC);
            beatRepeatInserts[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::FrequencyShifter:
            frequencyShifters[trackIndex][slotIndex].setShiftHz(paramA);
            frequencyShifters[trackIndex][slotIndex].setMix(paramB);
            frequencyShifters[trackIndex][slotIndex].setStereoPhase(paramC);
            frequencyShifters[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PitchShifter:
            pitchShifters[trackIndex][slotIndex].setSemitones(paramA);
            pitchShifters[trackIndex][slotIndex].setWindowMs(paramB);
            pitchShifters[trackIndex][slotIndex].setMix(paramC);
            pitchShifters[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Harmonizer:
            harmonizers[trackIndex][slotIndex].setIntervalSemitones(paramA);
            harmonizers[trackIndex][slotIndex].setBlend(paramB);
            harmonizers[trackIndex][slotIndex].setMix(paramC);
            harmonizers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TimeFreezer:
            timeFreezers[trackIndex][slotIndex].setFreezeWindowMs(paramA);
            timeFreezers[trackIndex][slotIndex].setPlaybackRate(paramB);
            timeFreezers[trackIndex][slotIndex].setMix(paramC);
            timeFreezers[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::GrainDelay:
            grainDelays[trackIndex][slotIndex].setGrainMs(paramA);
            grainDelays[trackIndex][slotIndex].setScatter(paramB);
            grainDelays[trackIndex][slotIndex].setMix(paramC);
            grainDelays[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PhysicalModelingResonator:
            physicalModelingResonators[trackIndex][slotIndex].setBaseFrequencyHz(paramA);
            physicalModelingResonators[trackIndex][slotIndex].setResonance(paramB);
            physicalModelingResonators[trackIndex][slotIndex].setMix(paramC);
            physicalModelingResonators[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::MultibandOtt:
            multibandOtts[trackIndex][slotIndex].setLowThresholdDb(paramA);
            multibandOtts[trackIndex][slotIndex].setMidRatio(paramB);
            multibandOtts[trackIndex][slotIndex].setHighRatio(paramC);
            multibandOtts[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ConvolutionReverb:
            convolutionReverbs[trackIndex][slotIndex].loadIrFromResource(
                irResourceName.empty() ? EffectPresetCatalog::kDefaultConvolutionResourceName : irResourceName);
            convolutionReverbs[trackIndex][slotIndex].setMix(paramA);
            convolutionReverbs[trackIndex][slotIndex].setPreDelayMs(paramB);
            convolutionReverbs[trackIndex][slotIndex].setIrGain(paramC);
            convolutionReverbs[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TapeDelay:
            tapeDelays[trackIndex][slotIndex].setDelayMs(paramA);
            tapeDelays[trackIndex][slotIndex].setFeedback(paramB);
            tapeDelays[trackIndex][slotIndex].setSaturation(paramC);
            tapeDelays[trackIndex][slotIndex].setMix(0.35f);
            tapeDelays[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PingPongDelay:
            pingPongDelays[trackIndex][slotIndex].setDelayMs(paramA);
            pingPongDelays[trackIndex][slotIndex].setFeedback(paramB);
            pingPongDelays[trackIndex][slotIndex].setMix(paramC);
            pingPongDelays[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CloudGenerator:
            cloudGenerators[trackIndex][slotIndex].setGrainSizeMs(paramA);
            cloudGenerators[trackIndex][slotIndex].setDensity(paramB);
            cloudGenerators[trackIndex][slotIndex].setMix(paramC);
            cloudGenerators[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SpectralBlur:
            spectralBlurs[trackIndex][slotIndex].setBlurTimeMs(paramA);
            spectralBlurs[trackIndex][slotIndex].setFrequencyRange(paramB);
            spectralBlurs[trackIndex][slotIndex].setMix(paramC);
            spectralBlurs[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SpectralDelay:
            spectralDelays[trackIndex][slotIndex].setDelayMs(paramA);
            spectralDelays[trackIndex][slotIndex].setFeedback(paramB);
            spectralDelays[trackIndex][slotIndex].setMix(paramC);
            spectralDelays[trackIndex][slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CompositeCategory:
            break;
    }
}

void EffectProcessor::configureMasterEffectFromParams(uint8_t slotIndex, EffectPresetCatalog::EffectType type, float paramA, float paramB, float paramC, float inputGainDb, float outputGainDb, std::string_view irResourceName)
{
    switch (type)
    {
        case EffectPresetCatalog::EffectType::None:
            break;
        case EffectPresetCatalog::EffectType::TapeStop:
            masterTapeStops[slotIndex].setStopTimeSeconds(paramA);
            masterTapeStops[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeat:
            masterBeatRepeats[slotIndex].setRepeatDivision(paramA);
            masterBeatRepeats[slotIndex].setMix(paramB);
            masterBeatRepeats[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::HighPassSweep:
            masterHighPassSweeps[slotIndex].setTargetCutoff(paramA);
            masterHighPassSweeps[slotIndex].setSweepRate(paramB);
            masterHighPassSweeps[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ReverbWash:
            masterReverbWashes[slotIndex].setTargetMix(paramA);
            masterReverbWashes[slotIndex].setRoomSize(paramB);
            masterReverbWashes[slotIndex].setDamping(paramC);
            masterReverbWashes[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ReduxCrush:
            masterReduxCrushers[slotIndex].setBitDepth(paramA);
            masterReduxCrushers[slotIndex].setDownsampleFactor(paramB);
            masterReduxCrushers[slotIndex].setMix(paramC);
            masterReduxCrushers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::DelayEcho:
            masterDelayEchos[slotIndex].setDelayMs(paramA);
            masterDelayEchos[slotIndex].setFeedback(paramB);
            masterDelayEchos[slotIndex].setMix(paramC);
            masterDelayEchos[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SaturationWaveshaper:
            masterSaturators[slotIndex].setDrive(paramA);
            masterSaturators[slotIndex].setMix(paramB);
            masterSaturators[slotIndex].setInputGainDb(inputGainDb);
            masterSaturators[slotIndex].setOutputTrimDb(paramC + outputGainDb);
            masterSaturators[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SoftHardClip:
            masterSoftHardClips[slotIndex].setDrive(paramA);
            masterSoftHardClips[slotIndex].setHardness(paramB);
            masterSoftHardClips[slotIndex].setMix(paramC);
            masterSoftHardClips[slotIndex].setInputGainDb(inputGainDb);
            masterSoftHardClips[slotIndex].setOutputGainDb(outputGainDb);
            masterSoftHardClips[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Wavefolder:
            masterWavefolders[slotIndex].setDrive(paramA);
            masterWavefolders[slotIndex].setFoldAmount(paramB);
            masterWavefolders[slotIndex].setMix(paramC);
            masterWavefolders[slotIndex].setInputGainDb(inputGainDb);
            masterWavefolders[slotIndex].setOutputGainDb(outputGainDb);
            masterWavefolders[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::AsymShaper:
            masterAsymShapers[slotIndex].setDrive(paramA);
            masterAsymShapers[slotIndex].setAsymmetry(paramB);
            masterAsymShapers[slotIndex].setMix(paramC);
            masterAsymShapers[slotIndex].setInputGainDb(inputGainDb);
            masterAsymShapers[slotIndex].setOutputGainDb(outputGainDb);
            masterAsymShapers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CompressorGlue:
            masterCompressors[slotIndex].setThresholdDb(paramA);
            masterCompressors[slotIndex].setRatio(paramB);
            masterCompressors[slotIndex].setMakeupDb(paramC);
            masterCompressors[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PeakLimiter:
            masterLimiters[slotIndex].setCeilingDb(paramA);
            masterLimiters[slotIndex].setReleaseMs(paramB);
            masterLimiters[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TransientShaper:
            masterTransientShapers[slotIndex].setAttack(paramA);
            masterTransientShapers[slotIndex].setSustain(paramB);
            masterTransientShapers[slotIndex].setMix(paramC);
            masterTransientShapers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CombFilter:
            masterCombFilters[slotIndex].setDelayMs(paramA);
            masterCombFilters[slotIndex].setFeedback(paramB);
            masterCombFilters[slotIndex].setMix(paramC);
            masterCombFilters[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::MultiModeEQ:
            masterMultiModeEqs[slotIndex].setModeSelector(paramC);
            masterMultiModeEqs[slotIndex].setFrequencyHz(paramA);
            masterMultiModeEqs[slotIndex].setShape(paramB);
            masterMultiModeEqs[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::FormantFilter:
            masterFormantFilters[slotIndex].setVowel(paramA);
            masterFormantFilters[slotIndex].setResonance(paramB);
            masterFormantFilters[slotIndex].setMix(paramC);
            masterFormantFilters[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Autopan:
            masterAutopans[slotIndex].setRateHz(paramA);
            masterAutopans[slotIndex].setDepth(paramB);
            masterAutopans[slotIndex].setWidth(paramC);
            masterAutopans[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::RingModulator:
            masterRingModulators[slotIndex].setRateHz(paramA);
            masterRingModulators[slotIndex].setDepth(paramB);
            masterRingModulators[slotIndex].setMix(paramC);
            masterRingModulators[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Chorus:
            masterChoruses[slotIndex].setRateHz(paramA);
            masterChoruses[slotIndex].setDepthMs(paramB);
            masterChoruses[slotIndex].setMix(paramC);
            masterChoruses[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Phaser:
            masterPhasers[slotIndex].setRateHz(paramA);
            masterPhasers[slotIndex].setDepth(paramB);
            masterPhasers[slotIndex].setMix(paramC);
            masterPhasers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Flanger:
            masterFlangers[slotIndex].setRateHz(paramA);
            masterFlangers[slotIndex].setFeedback(paramB);
            masterFlangers[slotIndex].setMix(paramC);
            masterFlangers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::JitterDegrade:
            masterJitterDegrades[slotIndex].setBaseDelayMs(paramA);
            masterJitterDegrades[slotIndex].setJitterMs(paramB);
            masterJitterDegrades[slotIndex].setMix(paramC);
            masterJitterDegrades[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ErosionDegrade:
            masterErosionDegrades[slotIndex].setDropProbability(paramA);
            masterErosionDegrades[slotIndex].setHoldMs(paramB);
            masterErosionDegrades[slotIndex].setMix(paramC);
            masterErosionDegrades[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TranceGate:
            masterTranceGates[slotIndex].setRepeatDivision(paramA);
            masterTranceGates[slotIndex].setDepth(paramB);
            masterTranceGates[slotIndex].setDutyCycle(paramC);
            masterTranceGates[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SidechainDucker:
            masterSidechainDuckers[slotIndex].setRepeatDivision(paramA);
            masterSidechainDuckers[slotIndex].setDepth(paramB);
            masterSidechainDuckers[slotIndex].setCurve(paramC);
            masterSidechainDuckers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeatInsert:
            masterBeatRepeatInserts[slotIndex].setRepeatDivision(paramA);
            masterBeatRepeatInserts[slotIndex].setMix(paramB);
            masterBeatRepeatInserts[slotIndex].setFeedback(paramC);
            masterBeatRepeatInserts[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::FrequencyShifter:
            masterFrequencyShifters[slotIndex].setShiftHz(paramA);
            masterFrequencyShifters[slotIndex].setMix(paramB);
            masterFrequencyShifters[slotIndex].setStereoPhase(paramC);
            masterFrequencyShifters[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PitchShifter:
            masterPitchShifters[slotIndex].setSemitones(paramA);
            masterPitchShifters[slotIndex].setWindowMs(paramB);
            masterPitchShifters[slotIndex].setMix(paramC);
            masterPitchShifters[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::Harmonizer:
            masterHarmonizers[slotIndex].setIntervalSemitones(paramA);
            masterHarmonizers[slotIndex].setBlend(paramB);
            masterHarmonizers[slotIndex].setMix(paramC);
            masterHarmonizers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TimeFreezer:
            masterTimeFreezers[slotIndex].setFreezeWindowMs(paramA);
            masterTimeFreezers[slotIndex].setPlaybackRate(paramB);
            masterTimeFreezers[slotIndex].setMix(paramC);
            masterTimeFreezers[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::GrainDelay:
            masterGrainDelays[slotIndex].setGrainMs(paramA);
            masterGrainDelays[slotIndex].setScatter(paramB);
            masterGrainDelays[slotIndex].setMix(paramC);
            masterGrainDelays[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PhysicalModelingResonator:
            masterPhysicalModelingResonators[slotIndex].setBaseFrequencyHz(paramA);
            masterPhysicalModelingResonators[slotIndex].setResonance(paramB);
            masterPhysicalModelingResonators[slotIndex].setMix(paramC);
            masterPhysicalModelingResonators[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::MultibandOtt:
            masterMultibandOtts[slotIndex].setLowThresholdDb(paramA);
            masterMultibandOtts[slotIndex].setMidRatio(paramB);
            masterMultibandOtts[slotIndex].setHighRatio(paramC);
            masterMultibandOtts[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::ConvolutionReverb:
            masterConvolutionReverbs[slotIndex].loadIrFromResource(
                irResourceName.empty() ? EffectPresetCatalog::kDefaultConvolutionResourceName : irResourceName);
            masterConvolutionReverbs[slotIndex].setMix(paramA);
            masterConvolutionReverbs[slotIndex].setPreDelayMs(paramB);
            masterConvolutionReverbs[slotIndex].setIrGain(paramC);
            masterConvolutionReverbs[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::TapeDelay:
            masterTapeDelays[slotIndex].setDelayMs(paramA);
            masterTapeDelays[slotIndex].setFeedback(paramB);
            masterTapeDelays[slotIndex].setSaturation(paramC);
            masterTapeDelays[slotIndex].setMix(0.35f);
            masterTapeDelays[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::PingPongDelay:
            masterPingPongDelays[slotIndex].setDelayMs(paramA);
            masterPingPongDelays[slotIndex].setFeedback(paramB);
            masterPingPongDelays[slotIndex].setMix(paramC);
            masterPingPongDelays[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CloudGenerator:
            masterCloudGenerators[slotIndex].setGrainSizeMs(paramA);
            masterCloudGenerators[slotIndex].setDensity(paramB);
            masterCloudGenerators[slotIndex].setMix(paramC);
            masterCloudGenerators[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SpectralBlur:
            masterSpectralBlurs[slotIndex].setBlurTimeMs(paramA);
            masterSpectralBlurs[slotIndex].setFrequencyRange(paramB);
            masterSpectralBlurs[slotIndex].setMix(paramC);
            masterSpectralBlurs[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::SpectralDelay:
            masterSpectralDelays[slotIndex].setDelayMs(paramA);
            masterSpectralDelays[slotIndex].setFeedback(paramB);
            masterSpectralDelays[slotIndex].setMix(paramC);
            masterSpectralDelays[slotIndex].setActive(true);
            break;
        case EffectPresetCatalog::EffectType::CompositeCategory:
            break;
    }
}

void EffectProcessor::processTrackEffectType(uint8_t trackIndex, uint8_t slotIndex, EffectPresetCatalog::EffectType type, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm)
{
    switch (type)
    {
        case EffectPresetCatalog::EffectType::None:
            break;
        case EffectPresetCatalog::EffectType::TapeStop:
            tapeStops[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeat:
            beatRepeats[trackIndex][slotIndex].setBpm(bpm);
            beatRepeats[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::HighPassSweep:
            highPassSweeps[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ReverbWash:
            reverbWashes[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ReduxCrush:
            reduxCrushers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::DelayEcho:
            delayEchos[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SaturationWaveshaper:
            saturators[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SoftHardClip:
            softHardClips[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Wavefolder:
            wavefolders[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::AsymShaper:
            asymShapers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CompressorGlue:
            compressors[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PeakLimiter:
            limiters[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TransientShaper:
            transientShapers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CombFilter:
            combFilters[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::MultiModeEQ:
            multiModeEqs[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::FormantFilter:
            formantFilters[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Autopan:
            autopans[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::RingModulator:
            ringModulators[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Chorus:
            choruses[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Phaser:
            phasers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Flanger:
            flangers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::JitterDegrade:
            jitterDegrades[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ErosionDegrade:
            erosionDegrades[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TranceGate:
            tranceGates[trackIndex][slotIndex].setBpm(bpm);
            tranceGates[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SidechainDucker:
            sidechainDuckers[trackIndex][slotIndex].setBpm(bpm);
            sidechainDuckers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeatInsert:
            beatRepeatInserts[trackIndex][slotIndex].setBpm(bpm);
            beatRepeatInserts[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::FrequencyShifter:
            frequencyShifters[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PitchShifter:
            pitchShifters[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Harmonizer:
            harmonizers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TimeFreezer:
            timeFreezers[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::GrainDelay:
            grainDelays[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PhysicalModelingResonator:
            physicalModelingResonators[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::MultibandOtt:
            multibandOtts[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ConvolutionReverb:
            convolutionReverbs[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TapeDelay:
            tapeDelays[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PingPongDelay:
            pingPongDelays[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CloudGenerator:
            cloudGenerators[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SpectralBlur:
            spectralBlurs[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SpectralDelay:
            spectralDelays[trackIndex][slotIndex].processBlock(trackBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CompositeCategory:
            processTrackCompositePreset(trackIndex, slotIndex, trackBuffer, numSamples, bpm);
            break;
    }
}

void EffectProcessor::processMasterEffectType(uint8_t slotIndex, EffectPresetCatalog::EffectType type, juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm)
{
    switch (type)
    {
        case EffectPresetCatalog::EffectType::None:
            break;
        case EffectPresetCatalog::EffectType::TapeStop:
            masterTapeStops[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeat:
            masterBeatRepeats[slotIndex].setBpm(bpm);
            masterBeatRepeats[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::HighPassSweep:
            masterHighPassSweeps[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ReverbWash:
            masterReverbWashes[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ReduxCrush:
            masterReduxCrushers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::DelayEcho:
            masterDelayEchos[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SaturationWaveshaper:
            masterSaturators[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SoftHardClip:
            masterSoftHardClips[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Wavefolder:
            masterWavefolders[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::AsymShaper:
            masterAsymShapers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CompressorGlue:
            masterCompressors[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PeakLimiter:
            masterLimiters[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TransientShaper:
            masterTransientShapers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CombFilter:
            masterCombFilters[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::MultiModeEQ:
            masterMultiModeEqs[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::FormantFilter:
            masterFormantFilters[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Autopan:
            masterAutopans[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::RingModulator:
            masterRingModulators[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Chorus:
            masterChoruses[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Phaser:
            masterPhasers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Flanger:
            masterFlangers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::JitterDegrade:
            masterJitterDegrades[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ErosionDegrade:
            masterErosionDegrades[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TranceGate:
            masterTranceGates[slotIndex].setBpm(bpm);
            masterTranceGates[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SidechainDucker:
            masterSidechainDuckers[slotIndex].setBpm(bpm);
            masterSidechainDuckers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::BeatRepeatInsert:
            masterBeatRepeatInserts[slotIndex].setBpm(bpm);
            masterBeatRepeatInserts[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::FrequencyShifter:
            masterFrequencyShifters[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PitchShifter:
            masterPitchShifters[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::Harmonizer:
            masterHarmonizers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TimeFreezer:
            masterTimeFreezers[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::GrainDelay:
            masterGrainDelays[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PhysicalModelingResonator:
            masterPhysicalModelingResonators[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::MultibandOtt:
            masterMultibandOtts[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::ConvolutionReverb:
            masterConvolutionReverbs[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::TapeDelay:
            masterTapeDelays[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::PingPongDelay:
            masterPingPongDelays[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CloudGenerator:
            masterCloudGenerators[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SpectralBlur:
            masterSpectralBlurs[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::SpectralDelay:
            masterSpectralDelays[slotIndex].processBlock(masterBuffer, numSamples);
            break;
        case EffectPresetCatalog::EffectType::CompositeCategory:
            processMasterCompositePreset(slotIndex, masterBuffer, numSamples, bpm);
            break;
    }
}

void EffectProcessor::processTrackCompositePreset(uint8_t trackIndex, uint8_t slotIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm)
{
    const auto& state = trackCompositeStates[trackIndex][slotIndex];
    if (state.componentCount == 0)
        return;

    if (state.routing == EffectPresetCatalog::CompositeRouting::Serial)
    {
        for (uint8_t i = 0; i < state.componentCount; ++i)
        {
            const auto& component = state.components[i];
            if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
                continue;
            processTrackEffectType(trackIndex, slotIndex, component.type, trackBuffer, numSamples, bpm);
        }
        return;
    }

    const int channels = trackBuffer.getNumChannels();
    float levelSum = 0.0f;
    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;
        levelSum += std::max(0.0f, component.level);
    }
    if (levelSum <= 0.0001f)
        levelSum = 1.0f;

    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        auto& workBuffer = compositeWorkBuffers[i];
        workBuffer.clear();
        for (int ch = 0; ch < channels; ++ch)
            workBuffer.copyFrom(ch, 0, trackBuffer, ch, 0, numSamples);

        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;
        processTrackEffectType(trackIndex, slotIndex, component.type, workBuffer, numSamples, bpm);
    }

    trackBuffer.clear();
    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;

        const float gain = std::max(0.0f, component.level) / levelSum;
        auto& workBuffer = compositeWorkBuffers[i];
        for (int ch = 0; ch < channels; ++ch)
            trackBuffer.addFrom(ch, 0, workBuffer, ch, 0, numSamples, gain);
    }
}

void EffectProcessor::processMasterCompositePreset(uint8_t slotIndex, juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm)
{
    const auto& state = masterCompositeStates[slotIndex];
    if (state.componentCount == 0)
        return;

    if (state.routing == EffectPresetCatalog::CompositeRouting::Serial)
    {
        for (uint8_t i = 0; i < state.componentCount; ++i)
        {
            const auto& component = state.components[i];
            if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
                continue;
            processMasterEffectType(slotIndex, component.type, masterBuffer, numSamples, bpm);
        }
        return;
    }

    const int channels = masterBuffer.getNumChannels();
    float levelSum = 0.0f;
    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;
        levelSum += std::max(0.0f, component.level);
    }
    if (levelSum <= 0.0001f)
        levelSum = 1.0f;

    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        auto& workBuffer = compositeWorkBuffers[i];
        workBuffer.clear();
        for (int ch = 0; ch < channels; ++ch)
            workBuffer.copyFrom(ch, 0, masterBuffer, ch, 0, numSamples);

        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;
        processMasterEffectType(slotIndex, component.type, workBuffer, numSamples, bpm);
    }

    masterBuffer.clear();
    for (uint8_t i = 0; i < state.componentCount; ++i)
    {
        const auto& component = state.components[i];
        if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
            continue;

        const float gain = std::max(0.0f, component.level) / levelSum;
        auto& workBuffer = compositeWorkBuffers[i];
        for (int ch = 0; ch < channels; ++ch)
            masterBuffer.addFrom(ch, 0, workBuffer, ch, 0, numSamples, gain);
    }
}

void EffectProcessor::applyTrackEffectPreset(uint8_t trackIndex, uint8_t slotIndex, uint16_t presetId)
{
    if (trackIndex >= TrackCount || slotIndex >= EffectSlotCount)
        return;

    const bool isCustomPreset = presetId >= PresetRegistry::kCustomEffectPresetIdBase
        && PresetRegistry::resolvePublishedCustomEffect(presetId).has_value();
    const uint16_t normalizedPresetId = (EffectPresetCatalog::isPresetAssignableToSlot(presetId) || isCustomPreset) ? presetId : 0;
    if (cachedTrackEffectPresets[trackIndex][slotIndex] == normalizedPresetId)
        return;

    cachedTrackEffectPresets[trackIndex][slotIndex] = normalizedPresetId;
    appState.tracks[trackIndex].setEffectPresetSlot(slotIndex, normalizedPresetId);
    trackCompositeStates[trackIndex][slotIndex] = CompositeSlotState{};

    auto& tapeStop = tapeStops[trackIndex][slotIndex];
    auto& beatRepeat = beatRepeats[trackIndex][slotIndex];
    auto& highPassSweep = highPassSweeps[trackIndex][slotIndex];
    auto& reverbWash = reverbWashes[trackIndex][slotIndex];
    auto& reduxCrush = reduxCrushers[trackIndex][slotIndex];
    auto& delayEcho = delayEchos[trackIndex][slotIndex];
    auto& saturator = saturators[trackIndex][slotIndex];
    auto& softHardClip = softHardClips[trackIndex][slotIndex];
    auto& wavefolder = wavefolders[trackIndex][slotIndex];
    auto& asymShaper = asymShapers[trackIndex][slotIndex];
    auto& compressor = compressors[trackIndex][slotIndex];
    auto& limiter = limiters[trackIndex][slotIndex];
    auto& transientShaper = transientShapers[trackIndex][slotIndex];
    auto& combFilter = combFilters[trackIndex][slotIndex];
    auto& multiModeEq = multiModeEqs[trackIndex][slotIndex];
    auto& formantFilter = formantFilters[trackIndex][slotIndex];
    auto& autopan = autopans[trackIndex][slotIndex];
    auto& ringModulator = ringModulators[trackIndex][slotIndex];
    auto& chorus = choruses[trackIndex][slotIndex];
    auto& phaser = phasers[trackIndex][slotIndex];
    auto& flanger = flangers[trackIndex][slotIndex];
    auto& jitterDegrade = jitterDegrades[trackIndex][slotIndex];
    auto& erosionDegrade = erosionDegrades[trackIndex][slotIndex];
    auto& tranceGate = tranceGates[trackIndex][slotIndex];
    auto& sidechainDucker = sidechainDuckers[trackIndex][slotIndex];
    auto& beatRepeatInsert = beatRepeatInserts[trackIndex][slotIndex];
    auto& frequencyShifter = frequencyShifters[trackIndex][slotIndex];
    auto& pitchShifter = pitchShifters[trackIndex][slotIndex];
    auto& harmonizer = harmonizers[trackIndex][slotIndex];
    auto& timeFreezer = timeFreezers[trackIndex][slotIndex];
    auto& grainDelay = grainDelays[trackIndex][slotIndex];
    auto& physicalModelingResonator = physicalModelingResonators[trackIndex][slotIndex];

    tapeStop.setActive(false);
    beatRepeat.setActive(false);
    highPassSweep.setActive(false);
    reverbWash.setActive(false);
    reduxCrush.setActive(false);
    delayEcho.setActive(false);
    saturator.setActive(false);
    softHardClip.setActive(false);
    wavefolder.setActive(false);
    asymShaper.setActive(false);
    compressor.setActive(false);
    limiter.setActive(false);
    limiter.setActive(false);
    transientShaper.setActive(false);
    combFilter.setActive(false);
    multiModeEq.setActive(false);
    multiModeEq.setActive(false);
    formantFilter.setActive(false);
    autopan.setActive(false);
    ringModulator.setActive(false);
    ringModulator.setActive(false);
    chorus.setActive(false);
    phaser.setActive(false);
    flanger.setActive(false);
    jitterDegrade.setActive(false);
    erosionDegrade.setActive(false);
    tranceGate.setActive(false);
    sidechainDucker.setActive(false);
    beatRepeatInsert.setActive(false);
    frequencyShifter.setActive(false);
    frequencyShifter.setActive(false);
    pitchShifter.setActive(false);
    harmonizer.setActive(false);
    timeFreezer.setActive(false);
    timeFreezer.setActive(false);
    grainDelay.setActive(false);
    physicalModelingResonator.setActive(false);
    masterMultibandOtts[slotIndex].setActive(false);
    masterConvolutionReverbs[slotIndex].setActive(false);
    masterTapeDelays[slotIndex].setActive(false);
    masterPingPongDelays[slotIndex].setActive(false);
    masterCloudGenerators[slotIndex].setActive(false);
    masterSpectralBlurs[slotIndex].setActive(false);
    masterSpectralDelays[slotIndex].setActive(false);

    tapeStop.reset();
    beatRepeat.reset();
    highPassSweep.reset();
    reverbWash.reset();
    reduxCrush.reset();
    delayEcho.reset();
    saturator.reset();
    softHardClip.reset();
    wavefolder.reset();
    asymShaper.reset();
    compressor.reset();
    limiter.reset();
    limiter.reset();
    transientShaper.reset();
    combFilter.reset();
    multiModeEq.reset();
    multiModeEq.reset();
    formantFilter.reset();
    autopan.reset();
    ringModulator.reset();
    ringModulator.reset();
    chorus.reset();
    phaser.reset();
    flanger.reset();
    jitterDegrade.reset();
    erosionDegrade.reset();
    tranceGate.reset();
    sidechainDucker.reset();
    beatRepeatInsert.reset();
    frequencyShifter.reset();
    frequencyShifter.reset();
    pitchShifter.reset();
    harmonizer.reset();
    timeFreezer.reset();
    timeFreezer.reset();
    grainDelay.reset();
    physicalModelingResonator.reset();
    multibandOtts[trackIndex][slotIndex].setActive(false);
    multibandOtts[trackIndex][slotIndex].reset();
    convolutionReverbs[trackIndex][slotIndex].setActive(false);
    convolutionReverbs[trackIndex][slotIndex].reset();
    tapeDelays[trackIndex][slotIndex].setActive(false);
    tapeDelays[trackIndex][slotIndex].reset();
    pingPongDelays[trackIndex][slotIndex].setActive(false);
    pingPongDelays[trackIndex][slotIndex].reset();
    cloudGenerators[trackIndex][slotIndex].setActive(false);
    cloudGenerators[trackIndex][slotIndex].reset();
    spectralBlurs[trackIndex][slotIndex].setActive(false);
    spectralBlurs[trackIndex][slotIndex].reset();
    spectralDelays[trackIndex][slotIndex].setActive(false);
    spectralDelays[trackIndex][slotIndex].reset();

    if (EffectPresetCatalog::isValidCompositePresetId(normalizedPresetId))
    {
        const auto& composite = EffectPresetCatalog::getCompositePresetById(normalizedPresetId);
        trackEffectTypes[trackIndex][slotIndex] = EffectPresetCatalog::EffectType::CompositeCategory;

        auto& state = trackCompositeStates[trackIndex][slotIndex];
        state.routing = composite.routing;
        state.componentCount = static_cast<uint8_t>(std::min<uint8_t>(composite.componentCount, EffectPresetCatalog::kCompositeMaxComponents));

        for (uint8_t i = 0; i < state.componentCount; ++i)
        {
            state.components[i] = composite.components[i];
            const auto& component = state.components[i];
            if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
                continue;

            configureTrackEffectFromParams(trackIndex, slotIndex, component.type, component.paramA, component.paramB, component.paramC, component.inputGainDb, component.outputGainDb);
        }
        return;
    }

    if (normalizedPresetId >= PresetRegistry::kCustomEffectPresetIdBase)
    {
        if (const auto custom = PresetRegistry::resolvePublishedCustomEffect(normalizedPresetId))
        {
            trackEffectTypes[trackIndex][slotIndex] = custom->type;
            configureTrackEffectFromParams(trackIndex, slotIndex, custom->type, custom->paramA, custom->paramB, custom->paramC);
            return;
        }
    }

    const auto preset = EffectPresetCatalog::getPresetById(normalizedPresetId);
    trackEffectTypes[trackIndex][slotIndex] = preset.type;
    configureTrackEffectFromParams(trackIndex, slotIndex, preset.type, preset.paramA, preset.paramB, preset.paramC, preset.inputGainDb, preset.outputGainDb, preset.irResourceName);
}

void EffectProcessor::applyMasterEffectPreset(uint8_t slotIndex, uint16_t presetId)
{
    if (slotIndex >= EffectSlotCount)
        return;

    const bool isCustomPreset = presetId >= PresetRegistry::kCustomEffectPresetIdBase
        && PresetRegistry::resolvePublishedCustomEffect(presetId).has_value();
    const uint16_t normalizedPresetId = (EffectPresetCatalog::isPresetAssignableToSlot(presetId) || isCustomPreset) ? presetId : 0;
    if (cachedMasterEffectPresets[slotIndex] == normalizedPresetId)
        return;

    cachedMasterEffectPresets[slotIndex] = normalizedPresetId;
    appState.master.setEffectPresetSlot(slotIndex, normalizedPresetId);
    masterCompositeStates[slotIndex] = CompositeSlotState{};

    auto& tapeStop = masterTapeStops[slotIndex];
    auto& beatRepeat = masterBeatRepeats[slotIndex];
    auto& highPassSweep = masterHighPassSweeps[slotIndex];
    auto& reverbWash = masterReverbWashes[slotIndex];
    auto& reduxCrush = masterReduxCrushers[slotIndex];
    auto& delayEcho = masterDelayEchos[slotIndex];
    auto& saturator = masterSaturators[slotIndex];
    auto& softHardClip = masterSoftHardClips[slotIndex];
    auto& wavefolder = masterWavefolders[slotIndex];
    auto& asymShaper = masterAsymShapers[slotIndex];
    auto& compressor = masterCompressors[slotIndex];
    auto& limiter = masterLimiters[slotIndex];
    auto& transientShaper = masterTransientShapers[slotIndex];
    auto& combFilter = masterCombFilters[slotIndex];
    auto& multiModeEq = masterMultiModeEqs[slotIndex];
    auto& formantFilter = masterFormantFilters[slotIndex];
    auto& autopan = masterAutopans[slotIndex];
    auto& ringModulator = masterRingModulators[slotIndex];
    auto& chorus = masterChoruses[slotIndex];
    auto& phaser = masterPhasers[slotIndex];
    auto& flanger = masterFlangers[slotIndex];
    auto& jitterDegrade = masterJitterDegrades[slotIndex];
    auto& erosionDegrade = masterErosionDegrades[slotIndex];
    auto& tranceGate = masterTranceGates[slotIndex];
    auto& sidechainDucker = masterSidechainDuckers[slotIndex];
    auto& beatRepeatInsert = masterBeatRepeatInserts[slotIndex];
    auto& frequencyShifter = masterFrequencyShifters[slotIndex];
    auto& pitchShifter = masterPitchShifters[slotIndex];
    auto& harmonizer = masterHarmonizers[slotIndex];
    auto& timeFreezer = masterTimeFreezers[slotIndex];
    auto& grainDelay = masterGrainDelays[slotIndex];
    auto& physicalModelingResonator = masterPhysicalModelingResonators[slotIndex];

    tapeStop.setActive(false);
    beatRepeat.setActive(false);
    highPassSweep.setActive(false);
    reverbWash.setActive(false);
    reduxCrush.setActive(false);
    delayEcho.setActive(false);
    saturator.setActive(false);
    softHardClip.setActive(false);
    wavefolder.setActive(false);
    asymShaper.setActive(false);
    compressor.setActive(false);
    limiter.setActive(false);
    limiter.setActive(false);
    transientShaper.setActive(false);
    combFilter.setActive(false);
    multiModeEq.setActive(false);
    multiModeEq.setActive(false);
    formantFilter.setActive(false);
    autopan.setActive(false);
    ringModulator.setActive(false);
    ringModulator.setActive(false);
    chorus.setActive(false);
    phaser.setActive(false);
    flanger.setActive(false);
    jitterDegrade.setActive(false);
    erosionDegrade.setActive(false);
    tranceGate.setActive(false);
    sidechainDucker.setActive(false);
    beatRepeatInsert.setActive(false);
    frequencyShifter.setActive(false);
    frequencyShifter.setActive(false);
    pitchShifter.setActive(false);
    harmonizer.setActive(false);
    timeFreezer.setActive(false);
    timeFreezer.setActive(false);
    grainDelay.setActive(false);
    physicalModelingResonator.setActive(false);

    tapeStop.reset();
    beatRepeat.reset();
    highPassSweep.reset();
    reverbWash.reset();
    reduxCrush.reset();
    delayEcho.reset();
    saturator.reset();
    softHardClip.reset();
    wavefolder.reset();
    asymShaper.reset();
    compressor.reset();
    limiter.reset();
    limiter.reset();
    transientShaper.reset();
    combFilter.reset();
    multiModeEq.reset();
    multiModeEq.reset();
    formantFilter.reset();
    autopan.reset();
    ringModulator.reset();
    ringModulator.reset();
    chorus.reset();
    phaser.reset();
    flanger.reset();
    jitterDegrade.reset();
    erosionDegrade.reset();
    tranceGate.reset();
    sidechainDucker.reset();
    beatRepeatInsert.reset();
    frequencyShifter.reset();
    frequencyShifter.reset();
    pitchShifter.reset();
    harmonizer.reset();
    timeFreezer.reset();
    timeFreezer.reset();
    grainDelay.reset();
    physicalModelingResonator.reset();
    masterMultibandOtts[slotIndex].reset();
    masterConvolutionReverbs[slotIndex].reset();
    masterTapeDelays[slotIndex].reset();
    masterPingPongDelays[slotIndex].reset();
    masterCloudGenerators[slotIndex].reset();
    masterSpectralBlurs[slotIndex].reset();
    masterSpectralDelays[slotIndex].reset();

    if (EffectPresetCatalog::isValidCompositePresetId(normalizedPresetId))
    {
        const auto& composite = EffectPresetCatalog::getCompositePresetById(normalizedPresetId);
        masterEffectTypes[slotIndex] = EffectPresetCatalog::EffectType::CompositeCategory;

        auto& state = masterCompositeStates[slotIndex];
        state.routing = composite.routing;
        state.componentCount = static_cast<uint8_t>(std::min<uint8_t>(composite.componentCount, EffectPresetCatalog::kCompositeMaxComponents));

        for (uint8_t i = 0; i < state.componentCount; ++i)
        {
            state.components[i] = composite.components[i];
            const auto& component = state.components[i];
            if (component.type == EffectPresetCatalog::EffectType::None || component.type == EffectPresetCatalog::EffectType::CompositeCategory)
                continue;

            configureMasterEffectFromParams(slotIndex, component.type, component.paramA, component.paramB, component.paramC, component.inputGainDb, component.outputGainDb);
        }
        return;
    }

    if (normalizedPresetId >= PresetRegistry::kCustomEffectPresetIdBase)
    {
        if (const auto custom = PresetRegistry::resolvePublishedCustomEffect(normalizedPresetId))
        {
            masterEffectTypes[slotIndex] = custom->type;
            configureMasterEffectFromParams(slotIndex, custom->type, custom->paramA, custom->paramB, custom->paramC);
            return;
        }
    }

    const auto preset = EffectPresetCatalog::getPresetById(normalizedPresetId);
    masterEffectTypes[slotIndex] = preset.type;
    configureMasterEffectFromParams(slotIndex, preset.type, preset.paramA, preset.paramB, preset.paramC, preset.inputGainDb, preset.outputGainDb, preset.irResourceName);
}

void EffectProcessor::processTrackInsertEffects(uint8_t trackIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm)
{
    if (trackIndex >= TrackCount)
        return;

    for (uint8_t slot = 0; slot < EffectSlotCount; ++slot)
    {
        processTrackEffectType(trackIndex, slot, trackEffectTypes[trackIndex][slot], trackBuffer, numSamples, bpm);
    }
}

void EffectProcessor::processMasterEffects(juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm)
{
    for (uint8_t slot = 0; slot < EffectSlotCount; ++slot)
    {
        processMasterEffectType(slot, masterEffectTypes[slot], masterBuffer, numSamples, bpm);
    }
}

void EffectProcessor::applySpotEffectsBitmask(uint8_t activeSpotEffectsMask)
{
    const uint8_t normalizedMask = static_cast<uint8_t>(activeSpotEffectsMask & SpotEffectCatalog::getSupportedBitmask());
    if (normalizedMask != activeSpotEffectsMask)
        appState.setActiveSpotEffects(normalizedMask);

    if (cachedSpotEffectsMask == normalizedMask)
        return;

    cachedSpotEffectsMask = normalizedMask;
    const uint8_t tapeBrakeBit = SpotEffectCatalog::getBitMask(Command::SpotEffectId::TapeBrake);
    const uint8_t stutterBit = SpotEffectCatalog::getBitMask(Command::SpotEffectId::Stutter);
    spotTapeStop.setActive((normalizedMask & tapeBrakeBit) != 0);
    spotBeatRepeat.setActive((normalizedMask & stutterBit) != 0);
}

void EffectProcessor::processSpotEffects(juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm)
{
    spotBeatRepeat.setBpm(bpm);
    spotBeatRepeat.processBlock(masterBuffer, numSamples);

    if (spotTapeStop.isActive())
        spotTapeStop.processBlock(masterBuffer, numSamples);
}
