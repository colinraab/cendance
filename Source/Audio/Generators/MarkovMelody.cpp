#include "MarkovMelody.h"
#include "GeneratorUtils.h"
#include <random>


void MarkovMelody::processMidi(juce::MidiBuffer& buffer,
                               double playheadBeats,
                               double blockLengthBeats,
                               int blockSamples,
                               const Scale& scale,
                               int rootNote,
                               float density,
                               float complexity)
{
    double grid = (density > 0.5f) ? 0.25 : 0.5;
    int sampleIdx;
    
    double start = playheadBeats;
    double end = playheadBeats + blockLengthBeats;
    
    double nextGrid = std::ceil(start / grid) * grid;
    if (std::abs(start - std::floor(start / grid) * grid) < 1e-6) {
        nextGrid = start;
    }
    
    while (nextGrid < end) {
        if (density > 0.2f || std::fmod(nextGrid, 1.0) == 0.0) { // Keep downbeats at low density
            double fraction = (nextGrid - start) / blockLengthBeats;
            sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
            
            // Generate next degree
            // Higher complexity = more random jumps
            std::mt19937 gen(static_cast<unsigned int>(nextGrid * 1000));
            std::uniform_real_distribution<float> dist(0.f, 1.f);
            
            if (dist(gen) < complexity) {
                // Large jump
                int jump = (dist(gen) > 0.5f) ? 2 : -2;
                if (dist(gen) > 0.8f) jump *= 2; 
                currentDegree_ += jump;
            } else {
                // Stepwise or stay
                if (dist(gen) > 0.5f) {
                    currentDegree_ += (dist(gen) > 0.5f) ? 1 : -1;
                }
            }
            
            // Restrict bounds
            if (currentDegree_ < -7) currentDegree_ += 7;
            if (currentDegree_ > 7) currentDegree_ -= 7;
            
            int note = scale.getDegree(currentDegree_, 4); // Root in octave 4
            
            addScheduledNoteAtSample(buffer, 13, note, 90, sampleIdx, grid * 0.8, playheadBeats, blockLengthBeats, blockSamples);
        }
        
        nextGrid += grid;
    }
}

void MarkovMelody::reset() {
    clearPendingNoteOffs();
    currentDegree_ = 0;
}
