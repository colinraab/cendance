#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

#include <juce_audio_basics/juce_audio_basics.h>

#include "../App/AppState.h"
#include "../App/EffectPresetCatalog.h"
#include "Effects/Spot/TapeStop.h"
#include "Effects/Spot/BeatRepeat.h"
#include "Effects/0_Dynamics/CompressorGlue.h"
#include "Effects/0_Dynamics/PeakLimiter.h"
#include "Effects/0_Dynamics/TransientShaper.h"
#include "Effects/1_Space/DelayEcho.h"
#include "Effects/1_Space/ReverbWash.h"
#include "Effects/2_Distortion/AsymShaper.h"
#include "Effects/2_Distortion/SaturationWaveshaper.h"
#include "Effects/2_Distortion/SoftHardClip.h"
#include "Effects/2_Distortion/Wavefolder.h"
#include "Effects/3_Filters/CombFilter.h"
#include "Effects/3_Filters/FormantFilter.h"
#include "Effects/3_Filters/HighPassSweep.h"
#include "Effects/3_Filters/MultiModeEQ.h"
#include "Effects/4_Modulation/Chorus.h"
#include "Effects/4_Modulation/Flanger.h"
#include "Effects/4_Modulation/Phaser.h"
#include "Effects/4_Modulation/RingModulator.h"
#include "Effects/5_Pitch/FrequencyShifter.h"
#include "Effects/5_Pitch/Harmonizer.h"
#include "Effects/5_Pitch/PitchShifter.h"
#include "Effects/6_Degrade/ErosionDegrade.h"
#include "Effects/6_Degrade/JitterDegrade.h"
#include "Effects/6_Degrade/ReduxCrush.h"
#include "Effects/7_Rhythm/Autopan.h"
#include "Effects/7_Rhythm/BeatRepeatInsert.h"
#include "Effects/7_Rhythm/SidechainDucker.h"
#include "Effects/7_Rhythm/TranceGate.h"
#include "Effects/8_Granular/GrainDelay.h"
#include "Effects/8_Granular/TimeFreezer.h"
#include "Effects/9_SpectralResonators/PhysicalModelingResonator.h"
#include "Effects/0_Dynamics/MultibandOtt.h"
#include "Effects/1_Space/ConvolutionReverb.h"
#include "Effects/1_Space/TapeDelay.h"
#include "Effects/1_Space/PingPongDelay.h"
#include "Effects/8_Granular/CloudGenerator.h"
#include "Effects/9_SpectralResonators/SpectralBlur.h"
#include "Effects/9_SpectralResonators/SpectralDelay.h"

class EffectProcessor final
{
public:
    static constexpr uint8_t TrackCount = AppState::kTrackCount;
    static constexpr uint8_t EffectSlotCount = 3;

    explicit EffectProcessor(AppState& appState);

    void prepare(double sampleRate, int maximumBlockSize);

    void applyTrackEffectPreset(uint8_t trackIndex, uint8_t slotIndex, uint16_t presetId);
    void applyMasterEffectPreset(uint8_t slotIndex, uint16_t presetId);
    void processTrackInsertEffects(uint8_t trackIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm);
    void processMasterEffects(juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm);
    void applySpotEffectsBitmask(uint8_t activeSpotEffectsMask);
    void processSpotEffects(juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm);

private:
    struct CompositeSlotState
    {
        EffectPresetCatalog::CompositeRouting routing = EffectPresetCatalog::CompositeRouting::Serial;
        uint8_t componentCount = 0;
        std::array<EffectPresetCatalog::CompositeComponent, EffectPresetCatalog::kCompositeMaxComponents> components{};
    };

    void configureTrackEffectFromParams(uint8_t trackIndex, uint8_t slotIndex, EffectPresetCatalog::EffectType type, float paramA, float paramB, float paramC, float inputGainDb = 0.0f, float outputGainDb = 0.0f, std::string_view irResourceName = {});
    void configureMasterEffectFromParams(uint8_t slotIndex, EffectPresetCatalog::EffectType type, float paramA, float paramB, float paramC, float inputGainDb = 0.0f, float outputGainDb = 0.0f, std::string_view irResourceName = {});
    void processTrackEffectType(uint8_t trackIndex, uint8_t slotIndex, EffectPresetCatalog::EffectType type, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm);
    void processMasterEffectType(uint8_t slotIndex, EffectPresetCatalog::EffectType type, juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm);
    void processTrackCompositePreset(uint8_t trackIndex, uint8_t slotIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples, float bpm);
    void processMasterCompositePreset(uint8_t slotIndex, juce::AudioBuffer<float>& masterBuffer, int numSamples, float bpm);

    AppState& appState;

    std::array<std::array<uint16_t, EffectSlotCount>, TrackCount> cachedTrackEffectPresets{};
    std::array<std::array<EffectPresetCatalog::EffectType, EffectSlotCount>, TrackCount> trackEffectTypes{};
    std::array<std::array<CompositeSlotState, EffectSlotCount>, TrackCount> trackCompositeStates{};
    std::array<std::array<TapeStop, EffectSlotCount>, TrackCount> tapeStops;
    std::array<std::array<BeatRepeat, EffectSlotCount>, TrackCount> beatRepeats;
    std::array<std::array<HighPassSweep, EffectSlotCount>, TrackCount> highPassSweeps;
    std::array<std::array<ReverbWash, EffectSlotCount>, TrackCount> reverbWashes;
    std::array<std::array<ReduxCrush, EffectSlotCount>, TrackCount> reduxCrushers;
    std::array<std::array<DelayEcho, EffectSlotCount>, TrackCount> delayEchos;
    std::array<std::array<SaturationWaveshaper, EffectSlotCount>, TrackCount> saturators;
    std::array<std::array<SoftHardClip, EffectSlotCount>, TrackCount> softHardClips;
    std::array<std::array<Wavefolder, EffectSlotCount>, TrackCount> wavefolders;
    std::array<std::array<AsymShaper, EffectSlotCount>, TrackCount> asymShapers;
    std::array<std::array<CompressorGlue, EffectSlotCount>, TrackCount> compressors;
    std::array<std::array<PeakLimiter, EffectSlotCount>, TrackCount> limiters;
    std::array<std::array<TransientShaper, EffectSlotCount>, TrackCount> transientShapers;
    std::array<std::array<CombFilter, EffectSlotCount>, TrackCount> combFilters;
    std::array<std::array<MultiModeEQ, EffectSlotCount>, TrackCount> multiModeEqs;
    std::array<std::array<FormantFilter, EffectSlotCount>, TrackCount> formantFilters;
    std::array<std::array<Autopan, EffectSlotCount>, TrackCount> autopans;
    std::array<std::array<RingModulator, EffectSlotCount>, TrackCount> ringModulators;
    std::array<std::array<Chorus, EffectSlotCount>, TrackCount> choruses;
    std::array<std::array<Phaser, EffectSlotCount>, TrackCount> phasers;
    std::array<std::array<Flanger, EffectSlotCount>, TrackCount> flangers;
    std::array<std::array<JitterDegrade, EffectSlotCount>, TrackCount> jitterDegrades;
    std::array<std::array<ErosionDegrade, EffectSlotCount>, TrackCount> erosionDegrades;
    std::array<std::array<TranceGate, EffectSlotCount>, TrackCount> tranceGates;
    std::array<std::array<SidechainDucker, EffectSlotCount>, TrackCount> sidechainDuckers;
    std::array<std::array<BeatRepeatInsert, EffectSlotCount>, TrackCount> beatRepeatInserts;
    std::array<std::array<FrequencyShifter, EffectSlotCount>, TrackCount> frequencyShifters;
    std::array<std::array<PitchShifter, EffectSlotCount>, TrackCount> pitchShifters;
    std::array<std::array<Harmonizer, EffectSlotCount>, TrackCount> harmonizers;
    std::array<std::array<TimeFreezer, EffectSlotCount>, TrackCount> timeFreezers;
    std::array<std::array<GrainDelay, EffectSlotCount>, TrackCount> grainDelays;
    std::array<std::array<PhysicalModelingResonator, EffectSlotCount>, TrackCount> physicalModelingResonators;
    std::array<std::array<MultibandOtt, EffectSlotCount>, TrackCount> multibandOtts;
    std::array<std::array<ConvolutionReverb, EffectSlotCount>, TrackCount> convolutionReverbs;
    std::array<std::array<TapeDelay, EffectSlotCount>, TrackCount> tapeDelays;
    std::array<std::array<PingPongDelay, EffectSlotCount>, TrackCount> pingPongDelays;
    std::array<std::array<CloudGenerator, EffectSlotCount>, TrackCount> cloudGenerators;
    std::array<std::array<SpectralBlur, EffectSlotCount>, TrackCount> spectralBlurs;
    std::array<std::array<SpectralDelay, EffectSlotCount>, TrackCount> spectralDelays;

    std::array<uint16_t, EffectSlotCount> cachedMasterEffectPresets{};
    std::array<EffectPresetCatalog::EffectType, EffectSlotCount> masterEffectTypes{};
    std::array<CompositeSlotState, EffectSlotCount> masterCompositeStates{};
    std::array<TapeStop, EffectSlotCount> masterTapeStops;
    std::array<BeatRepeat, EffectSlotCount> masterBeatRepeats;
    std::array<HighPassSweep, EffectSlotCount> masterHighPassSweeps;
    std::array<ReverbWash, EffectSlotCount> masterReverbWashes;
    std::array<ReduxCrush, EffectSlotCount> masterReduxCrushers;
    std::array<DelayEcho, EffectSlotCount> masterDelayEchos;
    std::array<SaturationWaveshaper, EffectSlotCount> masterSaturators;
    std::array<SoftHardClip, EffectSlotCount> masterSoftHardClips;
    std::array<Wavefolder, EffectSlotCount> masterWavefolders;
    std::array<AsymShaper, EffectSlotCount> masterAsymShapers;
    std::array<CompressorGlue, EffectSlotCount> masterCompressors;
    std::array<PeakLimiter, EffectSlotCount> masterLimiters;
    std::array<TransientShaper, EffectSlotCount> masterTransientShapers;
    std::array<CombFilter, EffectSlotCount> masterCombFilters;
    std::array<MultiModeEQ, EffectSlotCount> masterMultiModeEqs;
    std::array<FormantFilter, EffectSlotCount> masterFormantFilters;
    std::array<Autopan, EffectSlotCount> masterAutopans;
    std::array<RingModulator, EffectSlotCount> masterRingModulators;
    std::array<Chorus, EffectSlotCount> masterChoruses;
    std::array<Phaser, EffectSlotCount> masterPhasers;
    std::array<Flanger, EffectSlotCount> masterFlangers;
    std::array<JitterDegrade, EffectSlotCount> masterJitterDegrades;
    std::array<ErosionDegrade, EffectSlotCount> masterErosionDegrades;
    std::array<TranceGate, EffectSlotCount> masterTranceGates;
    std::array<SidechainDucker, EffectSlotCount> masterSidechainDuckers;
    std::array<BeatRepeatInsert, EffectSlotCount> masterBeatRepeatInserts;
    std::array<FrequencyShifter, EffectSlotCount> masterFrequencyShifters;
    std::array<PitchShifter, EffectSlotCount> masterPitchShifters;
    std::array<Harmonizer, EffectSlotCount> masterHarmonizers;
    std::array<TimeFreezer, EffectSlotCount> masterTimeFreezers;
    std::array<GrainDelay, EffectSlotCount> masterGrainDelays;
    std::array<PhysicalModelingResonator, EffectSlotCount> masterPhysicalModelingResonators;
    std::array<MultibandOtt, EffectSlotCount> masterMultibandOtts;
    std::array<ConvolutionReverb, EffectSlotCount> masterConvolutionReverbs;
    std::array<TapeDelay, EffectSlotCount> masterTapeDelays;
    std::array<PingPongDelay, EffectSlotCount> masterPingPongDelays;
    std::array<CloudGenerator, EffectSlotCount> masterCloudGenerators;
    std::array<SpectralBlur, EffectSlotCount> masterSpectralBlurs;
    std::array<SpectralDelay, EffectSlotCount> masterSpectralDelays;

    TapeStop spotTapeStop;
    BeatRepeat spotBeatRepeat;
    uint8_t cachedSpotEffectsMask = 0xFFu;
    std::array<juce::AudioBuffer<float>, EffectPresetCatalog::kCompositeMaxComponents> compositeWorkBuffers;
};
