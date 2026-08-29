#include "WalkingBass.h"
#include "GeneratorUtils.h"


void WalkingBass::processMidi(juce::MidiBuffer& buffer,
                              double playheadBeats,
                              double blockLengthBeats,
                              int blockSamples,
                              const Scale& scale,
                              int rootNote,
                              float density,
                              float complexity)
{
    // Walking bass typically hits on every beat (quarter notes)
    // Density maps to whether passing 8th notes are used.
    double grid = (density > 0.5f) ? 0.5 : 1.0;
    int sampleIdx;
    
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, grid, blockSamples, sampleIdx)) {
        double currentBeat = std::floor(playheadBeats * (1.0 / grid)) * grid;
        int stepBase = static_cast<int>(currentBeat);
        
        int degreeOffset = stepBase % 4; // basic ascending/descending walk
        
        int note = scale.getDegree(degreeOffset, 2); // octave 2
        
        // Chromatic passing tones on upbeat if complexity > 0.5
        if (complexity > 0.5f && std::fmod(currentBeat, 1.0) == 0.5) {
            note += 1; // shift chromatic
        }
        
        addScheduledNoteAtSample(buffer, 11, note, 100, sampleIdx, grid * 0.85, playheadBeats, blockLengthBeats, blockSamples);
        lastNote_ = note;
    }
}

void WalkingBass::reset() {
    clearPendingNoteOffs();
    lastNote_ = -1;
}
