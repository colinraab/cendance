#pragma once

#include <vector>
#include <array>

class Scale {
public:
    enum class Type { Major, NaturalMinor, HarmonicMinor, Dorian, Mixolydian, Phrygian };

    Scale(int rootNote, Type type);

    // Quantize any MIDI note to the nearest scale degree
    int quantize(int midiNote) const;

    // Get scale degree (0-indexed) as MIDI note in given octave
    int getDegree(int degree, int octave) const;

    // Get chord tones for a given degree (triad or 7th)
    std::vector<int> getChordTones(int degree, int octave, bool seventh = false) const;

private:
    int root_;
    std::array<int, 7> intervals_;  // Semitone intervals from root
};
