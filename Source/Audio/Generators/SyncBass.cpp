#include "SyncBass.h"
#include "GeneratorUtils.h"
#include <random>


void SyncBass::processMidi(juce::MidiBuffer& buffer,
                           double playheadBeats,
                           double blockLengthBeats,
                           int blockSamples,
                           const Scale& scale,
                           int rootNote,
                           float density,
                           float complexity)
{
    // 16th note syncopation
    double start = playheadBeats * 4.0;
    double end = (playheadBeats + blockLengthBeats) * 4.0;
    
    for (int step = std::ceil(start); step < end; ++step) {
        int posInBar = step % 16;
        if (posInBar < 0) posInBar += 16;
        
        bool hit = false;
        
        // Classic syncopation pattern
        if (posInBar == 0 || posInBar == 3 || posInBar == 6 || posInBar == 10 || posInBar == 14) {
            hit = true;
        }
        
        if (hit && (density > 0.2f || posInBar == 0 || posInBar == 10)) {
            double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
            int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
            
            int octave = 2;
            if (complexity > 0.5f && (posInBar == 6 || posInBar == 14)) {
                octave = 3; // octave jumps
            }
            
            int note = scale.getDegree(0, octave); // Simplest - root note
            addScheduledNoteAtSample(buffer, 11, note, 100, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);
        }
    }
}

void SyncBass::reset() { clearPendingNoteOffs(); }
