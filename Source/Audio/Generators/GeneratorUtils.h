#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

// Drum mappings (General MIDI standardish)
constexpr int MAPPING_KICK = 36;
constexpr int MAPPING_SNARE = 38;
constexpr int MAPPING_CLOSED_HAT = 42;
constexpr int MAPPING_OPEN_HAT = 46;

namespace GeneratorUtils {
    
    // Checks if the playhead crosses a specific grid interval (in beats) within this block.
    // If true, returns the sample index of the hit (0 to blockSamples - 1).
    inline bool checkHit(double playhead, double length, double grid, int blockSamples, int& outSampleIdx, double offset = 0.0) {
        double start = playhead;
        double end = playhead + length;
        
        // Find the next grid point with the given offset
        double relStart = start - offset;
        double nextGrid = std::ceil(relStart / grid) * grid + offset;
        
        // If the start itself is exactly on the grid (rare with floating point, but possible)
        if (std::abs(relStart - std::floor(relStart / grid) * grid) < 1e-6) {
            nextGrid = start;
        }

        if (nextGrid >= start && nextGrid < end) {
            double fraction = (nextGrid - start) / length;
            outSampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
            return true;
        }
        return false;
    }

    inline void addNoteEvent(juce::MidiBuffer& buffer,
                             int channel,
                             int note,
                             int velocity,
                             int samplePos,
                             int blockSamples,
                             int durationSamples = 100) {
        if (blockSamples <= 0) {
            return;
        }

        const int noteOnSample = juce::jlimit(0, blockSamples - 1, samplePos);
        const int noteOffSample = juce::jlimit(noteOnSample, blockSamples - 1, noteOnSample + durationSamples);
        buffer.addEvent(juce::MidiMessage::noteOn(channel, note, static_cast<juce::uint8>(velocity)), noteOnSample);
        buffer.addEvent(juce::MidiMessage::noteOff(channel, note, static_cast<juce::uint8>(0)), noteOffSample);
    }
}
