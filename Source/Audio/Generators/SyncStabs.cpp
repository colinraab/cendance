#include "SyncStabs.h"
#include "GeneratorUtils.h"
#include <random>


void SyncStabs::processMidi(juce::MidiBuffer& buffer,
                            double playheadBeats,
                            double blockLengthBeats,
                            int blockSamples,
                            const Scale& scale,
                            int rootNote,
                            float density,
                            float complexity)
{
    // Staccato stabs, syncopated
    double start = playheadBeats * 4.0;
    double end = (playheadBeats + blockLengthBeats) * 4.0;
    
    for (int step = std::ceil(start); step < end; ++step) {
        int posInBar = step % 16;
        if (posInBar < 0) posInBar += 16;
        
        bool hit = false;
        
        // syncopated
        if (posInBar == 2 || posInBar == 5 || posInBar == 9 || posInBar == 12) {
            hit = true;
        }
        
        const bool anchor = posInBar == 2;
        if (anchor || (hit && density > 0.3f)) {
            double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
            int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
            
            bool addSeventh = complexity > 0.5f;
            auto tones = scale.getChordTones(0, 3, addSeventh);
            
            for (int note : tones) {
                addScheduledNoteAtSample(buffer, 12, note, 90, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);
            }
        }
    }
}

void SyncStabs::reset() { clearPendingNoteOffs(); }
