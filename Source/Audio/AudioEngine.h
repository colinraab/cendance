#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

#include "../App/AlgorithmCatalog.h"
#include "../App/AlgorithmPresetRegistry.h"
#include "../App/AppState.h"
#include "../App/CommandQueue.h"
#include "../App/DrumSampleLibrary.h"
#include "../App/MelodicSampleLibrary.h"
#include "../App/MeterQueue.h"
#include "Transport.h"
#include "Synths/DrumEngine.h"
#include "Synths/BassEngine.h"
#include "Synths/ChordEngine.h"
#include "Synths/LeadEngine.h"
#include "Generators/GenerativeAlgorithm.h"
#include "Generators/FourOnFloor.h"
#include "Generators/CustomAlgorithmInstance.h"
#include "Generators/Breakbeat.h"
#include "Generators/EuclideanRhythm.h"
#include "Generators/WalkingBass.h"
#include "Generators/SyncBass.h"
#include "Generators/BlockChords.h"
#include "Generators/SyncStabs.h"
#include "Generators/Arpeggiator.h"
#include "Generators/MarkovMelody.h"
#include "Generators/DrumStyleAlgorithms.h"
#include "Generators/BassStyleAlgorithms.h"
#include "Generators/ChordStyleAlgorithms.h"
#include "Generators/LeadStyleAlgorithms.h"
#include "Harmony/Scale.h"
#include "EffectProcessor.h"
#include "CommandProcessor.h"
#include "AudioCaptureBus.h"
#include "FileRecorder.h"
#include "StreamSink.h"
#include "GrooveProcessor.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <string>

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine(AppState& appState,
                CommandQueue& commandQueue,
                MeterQueue& meterQueue,
                const juce::String& deviceName = "",
                bool enableAudioDevice = true,
                DrumSampleLibrary* drumSampleLibrary = nullptr,
                MelodicSampleLibrary* melodicSampleLibrary = nullptr,
                AlgorithmPresetRegistry* algorithmRegistry = nullptr);
    ~AudioEngine() override;

    void processCommandsForTest();
    void updateArrangementForTest(bool playing, bool isNewBar, uint16_t barNumber);
    int getArrangementProgressionStepForTest(double blockStartBeats) const;

    void audioDeviceIOCallback(const float* const* inputChannelData,
                               int numInputChannels,
                               float* const* outputChannelData,
                               int numOutputChannels,
                               int numSamples);
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    // --- Recording controls ---
    bool startRecording(const std::string& filePath, cendance::FileRecorder::Format format = cendance::FileRecorder::Format::WavF32);
    void stopRecording();
    bool isRecording() const;
    cendance::FileRecorder::Status getRecordingStatus() const;
    cendance::AudioCaptureBus::RecordingState getCaptureState() const;

    // --- Streaming controls ---
    bool startStreaming(cendance::StreamSink::SinkFn sink, cendance::StreamSink::Format format = cendance::StreamSink::Format::F32LE);
    void stopStreaming();
    bool isStreaming() const;
    cendance::StreamSink::Status getStreamingStatus() const;

private:
    static constexpr uint8_t TrackCount = 4;
    static constexpr uint8_t EffectSlotCount = 3;

    static constexpr int AnalyzerFftOrder = 8;
    static constexpr int AnalyzerFftSize = 1 << AnalyzerFftOrder;

    // --- Delegate adapter for CommandProcessor ---
    class CommandDelegate : public CommandProcessor::Delegate
    {
    public:
        explicit CommandDelegate(AudioEngine& engine) : engine(engine) {}
        ~CommandDelegate() override = default;

        uint8_t getMaxSynthPresetIdForTrack(uint8_t trackIndex) const override;
        void applySoundPreset(uint8_t trackIndex, uint8_t presetId, bool manualOverride) override;
        void resetTransportAndArrangement() override;
        void setArrangementAnchorInitialized(bool initialized) override;
        void resetAlgorithm(uint8_t trackIndex, uint16_t algorithmId) override;
        void setDrumSampleForSlot(uint8_t slotIndex, const DrumSampleData* sampleData) override;
        void setDrumSampleSlotVolume(uint8_t slotIndex, float value) override;
        void setDrumSampleSlotTuneSemitones(uint8_t slotIndex, float value) override;
        void setDrumSampleSlotStartOffset(uint8_t slotIndex, float value) override;
        void setDrumSampleSlotDecay(uint8_t slotIndex, float value) override;
        void setDrumSampleSlotVelocitySensitivity(uint8_t slotIndex, float value) override;

    private:
        AudioEngine& engine;
    };

    // Forwarding wrappers (kept for test access and internal use)
    uint16_t getMaxAlgorithmIdForTrack(uint8_t trackIndex)
    {
        return commandProcessor.getMaxAlgorithmIdForTrack(trackIndex);
    }

    uint8_t getMaxSynthPresetIdForTrack(uint8_t trackIndex)
    {
        return SynthCatalog::getMaxPresetIdForTrack(trackIndex);
    }

    GenerativeAlgorithm* getTrackAlgorithm(uint8_t trackIndex, uint16_t algorithmId)
    {
        return commandProcessor.getTrackAlgorithm(trackIndex, algorithmId, builtinTrackAlgorithms);
    }

    void processCommands()
    {
        commandProcessor.process();
    }

    void initializeAlgorithmMap();
    void applyDrumKitPreset(uint8_t presetId);
    void applyMelodicSamplePreset(uint8_t trackIndex, uint8_t presetId);
    void applySoundPresetEffectSlots(uint8_t trackIndex, uint8_t presetId);
    void applySoundPreset(uint8_t trackIndex, uint8_t presetId, bool manualOverride);
    void resetTrackRuntime(uint8_t trackIndex);
    void updateArrangementState(bool playing, bool isNewBar, uint16_t barNumber);
    int getArrangementProgressionStep(double blockStartBeats) const;
    void updateSpectrum(const float* source, int numSamples, MeterData& meterData);
    void renderMetronome(float* const* outputChannelData, int numOutputChannels, int numSamples);

    AppState& appState;
    CommandQueue& commandQueue;
    MeterQueue& meterQueue;
    DrumSampleLibrary* drumSampleLibrary = nullptr;
    MelodicSampleLibrary* melodicSampleLibrary = nullptr;
    AlgorithmPresetRegistry* algorithmRegistry = nullptr;

    // Processors
    EffectProcessor effectProcessor;
    CommandDelegate commandDelegate;
    CommandProcessor commandProcessor;

    juce::AudioDeviceManager deviceManager;
    bool audioDeviceEnabled = true;
    Transport transport;
    double sampleRate = 44100.0;
    bool arrangementAnchorInitialized = false;
    uint8_t arrangementAnchoredSection = 0;
    uint8_t arrangementAnchoredChainPosition = 0;
    uint16_t arrangementSectionStartBar = 0;

    juce::dsp::Oscillator<float> clickOsc;
    float clickEnvelope = 0.0f;

    DrumEngine drumEngine;
    BassEngine bassEngine;
    ChordEngine chordEngine;
    LeadEngine leadEngine;

    FourOnFloor fourOnFloor;
    Breakbeat breakbeat;
    EuclideanRhythm euclideanRhythm;
    WalkingBass walkingBass;
    SyncBass syncBass;
    BlockChords blockChords;
    SyncStabs syncStabs;
    Arpeggiator arpeggiator;
    MarkovMelody markovMelody;
    DnBBreaks dnbBreaks;
    AfroClaveGroove afroClaveGroove;
    HouseShuffleGroove houseShuffleGroove;
    TrapHalfTimeGroove trapHalfTimeGroove;
    GlitchPulseGroove glitchPulseGroove;
    TechnoRumbleGroove technoRumbleGroove;
    JerseyClubGroove jerseyClubGroove;
    BrokenStepperGroove brokenStepperGroove;
    PolyrhythmTomGroove polyrhythmTomGroove;
    ElectroBreaksGroove electroBreaksGroove;
    GarageSwingGroove garageSwingGroove;
    LatinPercGroove latinPercGroove;
    MinimalClicksGroove minimalClicksGroove;
    DubSkankGroove dubSkankGroove;
    Footwork160Groove footwork160Groove;
    HalfstepGroove halfstepGroove;
    IndustrialGroove industrialGroove;

    Sub808Bass sub808Bass;
    UKGarageBass ukGarageBass;
    TumbaoBass tumbaoBass;
    DubPedalBass dubPedalBass;
    ReesePulseBass reesePulseBass;
    MotifBass motifBass;
    AcidTripletBass acidTripletBass;
    GlideCounterBass glideCounterBass;
    PulseChopBass pulseChopBass;
    OctaveBounceBass octaveBounceBass;
    ReggaetonSubBass reggaetonSubBass;
    ElectroFunkBass electroFunkBass;
    MinimalDroneBass minimalDroneBass;
    BrokenOctaveBass brokenOctaveBass;
    StepperDubBass stepperDubBass;
    FunkPopBass funkPopBass;
    NeuroWobbleBass neuroWobbleBass;
    ClaveBass claveBass;

    HousePianoStabs housePianoStabs;
    AmbientPadSwells ambientPadSwells;
    NeoSoulVoicings neoSoulVoicings;
    TranceGateChords tranceGateChords;
    QuartalComping quartalComping;
    VoiceCloudChords voiceCloudChords;
    GospelLiftChords gospelLiftChords;
    DetuneStackChords detuneStackChords;
    BrokenStrumChords brokenStrumChords;
    PulseClusterChords pulseClusterChords;
    DubSkankChords dubSkankChords;
    MinimalPluckChords minimalPluckChords;
    RNBKeyChords rnbKeyChords;
    SuspendedPadChords suspendedPadChords;
    CinematicHitChords cinematicHitChords;
    FifthDroneChords fifthDroneChords;
    GarageOrganChords garageOrganChords;
    PolychordChords polychordChords;

    TranceContourLead tranceContourLead;
    RaveStabLead raveStabLead;
    AfroCallResponseLead afroCallResponseLead;
    CinematicSparseLead cinematicSparseLead;
    EuclideanLeadGate euclideanLeadGate;
    PhraseMutatorLead phraseMutatorLead;
    GlideRunLead glideRunLead;
    MicroMotifLead microMotifLead;
    WideIntervalLead wideIntervalLead;
    TripletRushLead tripletRushLead;
    PentatonicHookLead pentatonicHookLead;
    AcidLineLead acidLineLead;
    DubEchoLead dubEchoLead;
    GarageVoxLead garageVoxLead;
    MinimalPingLead minimalPingLead;
    OrnamentRunLead ornamentRunLead;
    SyncopatedPluckLead syncopatedPluckLead;
    LydianFloatLead lydianFloatLead;

    std::array<std::array<GenerativeAlgorithm*, AlgorithmCatalog::kAlgorithmsPerTrack>, TrackCount> builtinTrackAlgorithms{};

    std::array<juce::MidiBuffer, 4> trackMidiBuffers;
    std::array<juce::AudioBuffer<float>, 4> trackAudioBuffers;
    juce::AudioBuffer<float> masterMixBuffer;
    uint64_t activeNotes[4][2] = {{0}};

    juce::dsp::FFT analyzerFft{AnalyzerFftOrder};
    std::array<float, AnalyzerFftSize> analyzerFifo{};
    std::array<float, AnalyzerFftSize * 2> analyzerFftData{};
    std::array<float, AnalyzerFftSize> analyzerWindow{};
    std::array<float, kSpectrumBinCount> latestSpectrumBins{};
    int analyzerFifoIndex = 0;
    uint32_t analyzerFrameCounter = 0;

    struct ProfilingAccumulator
    {
        double callbackMsSum = 0.0;
        double callbackMsPeak = 0.0;
        double callbackUtilizationSum = 0.0;
        double callbackUtilizationPeak = 0.0;
        double commandsMsSum = 0.0;
        double generationMsSum = 0.0;
        double trackFxMsSum = 0.0;
        double masterFxMsSum = 0.0;
        double meteringMsSum = 0.0;
        uint32_t callbacks = 0;
    };

    struct ProfilingSnapshot
    {
        bool valid = false;
        uint32_t callbacks = 0;
        float bufferDurationMs = 0.0f;
        float callbackMsAvg = 0.0f;
        float callbackMsPeak = 0.0f;
        float callbackUtilizationAvg = 0.0f;
        float callbackUtilizationPeak = 0.0f;
        float commandsMsAvg = 0.0f;
        float generationMsAvg = 0.0f;
        float trackFxMsAvg = 0.0f;
        float masterFxMsAvg = 0.0f;
        float meteringMsAvg = 0.0f;
    };

    bool profilingEnabled = false;
    bool profilingTraceEnabled = false;
    uint32_t profilingReportIntervalCallbacks = 128;
    ProfilingAccumulator profilingAccumulator{};
    ProfilingSnapshot latestProfilingSnapshot{};
    std::atomic<bool> tracedFirstCallback{false};

    // --- Recording ---
    std::unique_ptr<cendance::AudioCaptureBus> captureBus;
    cendance::FileRecorder fileRecorder;

    // --- Streaming ---
    cendance::StreamSink streamSink;

    // --- Groove / Swing / Humanization ---
    cendance::GrooveProcessor grooveProcessor;
};
