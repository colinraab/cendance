#include "Breakbeat.h"
#include "GeneratorUtils.h"
#include <random>


void Breakbeat::processMidi(juce::MidiBuffer& buffer,
                            double playheadBeats,
                            double blockLengthBeats,
                            int blockSamples,
                            const Scale& scale,
                            int rootNote,
                            float density,
                            float complexity)
{
    double start = playheadBeats * 4.0;
    double end = (playheadBeats + blockLengthBeats) * 4.0;
    const bool isGhostNote = complexity > 0.25f;
    const float ghostProb = juce::jlimit(0.0f, 1.0f, complexity * 0.95f);

    for (int step = std::ceil(start); step < end; ++step) {
        int posInBar = step % 16;
        if (posInBar < 0) posInBar += 16;
        
        double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        
        bool kick = false;
        bool snare = false;
        const bool hat = (density > 0.22f && (posInBar % 2 == 0))
                         || (density > 0.58f && (posInBar % 2 == 1));
        const bool openHat = density > 0.66f && (posInBar == 7 || posInBar == 15);
        const int hatVel = (posInBar % 4 == 0) ? 78 : 44;
        
        // Classic break foundation, with complexity adding funkier displacements.
        if (posInBar == 0 || posInBar == 5 || (posInBar == 10 && complexity > 0.25f)) kick = true;
        if (complexity > 0.52f && (posInBar == 2 || posInBar == 13)) kick = true;
        if (complexity > 0.82f && posInBar == 15) kick = true;
        if (posInBar == 4 || posInBar == 12) snare = true;
        
        // Ghost snares
        if (isGhostNote && (posInBar == 3 || posInBar == 6 || posInBar == 7 || posInBar == 9 || posInBar == 11 || posInBar == 14)) {
            // Pseudo-random based on step
            std::mt19937 gen(step * 1337);
            std::uniform_real_distribution<float> dist(0.f, 1.f);
            if (dist(gen) < ghostProb * density) {
                GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 34 + static_cast<int>(complexity * 26.0f), sampleIdx, blockSamples);
            }
        }
        
        if (kick) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, (posInBar == 0) ? 104 : 88, sampleIdx, blockSamples);
        if (snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 100, sampleIdx, blockSamples);
        if (hat && !kick && !snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, hatVel, sampleIdx, blockSamples);
        if (openHat && !snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 66, sampleIdx, blockSamples);
    }
}

void Breakbeat::reset() {}
