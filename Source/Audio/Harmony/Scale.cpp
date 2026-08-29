#include "Scale.h"
#include <algorithm>

Scale::Scale(int rootNote, Type type) : root_(rootNote % 12) {
    switch (type) {
        case Type::Major:         intervals_ = {0, 2, 4, 5, 7, 9, 11}; break;
        case Type::NaturalMinor:  intervals_ = {0, 2, 3, 5, 7, 8, 10}; break;
        case Type::HarmonicMinor: intervals_ = {0, 2, 3, 5, 7, 8, 11}; break;
        case Type::Dorian:        intervals_ = {0, 2, 3, 5, 7, 9, 10}; break;
        case Type::Mixolydian:    intervals_ = {0, 2, 4, 5, 7, 9, 10}; break;
        case Type::Phrygian:      intervals_ = {0, 1, 3, 5, 7, 8, 10}; break;
    }
}

int Scale::quantize(int midiNote) const {
    int octave = midiNote / 12;
    int noteClass = midiNote % 12;
    
    int closestDiff = 12;
    int closestNote = noteClass;
    
    for (int interval : intervals_) {
        int targetClass = (root_ + interval) % 12;
        int diff = std::abs(noteClass - targetClass);
        if (diff > 6) diff = 12 - diff; // Circular distance
        
        if (diff < closestDiff) {
            closestDiff = diff;
            closestNote = targetClass;
        }
    }
    
    // Check if quantizing crossed an octave boundary relative to C
    int rounded = (octave * 12) + closestNote;
    if (std::abs(rounded - midiNote) > 6) {
        if (rounded > midiNote) rounded -= 12;
        else rounded += 12;
    }
    
    return std::clamp(rounded, 0, 127);
}

int Scale::getDegree(int degree, int octave) const {
    int normalizedDegree = degree % 7;
    if (normalizedDegree < 0) normalizedDegree += 7;
    int octaveOffset = degree / 7;
    if (degree < 0 && degree % 7 != 0) octaveOffset--;
    
    int note = root_ + intervals_[normalizedDegree] + ((octave + octaveOffset) * 12);
    return std::clamp(note, 0, 127);
}

std::vector<int> Scale::getChordTones(int degree, int octave, bool seventh) const {
    std::vector<int> tones;
    int numNotes = seventh ? 4 : 3;
    
    for (int i = 0; i < numNotes; ++i) {
        tones.push_back(getDegree(degree + (i * 2), octave));
    }
    
    return tones;
}
