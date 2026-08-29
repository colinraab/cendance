#include "BlockChords.h"
#include "GeneratorUtils.h"


void BlockChords::processMidi(juce::MidiBuffer& buffer,
                              double playheadBeats,
                              double blockLengthBeats,
                              int blockSamples,
                              const Scale& scale,
                              int rootNote,
                              float density,
                              float complexity)
{
    // Whole/half note chords on downbeats
    double grid = (density > 0.5f) ? 2.0 : 4.0; 
    int sampleIdx;
    
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, grid, blockSamples, sampleIdx)) {
        bool addSeventh = complexity > 0.3f;
        bool addNinth = complexity > 0.7f;
        
        auto tones = scale.getChordTones(0 /* root degree */, 3 /* octave */, addSeventh);
        
        if (addNinth) {
            tones.push_back(scale.getDegree(8, 3)); // 9th
        }
        
        for (int note : tones) {
            addScheduledNoteAtSample(buffer, 12, note, 80, sampleIdx, grid - 0.1,
                                     playheadBeats, blockLengthBeats, blockSamples);
        }
    }
}

void BlockChords::reset() { clearPendingNoteOffs(); }
