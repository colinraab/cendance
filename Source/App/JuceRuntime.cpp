#include "JuceRuntime.h"

#include "AlgorithmPresetRegistry.h"

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

using namespace juce;

JuceRuntime::JuceRuntime(AppState& appState,
                         CommandQueue& commandQueue,
                         MeterQueue& meterQueue,
                         DrumSampleLibrary& drumSampleLibrary,
                         MelodicSampleLibrary& melodicSampleLibrary,
                         std::string deviceName)
    : appState(appState),
      commandQueue(commandQueue),
      meterQueue(meterQueue),
      drumSampleLibrary(drumSampleLibrary),
      melodicSampleLibrary(melodicSampleLibrary),
      deviceName(std::move(deviceName)) {
}

JuceRuntime::~JuceRuntime() {
    stop();
}

void JuceRuntime::start() {
    messageManager = juce::MessageManager::getInstance();
    exitRequested.store(false, std::memory_order_relaxed);

    thread = std::thread([this]() {
        messageManager->setCurrentThreadAsMessageThread();

        const bool disableAudio = std::getenv("CENDANCE_DISABLE_AUDIO") != nullptr;
        audioEngine.reset(new AudioEngine(appState,
                                          commandQueue,
                                          meterQueue,
                                          juce::String(deviceName),
                                          !disableAudio,
                                          &drumSampleLibrary,
                                          &melodicSampleLibrary,
                                          &globalAlgorithmPresetRegistry()));

        {
            std::lock_guard<std::mutex> lock(readyMutex);
            ready = true;
        }
        readyCv.notify_one();

        while (!exitRequested.load(std::memory_order_relaxed)) {
            juce::Thread::sleep(20);
        }

        audioEngine.reset();
        juce::DeletedAtShutdown::deleteAll();
    });

    {
        std::unique_lock<std::mutex> lock(readyMutex);
        readyCv.wait(lock, [this]() { return ready; });
    }
}

void JuceRuntime::stop() {
    if (thread.joinable()) {
        exitRequested.store(true, std::memory_order_relaxed);
        messageManager->stopDispatchLoop();
        thread.join();
    }
    if (messageManager != nullptr) {
        juce::MessageManager::deleteInstance();
        messageManager = nullptr;
    }
}

// --- Recording ---

bool JuceRuntime::startRecording(const std::string& filePath, cendance::FileRecorder::Format format) {
    if (!audioEngine) return false;
    return audioEngine->startRecording(filePath, format);
}

void JuceRuntime::stopRecording() {
    if (audioEngine) audioEngine->stopRecording();
}

bool JuceRuntime::isRecording() const {
    if (!audioEngine) return false;
    return audioEngine->isRecording();
}

cendance::FileRecorder::Status JuceRuntime::getRecordingStatus() const {
    if (!audioEngine) return cendance::FileRecorder::Status{};
    return audioEngine->getRecordingStatus();
}

// --- Streaming ---

bool JuceRuntime::startStreaming(cendance::StreamSink::SinkFn sink, cendance::StreamSink::Format format) {
    if (!audioEngine) return false;
    return audioEngine->startStreaming(std::move(sink), format);
}

void JuceRuntime::stopStreaming() {
    if (audioEngine) audioEngine->stopStreaming();
}

bool JuceRuntime::isStreaming() const {
    if (!audioEngine) return false;
    return audioEngine->isStreaming();
}

cendance::StreamSink::Status JuceRuntime::getStreamingStatus() const {
    if (!audioEngine) return cendance::StreamSink::Status{};
    return audioEngine->getStreamingStatus();
}
