#include "AudioEngine.h"
#include "Harmony/ChordProgression.h"
#include "../App/AlgorithmCatalog.h"
#include "../App/DrumKitPresetCatalog.h"
#include "../App/GenreCatalog.h"
#include "../App/PresetRegistry.h"
#include "../App/SpotEffectCatalog.h"
#include "../App/SynthCatalog.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

constexpr uint32_t kDefaultProfileIntervalCallbacks = 128;
constexpr float kDrumTrackTrimLinear = 0.354813f; // -9 dB

uint32_t parseProfileInterval(const char* value, uint32_t fallback) {
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(value, &end, 10);
    if (end == value || (end != nullptr && *end != '\0')) {
        return fallback;
    }

    if (parsed == 0ul) {
        return fallback;
    }

    return static_cast<uint32_t>(std::min<unsigned long>(parsed, std::numeric_limits<uint32_t>::max()));
}

double elapsedMs(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

} // namespace

// --- CommandDelegate implementation ---

uint8_t AudioEngine::CommandDelegate::getMaxSynthPresetIdForTrack(uint8_t trackIndex) const
{
    return engine.getMaxSynthPresetIdForTrack(trackIndex);
}

void AudioEngine::CommandDelegate::applySoundPreset(uint8_t trackIndex, uint8_t presetId, bool manualOverride)
{
    engine.applySoundPreset(trackIndex, presetId, manualOverride);
}

void AudioEngine::CommandDelegate::resetTransportAndArrangement()
{
    engine.transport.reset();
    engine.appState.setArrangementCurrentSection(0);
    engine.arrangementAnchorInitialized = false;
    engine.arrangementAnchoredSection = 0;
    engine.arrangementAnchoredChainPosition = 0;
    engine.arrangementSectionStartBar = 0;
}

void AudioEngine::CommandDelegate::setArrangementAnchorInitialized(bool initialized)
{
    engine.arrangementAnchorInitialized = initialized;
}

void AudioEngine::CommandDelegate::resetAlgorithm(uint8_t trackIndex, uint16_t algorithmId)
{
    if (auto* algo = engine.getTrackAlgorithm(trackIndex, algorithmId)) {
        algo->reset();
        algo->clearPendingNoteOffs();
    }
    engine.resetTrackRuntime(trackIndex);
}

void AudioEngine::CommandDelegate::setDrumSampleForSlot(uint8_t slotIndex, const DrumSampleData* sampleData)
{
    engine.drumEngine.setSampleForSlot(slotIndex, sampleData);
}

void AudioEngine::CommandDelegate::setDrumSampleSlotVolume(uint8_t slotIndex, float value)
{
    engine.drumEngine.setSampleSlotVolume(slotIndex, value);
}

void AudioEngine::CommandDelegate::setDrumSampleSlotTuneSemitones(uint8_t slotIndex, float value)
{
    engine.drumEngine.setSampleSlotTuneSemitones(slotIndex, value);
}

void AudioEngine::CommandDelegate::setDrumSampleSlotStartOffset(uint8_t slotIndex, float value)
{
    engine.drumEngine.setSampleSlotStartOffset(slotIndex, value);
}

void AudioEngine::CommandDelegate::setDrumSampleSlotDecay(uint8_t slotIndex, float value)
{
    engine.drumEngine.setSampleSlotDecay(slotIndex, value);
}

void AudioEngine::CommandDelegate::setDrumSampleSlotVelocitySensitivity(uint8_t slotIndex, float value)
{
    engine.drumEngine.setSampleSlotVelocitySensitivity(slotIndex, value);
}

void AudioEngine::initializeAlgorithmMap() {
    builtinTrackAlgorithms[0] = {
        &fourOnFloor,
        &breakbeat,
        &euclideanRhythm,
        &dnbBreaks,
        &afroClaveGroove,
        &houseShuffleGroove,
        &trapHalfTimeGroove,
        &glitchPulseGroove,
        &technoRumbleGroove,
        &jerseyClubGroove,
        &brokenStepperGroove,
        &polyrhythmTomGroove,
        &electroBreaksGroove,
        &garageSwingGroove,
        &latinPercGroove,
        &minimalClicksGroove,
        &dubSkankGroove,
        &footwork160Groove,
        &halfstepGroove,
        &industrialGroove
    };

    builtinTrackAlgorithms[1] = {
        &walkingBass,
        &syncBass,
        &sub808Bass,
        &ukGarageBass,
        &tumbaoBass,
        &dubPedalBass,
        &reesePulseBass,
        &motifBass,
        &acidTripletBass,
        &glideCounterBass,
        &pulseChopBass,
        &octaveBounceBass,
        &reggaetonSubBass,
        &electroFunkBass,
        &minimalDroneBass,
        &brokenOctaveBass,
        &stepperDubBass,
        &funkPopBass,
        &neuroWobbleBass,
        &claveBass
    };

    builtinTrackAlgorithms[2] = {
        &blockChords,
        &syncStabs,
        &housePianoStabs,
        &ambientPadSwells,
        &neoSoulVoicings,
        &tranceGateChords,
        &quartalComping,
        &voiceCloudChords,
        &gospelLiftChords,
        &detuneStackChords,
        &brokenStrumChords,
        &pulseClusterChords,
        &dubSkankChords,
        &minimalPluckChords,
        &rnbKeyChords,
        &suspendedPadChords,
        &cinematicHitChords,
        &fifthDroneChords,
        &garageOrganChords,
        &polychordChords
    };

    builtinTrackAlgorithms[3] = {
        &arpeggiator,
        &markovMelody,
        &tranceContourLead,
        &raveStabLead,
        &afroCallResponseLead,
        &cinematicSparseLead,
        &euclideanLeadGate,
        &phraseMutatorLead,
        &glideRunLead,
        &microMotifLead,
        &wideIntervalLead,
        &tripletRushLead,
        &pentatonicHookLead,
        &acidLineLead,
        &dubEchoLead,
        &garageVoxLead,
        &minimalPingLead,
        &ornamentRunLead,
        &syncopatedPluckLead,
        &lydianFloatLead
    };

    for (uint8_t track = 0; track < TrackCount; ++track) {
        for (uint8_t algorithmId = 0; algorithmId < AlgorithmCatalog::getAlgorithmCountForTrack(track); ++algorithmId) {
            auto* algorithm = builtinTrackAlgorithms[track][algorithmId];
            assert(algorithm != nullptr);
            assert(algorithm->getName().toStdString() == std::string(AlgorithmCatalog::getAlgorithmName(track, algorithmId)));
        }
    }
}

AudioEngine::AudioEngine(AppState& appState,
                         CommandQueue& commandQueue,
                         MeterQueue& meterQueue,
                         const juce::String& deviceName,
                         bool enableAudioDevice,
                         DrumSampleLibrary* drumSampleLibrary,
                         MelodicSampleLibrary* melodicSampleLibrary,
                         AlgorithmPresetRegistry* algorithmRegistry)
        : appState(appState),
            commandQueue(commandQueue),
            meterQueue(meterQueue),
            drumSampleLibrary(drumSampleLibrary),
            melodicSampleLibrary(melodicSampleLibrary),
            algorithmRegistry(algorithmRegistry),
            audioDeviceEnabled(enableAudioDevice),
            effectProcessor(appState),
            commandDelegate(*this),
            commandProcessor(appState, commandQueue, effectProcessor, algorithmRegistry, drumSampleLibrary, commandDelegate)
{
    const bool traceAudio = std::getenv("CENDANCE_TRACE_AUDIO") != nullptr;
    profilingEnabled = std::getenv("CENDANCE_PROFILE_AUDIO") != nullptr;
    profilingTraceEnabled = std::getenv("CENDANCE_PROFILE_AUDIO_TRACE") != nullptr;
    profilingReportIntervalCallbacks = parseProfileInterval(
        std::getenv("CENDANCE_PROFILE_AUDIO_INTERVAL"),
        kDefaultProfileIntervalCallbacks);

    if (traceAudio) {
        std::cerr << "[cendance:audio] Constructing AudioEngine (audioDeviceEnabled="
                  << (audioDeviceEnabled ? "true" : "false") << ")" << std::endl;
    }

    if (profilingEnabled && (traceAudio || profilingTraceEnabled)) {
        std::cerr << "[cendance:profile] Audio callback profiling enabled"
                  << " (window=" << profilingReportIntervalCallbacks << " callbacks)"
                  << std::endl;
    }

    clickOsc.initialise([] (float x) { return std::sin(x); });
    initializeAlgorithmMap();
    commandProcessor.rebuildCustomAlgorithmInstances();

    for (uint8_t track = 1; track < TrackCount; ++track) {
        const uint16_t algoId = appState.tracks[track].algorithmId.load(std::memory_order_relaxed);
        const uint8_t defaultPreset = SynthCatalog::getDefaultPresetForAlgorithm(track, static_cast<uint8_t>(std::min<uint16_t>(algoId, 255)));
        applySoundPreset(track, defaultPreset, false);
    }

    for (uint8_t track = 0; track < TrackCount; ++track) {
        for (uint8_t drumSlot = 0; drumSlot < AppState::TrackState::DrumSampleSlotCount; ++drumSlot) {
            appState.tracks[track].setDrumSampleSlotSampleId(drumSlot, 0);
            appState.tracks[track].setDrumSampleSlotVolume(drumSlot, 1.0f);
            appState.tracks[track].setDrumSampleSlotTuneSemitones(drumSlot, 0.0f);
            appState.tracks[track].setDrumSampleSlotStartOffset(drumSlot, 0.0f);
            appState.tracks[track].setDrumSampleSlotDecay(drumSlot, 1.0f);
            appState.tracks[track].setDrumSampleSlotVelocitySensitivity(drumSlot, 1.0f);
        }
    }

    for (uint8_t slot = 0; slot < EffectSlotCount; ++slot) {
        appState.master.setEffectPresetSlot(slot, 0);
    }
    appState.master.setEffectPresetSlot(2, EffectPresetCatalog::kDefaultMasterLimiterPresetId);

    for (uint8_t drumSlot = 0; drumSlot < AppState::TrackState::DrumSampleSlotCount; ++drumSlot) {
        drumEngine.setSampleForSlot(drumSlot, nullptr);
        drumEngine.setSampleSlotVolume(drumSlot, 1.0f);
        drumEngine.setSampleSlotTuneSemitones(drumSlot, 0.0f);
        drumEngine.setSampleSlotStartOffset(drumSlot, 0.0f);
        drumEngine.setSampleSlotDecay(drumSlot, 1.0f);
        drumEngine.setSampleSlotVelocitySensitivity(drumSlot, 1.0f);
    }

    applySoundPreset(0, appState.tracks[0].synthPreset.load(std::memory_order_relaxed), false);

    if (audioDeviceEnabled) {
        const juce::String initError = deviceManager.initialise(0, 2, nullptr, true, deviceName, nullptr);
        if (initError.isNotEmpty()) {
            std::cerr << "[cendance:audio] Failed to initialize audio device: "
                      << initError.toStdString() << std::endl;
        }

        if (traceAudio) {
            if (auto* currentDevice = deviceManager.getCurrentAudioDevice()) {
                std::cerr << "[cendance:audio] Output device: "
                          << currentDevice->getName().toStdString()
                          << " @ " << currentDevice->getCurrentSampleRate()
                          << " Hz, buffer " << currentDevice->getCurrentBufferSizeSamples()
                          << " samples" << std::endl;
            } else {
                std::cerr << "[cendance:audio] No current audio device" << std::endl;
            }
        }
        deviceManager.addAudioCallback(this);
    }

    // Initialize capture bus
    cendance::AudioCaptureBus::Config captureConfig;
    captureConfig.numChannels = 2;
    captureConfig.sampleRate = static_cast<int>(sampleRate);
    captureConfig.capacitySeconds = 10;
    captureBus = std::make_unique<cendance::AudioCaptureBus>(captureConfig);
}

void AudioEngine::applyDrumKitPreset(uint8_t presetId) {
    const auto& preset = DrumKitPresetCatalog::getPreset(presetId);
    constexpr uint8_t drumTrack = 0;

    for (uint8_t slot = 0; slot < AppState::TrackState::DrumSampleSlotCount; ++slot) {
        const auto& slotConfig = preset.slots[slot];

        appState.tracks[drumTrack].setDrumSampleSlotSampleId(slot, slotConfig.sampleId);
        appState.tracks[drumTrack].setDrumSampleSlotVolume(slot, slotConfig.volume);
        appState.tracks[drumTrack].setDrumSampleSlotTuneSemitones(slot, slotConfig.tuneSemitones);
        appState.tracks[drumTrack].setDrumSampleSlotStartOffset(slot, slotConfig.startOffset);
        appState.tracks[drumTrack].setDrumSampleSlotDecay(slot, slotConfig.decay);
        appState.tracks[drumTrack].setDrumSampleSlotVelocitySensitivity(slot, slotConfig.velocitySensitivity);

        const DrumSampleData* sampleData = drumSampleLibrary != nullptr
            ? drumSampleLibrary->getRtSample(slotConfig.sampleId)
            : nullptr;
        drumEngine.setSampleForSlot(slot, sampleData);
        drumEngine.setSampleSlotVolume(slot, slotConfig.volume);
        drumEngine.setSampleSlotTuneSemitones(slot, slotConfig.tuneSemitones);
        drumEngine.setSampleSlotStartOffset(slot, slotConfig.startOffset);
        drumEngine.setSampleSlotDecay(slot, slotConfig.decay);
        drumEngine.setSampleSlotVelocitySensitivity(slot, slotConfig.velocitySensitivity);
    }
}

void AudioEngine::applyMelodicSamplePreset(uint8_t trackIndex, uint8_t presetId) {
    if (!SynthCatalog::isMelodicSamplerPreset(trackIndex, presetId)) {
        return;
    }

    if (melodicSampleLibrary == nullptr) {
        return;
    }

    std::string error;
    if (!melodicSampleLibrary->configurePreset(trackIndex,
                                               presetId,
                                               &bassEngine,
                                               &chordEngine,
                                               &leadEngine,
                                               error)
        && !error.empty()) {
        std::cerr << "[cendance:audio] Failed to configure melodic sample preset "
                  << static_cast<int>(presetId)
                  << " on track " << static_cast<int>(trackIndex)
                  << ": " << error << std::endl;
    }
}

void AudioEngine::applySoundPresetEffectSlots(uint8_t trackIndex, uint8_t presetId) {
    if (trackIndex >= TrackCount)
        return;

    const auto slots = SynthCatalog::getPresetEffectSlots(trackIndex, presetId);
    for (uint8_t slot = 0; slot < EffectSlotCount; ++slot)
        effectProcessor.applyTrackEffectPreset(trackIndex, slot, slots[slot]);
}

void AudioEngine::applySoundPreset(uint8_t trackIndex, uint8_t presetId, bool manualOverride) {
    if (trackIndex >= TrackCount) {
        return;
    }

    const uint8_t maxPreset = getMaxSynthPresetIdForTrack(trackIndex);
    const uint8_t preset = static_cast<uint8_t>(std::min<uint16_t>(presetId, maxPreset));
    const uint8_t previousPreset = appState.tracks[trackIndex].synthPreset.load(std::memory_order_relaxed);
    appState.tracks[trackIndex].setSynthPreset(preset);
    appState.tracks[trackIndex].setSynthManualOverride(manualOverride);

    if (preset != previousPreset) {
        resetTrackRuntime(trackIndex);
    }

    if (trackIndex == 0) {
        applyDrumKitPreset(preset);
    } else {
        applyMelodicSamplePreset(trackIndex, preset);
    }

    applySoundPresetEffectSlots(trackIndex, preset);
}

void AudioEngine::resetTrackRuntime(uint8_t trackIndex) {
    if (trackIndex >= TrackCount) {
        return;
    }

    activeNotes[trackIndex][0] = 0;
    activeNotes[trackIndex][1] = 0;

    switch (trackIndex) {
        case 0: drumEngine.reset(); break;
        case 1: bassEngine.reset(); break;
        case 2: chordEngine.reset(); break;
        case 3: leadEngine.reset(); break;
        default: break;
    }
}

AudioEngine::~AudioEngine() {
    if (audioDeviceEnabled) {
        deviceManager.removeAudioCallback(this);
    }
}

void AudioEngine::processCommandsForTest() {
    commandProcessor.process();
}

void AudioEngine::updateArrangementForTest(bool playing, bool isNewBar, uint16_t barNumber) {
    updateArrangementState(playing, isNewBar, barNumber);
}

int AudioEngine::getArrangementProgressionStepForTest(double blockStartBeats) const {
    return getArrangementProgressionStep(blockStartBeats);
}

int AudioEngine::getArrangementProgressionStep(double blockStartBeats) const {
    const double sectionStartBeats = static_cast<double>(arrangementSectionStartBar) * 4.0;
    const double sectionRelativeBeats = std::max(0.0, blockStartBeats - sectionStartBeats);
    int progressionStep = static_cast<int>(std::floor(sectionRelativeBeats / 4.0)) % 4;
    if (progressionStep < 0) {
        progressionStep += 4;
    }
    return progressionStep;
}

void AudioEngine::updateArrangementState(bool playing, bool isNewBar, uint16_t barNumber) {
    auto applySectionTrackParameters = [&](uint8_t section) {
        if (!appState.arrangementSectionParametersEnabled.load(std::memory_order_relaxed)) {
            return;
        }
        for (uint8_t track = 0; track < TrackCount; ++track) {
            appState.tracks[track].setDensity(appState.getArrangementSectionTrackParameter(section, track, 0));
            appState.tracks[track].setComplexity(appState.getArrangementSectionTrackParameter(section, track, 1));
            appState.tracks[track].setTone(appState.getArrangementSectionTrackParameter(section, track, 2));
            appState.tracks[track].setMotion(appState.getArrangementSectionTrackParameter(section, track, 3));
        }
    };

    const uint8_t sectionCount = static_cast<uint8_t>(std::max<uint8_t>(
        appState.arrangementSectionCount.load(std::memory_order_relaxed),
        1));
    uint8_t currentSection = appState.arrangementCurrentSection.load(std::memory_order_relaxed);
    if (currentSection >= sectionCount) {
        currentSection = static_cast<uint8_t>(sectionCount - 1);
        appState.setArrangementCurrentSection(currentSection);
    }

    std::array<uint8_t, AppState::kArrangementMaxSections> chainSteps{};
    uint8_t chainStepCount = 0;

    if (appState.arrangementChainEnabled.load(std::memory_order_relaxed)) {
        const uint8_t chainLength = appState.getArrangementChainLength();
        for (uint8_t i = 0; i < chainLength; ++i) {
            const uint8_t section = appState.getArrangementChainStep(i);
            if (section < sectionCount) {
                chainSteps[chainStepCount++] = section;
            }
        }
    }

    if (chainStepCount == 0) {
        for (uint8_t i = 0; i < sectionCount; ++i) {
            chainSteps[i] = i;
        }
        chainStepCount = sectionCount;
    }

    auto resolveChainPosition = [&](uint8_t section) {
        for (uint8_t i = 0; i < chainStepCount; ++i) {
            if (chainSteps[i] == section) {
                return i;
            }
        }
        return static_cast<uint8_t>(0);
    };

    if (!arrangementAnchorInitialized || arrangementAnchoredSection != currentSection) {
        arrangementAnchorInitialized = true;
        arrangementAnchoredSection = currentSection;
        arrangementAnchoredChainPosition = resolveChainPosition(currentSection);
        arrangementSectionStartBar = barNumber;
        applySectionTrackParameters(currentSection);
    }

    if (playing && isNewBar) {
        const uint8_t mode = appState.arrangementMode.load(std::memory_order_relaxed);
        const bool autoAdvance = (mode == AppState::kArrangementModeAuto)
            || (mode == AppState::kArrangementModeMixed);
        if (autoAdvance) {
            const uint8_t barsPerSection = appState.getArrangementSectionLength(currentSection);
            const uint16_t barsElapsed = static_cast<uint16_t>(barNumber - arrangementSectionStartBar);
            if (barsElapsed >= barsPerSection) {
                const uint8_t nextChainPosition = static_cast<uint8_t>((arrangementAnchoredChainPosition + 1u)
                                                                        % chainStepCount);
                const uint8_t nextSection = chainSteps[nextChainPosition];
                appState.setArrangementCurrentSection(nextSection);
                arrangementAnchoredSection = nextSection;
                arrangementAnchoredChainPosition = nextChainPosition;
                arrangementSectionStartBar = barNumber;
                applySectionTrackParameters(nextSection);
            }
        }
    }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (std::getenv("CENDANCE_TRACE_AUDIO") != nullptr && device != nullptr) {
        std::cerr << "[cendance:audio] audioDeviceAboutToStart: "
                  << device->getName().toStdString()
                  << " @ " << device->getCurrentSampleRate()
                  << " Hz, buffer " << device->getCurrentBufferSizeSamples()
                  << std::endl;
    }

    sampleRate = device->getCurrentSampleRate();
    transport.prepare(sampleRate);
    transport.setBpm(appState.bpm.load(std::memory_order_relaxed));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = device->getDefaultBufferSize();
    spec.numChannels = 2;
    clickOsc.prepare(spec);
    clickOsc.setFrequency(1000.0f);

    effectProcessor.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));

    drumEngine.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));
    bassEngine.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));
    chordEngine.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));
    leadEngine.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));
    grooveProcessor.prepare(sampleRate, static_cast<int>(spec.maximumBlockSize));

    for (auto& trackBuffer : trackAudioBuffers) {
        trackBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize), false, false, true);
        trackBuffer.clear();
    }
    masterMixBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize), false, false, true);
    masterMixBuffer.clear();

    analyzerFifo.fill(0.0f);
    analyzerFftData.fill(0.0f);
    latestSpectrumBins.fill(0.0f);
    analyzerFifoIndex = 0;
    analyzerFrameCounter = 0;
    profilingAccumulator = ProfilingAccumulator{};
    latestProfilingSnapshot = ProfilingSnapshot{};
    for (int i = 0; i < AnalyzerFftSize; ++i) {
        analyzerWindow[static_cast<size_t>(i)] = 0.5f - 0.5f * std::cos((2.0f * juce::MathConstants<float>::pi * i) / static_cast<float>(AnalyzerFftSize - 1));
    }
}

void AudioEngine::audioDeviceStopped() {
    if (std::getenv("CENDANCE_TRACE_AUDIO") != nullptr) {
        std::cerr << "[cendance:audio] audioDeviceStopped" << std::endl;
    }

    analyzerFifo.fill(0.0f);
    analyzerFftData.fill(0.0f);
    latestSpectrumBins.fill(0.0f);
    analyzerFifoIndex = 0;
}

void AudioEngine::audioDeviceError(const juce::String& errorMessage) {
    if (std::getenv("CENDANCE_TRACE_AUDIO") != nullptr) {
        std::cerr << "[cendance:audio] audioDeviceError: " << errorMessage.toStdString() << std::endl;
    }
}

void AudioEngine::audioDeviceIOCallback(const float* const* inputChannelData,
                                        int numInputChannels,
                                        float* const* outputChannelData,
                                        int numOutputChannels,
                                        int numSamples)
{
    const juce::AudioIODeviceCallbackContext defaultContext{};
    audioDeviceIOCallbackWithContext(inputChannelData,
                                     numInputChannels,
                                     outputChannelData,
                                     numOutputChannels,
                                     numSamples,
                                     defaultContext);
}



void AudioEngine::updateSpectrum(const float* source, int numSamples, MeterData& meterData) {
    if (source == nullptr || numSamples <= 0) {
        meterData.spectrumBins = latestSpectrumBins;
        return;
    }

    bool computedSpectrum = false;
    for (int i = 0; i < numSamples; ++i) {
        analyzerFifo[static_cast<size_t>(analyzerFifoIndex)] = source[i];
        ++analyzerFifoIndex;

        if (analyzerFifoIndex >= AnalyzerFftSize) {
            analyzerFifoIndex = 0;
            analyzerFftData.fill(0.0f);

            for (int n = 0; n < AnalyzerFftSize; ++n) {
                analyzerFftData[static_cast<size_t>(n)] = analyzerFifo[static_cast<size_t>(n)] * analyzerWindow[static_cast<size_t>(n)];
            }

            analyzerFft.performFrequencyOnlyForwardTransform(analyzerFftData.data(), true);

            constexpr int positiveBinCount = AnalyzerFftSize / 2;
            const float nyquistHz = static_cast<float>(sampleRate * 0.5);
            const float minHz = 20.0f;
            const float maxHz = std::max(minHz + 1.0f, nyquistHz);
            const float fftSizeF = static_cast<float>(AnalyzerFftSize);

            auto hzToBin = [&](float hz) {
                const float normalized = std::clamp(hz / static_cast<float>(sampleRate), 0.0f, 0.5f);
                const int fftBin = static_cast<int>(std::floor(normalized * fftSizeF));
                return std::clamp(fftBin, 1, positiveBinCount - 1);
            };

            for (size_t bin = 0; bin < latestSpectrumBins.size(); ++bin) {
                const float t0 = static_cast<float>(bin) / static_cast<float>(latestSpectrumBins.size());
                const float t1 = static_cast<float>(bin + 1) / static_cast<float>(latestSpectrumBins.size());
                const float hzStart = minHz * std::pow(maxHz / minHz, t0);
                const float hzEnd = minHz * std::pow(maxHz / minHz, t1);

                const int start = hzToBin(hzStart);
                const int endExclusive = std::max(start + 1, hzToBin(hzEnd) + 1);

                float peak = 0.0f;
                for (int fftBin = start; fftBin < endExclusive; ++fftBin) {
                    peak = std::max(peak, analyzerFftData[static_cast<size_t>(fftBin)]);
                }

                // Normalize by FFT Size
                const float normalizedPeak = peak / static_cast<float>(positiveBinCount);
                if (normalizedPeak <= 1.0e-7f) {
                    latestSpectrumBins[bin] = 0.0f;
                    continue;
                }

                // Add +3dB per octave tilt for pink noise flat response
                const float centerHz = std::sqrt(hzStart * hzEnd);
                const float octavesAbove20Hz = std::log2(std::max(centerHz / 20.0f, 1.0f));
                const float tiltDb = octavesAbove20Hz * 3.0f;

                const float dB = 20.0f * std::log10(std::max(normalizedPeak, 1.0e-5f)) + tiltDb;
                
                // Adjust dynamic range: -90dB to 0dB
                latestSpectrumBins[bin] = std::clamp((dB + 90.0f) / 90.0f, 0.0f, 1.0f);
            }
            computedSpectrum = true;
        }
    }

    meterData.spectrumBins = latestSpectrumBins;
    if (!computedSpectrum && !meterData.analyzerValid) {
        meterData.spectrumBins.fill(0.0f);
    }
}















void AudioEngine::renderMetronome(float* const* outputChannelData, int numOutputChannels, int numSamples) {
    constexpr float metronomeGain = 0.15811388f; // 0.5 reduced by 10 dB.

    for (int i = 0; i < numSamples; ++i) {
        float sample = clickOsc.processSample(0.0f) * clickEnvelope;
        clickEnvelope *= 0.999f; // Slightly longer decay 
        
        for (int ch = 0; ch < numOutputChannels; ++ch) {
            if (outputChannelData[ch] != nullptr) {
                outputChannelData[ch][i] += sample * metronomeGain;
            }
        }
    }
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                   int numInputChannels,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext& context)
{
    if (std::getenv("CENDANCE_TRACE_AUDIO") != nullptr && !tracedFirstCallback.exchange(true, std::memory_order_relaxed)) {
        std::cerr << "[cendance:audio] First audio callback: numSamples=" << numSamples
                  << ", outputs=" << numOutputChannels << std::endl;
    }

    const bool profileThisCallback = profilingEnabled;
    std::chrono::steady_clock::time_point callbackStart{};
    std::chrono::steady_clock::time_point phaseStart{};
    double commandsMs = 0.0;
    double generationMs = 0.0;
    double trackFxMs = 0.0;
    double masterFxMs = 0.0;
    double meteringMs = 0.0;
    if (profileThisCallback) {
        callbackStart = std::chrono::steady_clock::now();
        phaseStart = callbackStart;
    }

    juce::ignoreUnused(inputChannelData, numInputChannels, context);

    // Clear outputs
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch] != nullptr) {
            std::fill_n(outputChannelData[ch], numSamples, 0.0f);
        }
    }

    commandProcessor.process();
    effectProcessor.applySpotEffectsBitmask(appState.activeSpotEffects.load(std::memory_order_relaxed));
    if (profileThisCallback) {
        const auto afterCommands = std::chrono::steady_clock::now();
        commandsMs = elapsedMs(phaseStart, afterCommands);
        phaseStart = afterCommands;
    }

    float currentBpm = appState.bpm.load(std::memory_order_relaxed);
    transport.setBpm(currentBpm);
    const double blockStartBeats = transport.getPlayheadPosition();
    const double blockLengthBeats = (sampleRate > 0.0)
        ? (static_cast<double>(numSamples) * currentBpm) / (60.0 * sampleRate)
        : 0.0;
    
    bool playing = appState.playing.load(std::memory_order_relaxed);
    
    if (playing) {
        transport.advance(numSamples);

        if (transport.isNewBeat()) {
            if (transport.getCurrentBeat() == 0) {
                clickOsc.setFrequency(1500.0f);
            } else {
                clickOsc.setFrequency(1000.0f);
            }
            clickEnvelope = 1.0f;
        }
    }

    MeterData meterData;
    meterData.isPlaying = playing;
    meterData.beatPosition = transport.getCurrentBeat();
    meterData.barNumber = transport.getCurrentBar();
    updateArrangementState(playing, transport.isNewBar(), meterData.barNumber);
    const uint8_t currentSection = appState.arrangementCurrentSection.load(std::memory_order_relaxed);

    if (playing && blockLengthBeats > 0.0) {
        masterMixBuffer.clear();

        const uint8_t globalProgression = appState.chordProgression.load(std::memory_order_relaxed);
        const uint8_t sectionProgression = appState.getArrangementSectionProgression(currentSection);
        uint8_t activeProgression = globalProgression;
        if (sectionProgression != AppState::kArrangementProgressionFollowGlobal) {
            const int progressionCount = ChordProgression::getNumProgressions();
            const uint8_t maxProgression = progressionCount > 0
                ? static_cast<uint8_t>(progressionCount - 1)
                : 0;
            activeProgression = static_cast<uint8_t>(std::min<uint8_t>(sectionProgression, maxProgression));
        }

        const uint8_t sectionTrackMask = appState.getArrangementSectionTrackMask(currentSection);
        const auto& progression = ChordProgression::get(activeProgression);
        const uint8_t keyRootClass = static_cast<uint8_t>(appState.projectKeyRoot.load(std::memory_order_relaxed) % 12);
        const uint8_t keyModeValue = appState.projectKeyMode.load(std::memory_order_relaxed);
        const Scale::Type keyMode = (keyModeValue == AppState::kProjectKeyModeMajor)
            ? Scale::Type::Major
            : Scale::Type::NaturalMinor;
        Scale projectScale(keyRootClass, keyMode);
        const int progressionStep = getArrangementProgressionStep(blockStartBeats);
        const int progressionDegree = progression.degrees[progressionStep];
        const int rootNote = projectScale.getDegree(progressionDegree, 3);
        const int progressionRootClass = ((rootNote % 12) + 12) % 12;
        Scale progressionScale(progressionRootClass, keyMode);

        for (int track = 0; track < TrackCount; ++track) {
            std::chrono::steady_clock::time_point generationStart{};
            if (profileThisCallback) {
                generationStart = std::chrono::steady_clock::now();
            }

            auto& midi = trackMidiBuffers[track];
            auto& trackBuffer = trackAudioBuffers[track];
            midi.clear();
            trackBuffer.clear();

            const uint16_t algoId = appState.tracks[track].algorithmId.load(std::memory_order_relaxed);
            const uint8_t synthPreset = appState.tracks[track].synthPreset.load(std::memory_order_relaxed);
            const float density = appState.tracks[track].density.load(std::memory_order_relaxed);
            const float complexity = appState.tracks[track].complexity.load(std::memory_order_relaxed);
            const float tone = appState.tracks[track].tone.load(std::memory_order_relaxed);
            const float motion = appState.tracks[track].motion.load(std::memory_order_relaxed);
            const bool muted = appState.tracks[track].muted.load(std::memory_order_relaxed);
            const bool arrangementMuted = (sectionTrackMask & static_cast<uint8_t>(1u << track)) == 0u;
            const float trackGain = appState.tracks[track].gain.load(std::memory_order_relaxed);
            const float trackTrim = ((track == 0) ? kDrumTrackTrimLinear : 1.0f)
                * SynthCatalog::getPresetLoudnessTrim(static_cast<uint8_t>(track), synthPreset);

            meterData.activeAlgorithm[track] = algoId;

            if (muted || arrangementMuted) {
                meterData.trackLevels[track] = 0.0f;
                activeNotes[track][0] = 0;
                activeNotes[track][1] = 0;
                meterData.activeNotes[track][0] = 0;
                meterData.activeNotes[track][1] = 0;
                if (profileThisCallback) {
                    generationMs += elapsedMs(generationStart, std::chrono::steady_clock::now());
                }
                continue;
            }

            if (auto* algorithm = getTrackAlgorithm(static_cast<uint8_t>(track), algoId)) {
                algorithm->flushPendingNoteOffs(midi, blockStartBeats, blockLengthBeats, numSamples);
                algorithm->processMidi(midi, blockStartBeats, blockLengthBeats, numSamples, progressionScale, rootNote, density, complexity);
            }

            // Apply groove: swing, velocity humanization, timing jitter
            {
                const float swing = appState.getSwingAmount();
                const float velH = appState.getVelocityHumanize();
                const float jitter = appState.getTimingJitter();
                if (swing > 0.001f || velH > 0.001f || jitter > 0.001f) {
                    grooveProcessor.apply(midi, blockStartBeats, blockLengthBeats, numSamples, swing, velH, jitter);
                }
            }

            meterData.activeNotes[track][0] = activeNotes[track][0];
            meterData.activeNotes[track][1] = activeNotes[track][1];

            for (const auto meta : midi) {
                const auto msg = meta.getMessage();
                if (msg.isNoteOn()) {
                    int note = msg.getNoteNumber();
                    if (note >= 0 && note < 128) {
                        activeNotes[track][note / 64] |= (1ULL << (note % 64));
                        meterData.activeNotes[track][note / 64] |= (1ULL << (note % 64));
                    }
                } else if (msg.isNoteOff()) {
                    int note = msg.getNoteNumber();
                    if (note >= 0 && note < 128) {
                        activeNotes[track][note / 64] &= ~(1ULL << (note % 64));
                    }
                }
            }

            if (track == 0) {
                drumEngine.setTone(tone);
                drumEngine.setMotion(motion);
            } else if (track == 1) {
                bassEngine.setPreset(synthPreset);
                bassEngine.setTone(tone);
                bassEngine.setMotion(motion);
            } else if (track == 2) {
                chordEngine.setPreset(synthPreset);
                chordEngine.setTone(tone);
                chordEngine.setMotion(motion);
            } else if (track == 3) {
                leadEngine.setPreset(synthPreset);
                leadEngine.setTone(tone);
                leadEngine.setMotion(motion);
            }

            switch (track) {
                case 0: drumEngine.renderNextBlock(trackBuffer, midi, numSamples); break;
                case 1: bassEngine.renderNextBlock(trackBuffer, midi, numSamples); break;
                case 2: chordEngine.renderNextBlock(trackBuffer, midi, numSamples); break;
                case 3: leadEngine.renderNextBlock(trackBuffer, midi, numSamples); break;
            }

            std::chrono::steady_clock::time_point trackFxStart{};
            if (profileThisCallback) {
                trackFxStart = std::chrono::steady_clock::now();
                generationMs += elapsedMs(generationStart, trackFxStart);
            }
            effectProcessor.processTrackInsertEffects(static_cast<uint8_t>(track), trackBuffer, numSamples, currentBpm);
            if (profileThisCallback) {
                const auto afterTrackFx = std::chrono::steady_clock::now();
                trackFxMs += elapsedMs(trackFxStart, afterTrackFx);
                generationStart = afterTrackFx;
            }

            for (int ch = 0; ch < masterMixBuffer.getNumChannels() && ch < trackBuffer.getNumChannels(); ++ch) {
                const float* trackData = trackBuffer.getReadPointer(ch);
                float* outData = masterMixBuffer.getWritePointer(ch);
                for (int i = 0; i < numSamples; ++i) {
                    outData[i] += trackData[i] * 0.5f * trackGain * trackTrim;
                }
            }

            float trackPeak = 0.0f;
            if (trackBuffer.getNumChannels() > 0) {
                const float* t = trackBuffer.getReadPointer(0);
                for (int i = 0; i < numSamples; ++i) {
                    trackPeak = std::max(trackPeak, std::abs(t[i]) * trackGain * trackTrim);
                }
            }
            const float dB = 20.0f * std::log10(std::max(trackPeak, 1.0e-5f));
            float normalized = std::clamp((dB + 60.0f) / 60.0f, 0.0f, 1.0f);
            normalized = std::pow(normalized, 0.7f);
            meterData.trackLevels[track] = normalized;

            if (profileThisCallback) {
                generationMs += elapsedMs(generationStart, std::chrono::steady_clock::now());
            }
        }

        std::chrono::steady_clock::time_point masterFxStart{};
        if (profileThisCallback) {
            masterFxStart = std::chrono::steady_clock::now();
        }
        effectProcessor.processMasterEffects(masterMixBuffer, numSamples, currentBpm);
        effectProcessor.processSpotEffects(masterMixBuffer, numSamples, currentBpm);
        const float masterGain = appState.master.gain.load(std::memory_order_relaxed);
        if (profileThisCallback) {
            masterFxMs += elapsedMs(masterFxStart, std::chrono::steady_clock::now());
        }

        for (int ch = 0; ch < numOutputChannels && ch < masterMixBuffer.getNumChannels(); ++ch) {
            if (outputChannelData[ch] == nullptr) {
                continue;
            }

            const float* mixData = masterMixBuffer.getReadPointer(ch);
            float* outData = outputChannelData[ch];
            for (int i = 0; i < numSamples; ++i) {
                outData[i] += mixData[i] * masterGain;
            }
        }
    } else {
        for (int track = 0; track < TrackCount; ++track) {
            meterData.trackLevels[track] = 0.0f;
            meterData.activeAlgorithm[track] = appState.tracks[track].algorithmId.load(std::memory_order_relaxed);
        }
    }

    if (playing && appState.metronomeEnabled.load(std::memory_order_relaxed)) {
        renderMetronome(outputChannelData, numOutputChannels, numSamples);
    }

    // Capture the final output (post-master-effects, post-metronome)
    if (captureBus && captureBus->isActive()) {
        captureBus->push(outputChannelData, numSamples);
    }

    // Stream the final output
    if (streamSink.getStatus().streaming) {
        streamSink.push(outputChannelData, numSamples);
    }

    std::chrono::steady_clock::time_point meteringStart{};
    if (profileThisCallback) {
        meteringStart = std::chrono::steady_clock::now();
    }

    float masterPeak = 0.0f;
    if (numOutputChannels > 0 && outputChannelData[0] != nullptr) {
        const float* master = outputChannelData[0];
        for (int i = 0; i < numSamples; ++i) {
            masterPeak = std::max(masterPeak, std::abs(master[i]));
        }
    }
    const float masterdB = 20.0f * std::log10(std::max(masterPeak, 1.0e-5f));
    float normalizedMaster = std::clamp((masterdB + 60.0f) / 60.0f, 0.0f, 1.0f);
    normalizedMaster = std::pow(normalizedMaster, 0.7f);
    meterData.masterLevel = normalizedMaster;

    meterData.analyzerValid = (numOutputChannels > 0 && outputChannelData[0] != nullptr && numSamples > 0);
    meterData.analyzerFrame = ++analyzerFrameCounter;

    if (meterData.analyzerValid) {
        const float* analysisSource = outputChannelData[0];
        updateSpectrum(analysisSource, numSamples, meterData);
    } else {
        meterData.spectrumBins.fill(0.0f);
    }

    if (profileThisCallback) {
        const auto afterMetering = std::chrono::steady_clock::now();
        meteringMs = elapsedMs(meteringStart, afterMetering);
        const double callbackMs = elapsedMs(callbackStart, afterMetering);
        const double bufferDurationMs = sampleRate > 0.0
            ? (static_cast<double>(numSamples) * 1000.0) / sampleRate
            : 0.0;
        const double callbackUtilization = bufferDurationMs > 0.0
            ? (callbackMs / bufferDurationMs) * 100.0
            : 0.0;

        profilingAccumulator.callbackMsSum += callbackMs;
        profilingAccumulator.callbackMsPeak = std::max(profilingAccumulator.callbackMsPeak, callbackMs);
        profilingAccumulator.callbackUtilizationSum += callbackUtilization;
        profilingAccumulator.callbackUtilizationPeak = std::max(profilingAccumulator.callbackUtilizationPeak, callbackUtilization);
        profilingAccumulator.commandsMsSum += commandsMs;
        profilingAccumulator.generationMsSum += generationMs;
        profilingAccumulator.trackFxMsSum += trackFxMs;
        profilingAccumulator.masterFxMsSum += masterFxMs;
        profilingAccumulator.meteringMsSum += meteringMs;
        ++profilingAccumulator.callbacks;

        if (profilingAccumulator.callbacks >= profilingReportIntervalCallbacks) {
            const double callbackCount = static_cast<double>(profilingAccumulator.callbacks);
            latestProfilingSnapshot.valid = true;
            latestProfilingSnapshot.callbacks = profilingAccumulator.callbacks;
            latestProfilingSnapshot.bufferDurationMs = static_cast<float>(bufferDurationMs);
            latestProfilingSnapshot.callbackMsAvg = static_cast<float>(profilingAccumulator.callbackMsSum / callbackCount);
            latestProfilingSnapshot.callbackMsPeak = static_cast<float>(profilingAccumulator.callbackMsPeak);
            latestProfilingSnapshot.callbackUtilizationAvg = static_cast<float>(profilingAccumulator.callbackUtilizationSum / callbackCount);
            latestProfilingSnapshot.callbackUtilizationPeak = static_cast<float>(profilingAccumulator.callbackUtilizationPeak);
            latestProfilingSnapshot.commandsMsAvg = static_cast<float>(profilingAccumulator.commandsMsSum / callbackCount);
            latestProfilingSnapshot.generationMsAvg = static_cast<float>(profilingAccumulator.generationMsSum / callbackCount);
            latestProfilingSnapshot.trackFxMsAvg = static_cast<float>(profilingAccumulator.trackFxMsSum / callbackCount);
            latestProfilingSnapshot.masterFxMsAvg = static_cast<float>(profilingAccumulator.masterFxMsSum / callbackCount);
            latestProfilingSnapshot.meteringMsAvg = static_cast<float>(profilingAccumulator.meteringMsSum / callbackCount);

            if (profilingTraceEnabled) {
                std::cerr << "[cendance:profile] avg=" << latestProfilingSnapshot.callbackMsAvg
                          << "ms peak=" << latestProfilingSnapshot.callbackMsPeak
                          << "ms util(avg/peak)=" << latestProfilingSnapshot.callbackUtilizationAvg
                          << "%/" << latestProfilingSnapshot.callbackUtilizationPeak
                          << "% cmd=" << latestProfilingSnapshot.commandsMsAvg
                          << " gen=" << latestProfilingSnapshot.generationMsAvg
                          << " fx=" << latestProfilingSnapshot.trackFxMsAvg
                          << " master=" << latestProfilingSnapshot.masterFxMsAvg
                          << " meter=" << latestProfilingSnapshot.meteringMsAvg
                          << std::endl;
            }

            profilingAccumulator = ProfilingAccumulator{};
        }
    }

    meterData.performanceProfileValid = latestProfilingSnapshot.valid;
    if (latestProfilingSnapshot.valid) {
        meterData.profileWindowCallbacks = latestProfilingSnapshot.callbacks;
        meterData.profileBufferDurationMs = latestProfilingSnapshot.bufferDurationMs;
        meterData.callbackMsAvg = latestProfilingSnapshot.callbackMsAvg;
        meterData.callbackMsPeak = latestProfilingSnapshot.callbackMsPeak;
        meterData.callbackUtilizationAvg = latestProfilingSnapshot.callbackUtilizationAvg;
        meterData.callbackUtilizationPeak = latestProfilingSnapshot.callbackUtilizationPeak;
        meterData.commandsMsAvg = latestProfilingSnapshot.commandsMsAvg;
        meterData.generationMsAvg = latestProfilingSnapshot.generationMsAvg;
        meterData.trackFxMsAvg = latestProfilingSnapshot.trackFxMsAvg;
        meterData.masterFxMsAvg = latestProfilingSnapshot.masterFxMsAvg;
        meterData.meteringMsAvg = latestProfilingSnapshot.meteringMsAvg;
    }

    meterQueue.push(meterData);
}

// --- Recording controls ---

bool AudioEngine::startRecording(const std::string& filePath, cendance::FileRecorder::Format format) {
    if (!captureBus)
        return false;

    // Stop any existing recording
    if (fileRecorder.getStatus().recording)
        fileRecorder.stop();

    // Reset the capture bus
    captureBus->reset();

    // Configure and start the recorder
    cendance::FileRecorder::Info info;
    info.filePath = filePath;
    info.format = format;
    info.sampleRate = static_cast<int>(sampleRate);
    info.numChannels = 2;

    if (!fileRecorder.start(*captureBus, info))
        return false;

    // Enable capture
    captureBus->setActive(true);
    return true;
}

void AudioEngine::stopRecording() {
    if (captureBus)
        captureBus->setActive(false);
    fileRecorder.stop();
}

bool AudioEngine::isRecording() const {
    return fileRecorder.getStatus().recording;
}

cendance::FileRecorder::Status AudioEngine::getRecordingStatus() const {
    return fileRecorder.getStatus();
}

cendance::AudioCaptureBus::RecordingState AudioEngine::getCaptureState() const {
    if (captureBus)
        return captureBus->getState();
    return cendance::AudioCaptureBus::RecordingState{};
}

// --- Streaming controls ---

bool AudioEngine::startStreaming(cendance::StreamSink::SinkFn sink, cendance::StreamSink::Format format) {
    cendance::StreamSink::Config config;
    config.numChannels = 2;
    config.sampleRate = static_cast<int>(sampleRate);
    config.format = format;
    return streamSink.start(config, std::move(sink));
}

void AudioEngine::stopStreaming() {
    streamSink.stop();
}

bool AudioEngine::isStreaming() const {
    return streamSink.getStatus().streaming;
}

cendance::StreamSink::Status AudioEngine::getStreamingStatus() const {
    return streamSink.getStatus();
}
