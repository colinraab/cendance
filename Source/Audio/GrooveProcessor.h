#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cstdint>
#include <random>

namespace cendance {

// Transforms MIDI buffers with swing, velocity humanization, and timing jitter.
// Designed to be called from the audio callback after algorithms generate MIDI
// but before the MIDI is rendered to audio.
//
// Swing: shifts even 8th notes (beat positions 1.0, 3.0, 5.0, ...) later.
//   swingAmount 0.0 = no shift, 0.5 = triplet feel, 1.0 = max shift (half an 8th).
//   The shift is computed as: shift = swingAmount * 0.5 * eighthNoteInSamples.
//
// Velocity Humanization: random ±velocity offset per note.
//   velocityHumanize 0.0 = no change, 1.0 = ±50 velocity range.
//
// Timing Jitter: random ±sample offset per note.
//   timingJitter 0.0 = no change, 1.0 = ±10ms max.
class GrooveProcessor {
public:
    void prepare(double sampleRate, int maxBlockSize);

    // Apply groove transformations to a MIDI buffer in-place.
    // playheadBeats: transport position at start of this block
    // blockLengthBeats: length of this block in beats
    // numSamples: number of audio samples in this block
    void apply(juce::MidiBuffer& midi,
               double playheadBeats,
               double blockLengthBeats,
               int numSamples,
               float swingAmount,
               float velocityHumanize,
               float timingJitter);

    void reset();

private:
    static constexpr double kEighthNoteDivisor = 2.0; // 8th note = beat / 2
    static constexpr float kMaxVelocityOffset = 50.0f;
    static constexpr double kMaxJitterMs = 10.0;

    double sampleRate_ = 44100.0;
    int maxBlockSize_ = 512;

    // Reusable buffer for transformed events
    juce::MidiBuffer outputBuffer_;

    // Random generator (seeded once, not per-call)
    std::mt19937 rng_;
};

} // namespace cendance
