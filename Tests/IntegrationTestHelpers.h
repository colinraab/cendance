#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// RAII temp directory that deletes itself on destruction
struct TempDir {
    juce::File dir;
    bool cleanup;

    TempDir(const std::string& prefix = "cendance_test") {
        auto tmp = juce::File::getSpecialLocation(juce::File::tempDirectory);
        dir = tmp.getChildFile(juce::String(prefix + "_" + std::to_string(juce::Time::currentTimeMillis())));
        dir.createDirectory();
        cleanup = true;
    }

    ~TempDir() {
        if (cleanup && dir.exists())
            dir.deleteRecursively();
    }

    // Non-copyable, non-movable
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;
    TempDir(TempDir&&) = delete;
    TempDir& operator=(TempDir&&) = delete;
};

// Generate a simple sine wave WAV file at the given path
// Returns true on success
inline bool generateTestWav(const std::string& path, double sampleRate = 44100.0,
                             int numSamples = 44100, double frequency = 440.0) {
    juce::File file(path);
    auto parent = file.getParentDirectory();
    if (!parent.exists())
        parent.createDirectory();

    juce::WavAudioFormat format;
    std::unique_ptr<juce::OutputStream> outputStream(new juce::FileOutputStream(file));
    auto options = juce::AudioFormatWriterOptions()
        .withSampleRate(sampleRate)
        .withNumChannels(1)
        .withBitsPerSample(16);
    auto writer = format.createWriterFor(outputStream, options);
    if (!writer)
        return false;

    juce::AudioBuffer<float> buffer(1, numSamples);
    for (int i = 0; i < numSamples; ++i) {
        buffer.setSample(0, i, static_cast<float>(0.5 * std::sin(2.0 * juce::MathConstants<double>::pi * frequency * i / sampleRate)));
    }
    return writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
}

// Read entire file contents as string
inline std::string readFileContents(const juce::File& file) {
    return file.loadFileAsString().toStdString();
}

// Check if a file exists and has non-zero size
inline bool fileExistsNonEmpty(const juce::File& path) {
    return path.existsAsFile() && path.getSize() > 0;
}

// Helper to safely get a string property from a juce::var (avoids macro issues with juce::var())
inline std::string varGetString(const juce::var& v, const char* key) {
    return v.getProperty(key, juce::var()).toString().toStdString();
}

// Helper to safely get a bool property from a juce::var
inline bool varGetBool(const juce::var& v, const char* key) {
    return static_cast<bool>(v.getProperty(key, juce::var()));
}

// Helper to safely get an int property from a juce::var
inline int varGetInt(const juce::var& v, const char* key) {
    return static_cast<int>(v.getProperty(key, juce::var()));
}

// Helper to check if a var property exists and is not empty
inline bool varHasProperty(const juce::var& v, const char* key) {
    auto prop = v.getProperty(key, juce::var());
    if (prop.isVoid()) return false;
    if (prop.isString()) return !prop.toString().isEmpty();
    return true;
}

// Helper to check type of a var property
inline bool varIsInt(const juce::var& v, const char* key) {
    return v.getProperty(key, juce::var()).isInt();
}
inline bool varIsString(const juce::var& v, const char* key) {
    return v.getProperty(key, juce::var()).isString();
}
inline bool varIsArray(const juce::var& v, const char* key) {
    return v.getProperty(key, juce::var()).isArray();
}
inline juce::var varGet(const juce::var& v, const char* key) {
    return v.getProperty(key, juce::var());
}

inline void isolateToSConfigForTests() {
    static TempDir dir("cendance_tos_test");
    setenv("CENDANCE_CONFIG_DIR", dir.dir.getFullPathName().toRawUTF8(), 1);
}
