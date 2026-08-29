#include "Arpeggiator.h"
#include "GeneratorUtils.h"


void Arpeggiator::processMidi(juce::MidiBuffer& buffer,
                              double playheadBeats,
                              double blockLengthBeats,
                              int blockSamples,
                              const Scale& scale,
                              int rootNote,
                              float density,
                              float complexity)
{
    // Density controls note rate (8ths, 16ths, 32nds)
    double grid = 0.5;
    if (density > 0.3f) grid = 0.25;
    if (density > 0.8f) grid = 0.125;
    
    int sampleIdx;
    
    // Evaluate across block length
    double start = playheadBeats;
    double end = playheadBeats + blockLengthBeats;
    
    double nextGrid = std::ceil(start / grid) * grid;
    if (std::abs(start - std::floor(start / grid) * grid) < 1e-6) {
        nextGrid = start;
    }
    
    while (nextGrid < end) {
        double fraction = (nextGrid - start) / blockLengthBeats;
        sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        
        // Pattern based on complexity
        int direction = (complexity < 0.3f) ? 0 : (complexity < 0.7f) ? 1 : 2; // Up, UpDown, Random
        int octaves = (complexity > 0.5f) ? 2 : 1;
        
        auto tones = scale.getChordTones(0, 4, true); // 4-note chord in octave 4
        
        int noteIndex = 0;
        
        int step = static_cast<int>(nextGrid / grid);
        if (direction == 0) { // Up
            noteIndex = step % (tones.size() * octaves);
        } else if (direction == 1) { // UpDown
            int cycle = tones.size() * octaves * 2 - 2;
            int pos = step % cycle;
            if (pos >= tones.size() * octaves) pos = cycle - pos;
            noteIndex = pos;
        } else { // Random
            noteIndex = (step * 7) % (tones.size() * octaves); // Pseudo-random hash
        }
        
        int octaveAdd = noteIndex / tones.size();
        noteIndex = noteIndex % tones.size();
        
        int note = tones[noteIndex] + octaveAdd * 12;
        
        addScheduledNoteAtSample(buffer, 13, note, 100, sampleIdx, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);
        
        nextGrid += grid;
    }
}

void Arpeggiator::reset() {
    clearPendingNoteOffs();
    stepCount_ = 0;
}
