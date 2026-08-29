#pragma once

#include "AppState.h"
#include "CommandQueue.h"
#include "DrumSampleLibrary.h"
#include "MelodicSampleLibrary.h"
#include "MeterQueue.h"
#include "../Audio/AudioEngine.h"

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class JuceRuntime final {
public:
    JuceRuntime(AppState& appState,
                CommandQueue& commandQueue,
                MeterQueue& meterQueue,
                DrumSampleLibrary& drumSampleLibrary,
                MelodicSampleLibrary& melodicSampleLibrary,
                std::string deviceName);
    ~JuceRuntime();

    void start();
    void stop();

    // --- Recording ---
    bool startRecording(const std::string& filePath, cendance::FileRecorder::Format format = cendance::FileRecorder::Format::WavF32);
    void stopRecording();
    bool isRecording() const;
    cendance::FileRecorder::Status getRecordingStatus() const;

    // --- Streaming ---
    bool startStreaming(cendance::StreamSink::SinkFn sink, cendance::StreamSink::Format format = cendance::StreamSink::Format::F32LE);
    void stopStreaming();
    bool isStreaming() const;
    cendance::StreamSink::Status getStreamingStatus() const;

private:
    AppState& appState;
    CommandQueue& commandQueue;
    MeterQueue& meterQueue;
    DrumSampleLibrary& drumSampleLibrary;
    MelodicSampleLibrary& melodicSampleLibrary;
    std::string deviceName;
    juce::MessageManager* messageManager = nullptr;
    std::thread thread;
    std::mutex readyMutex;
    std::condition_variable readyCv;
    bool ready = false;
    std::atomic<bool> exitRequested{false};
    std::unique_ptr<AudioEngine> audioEngine;
};
