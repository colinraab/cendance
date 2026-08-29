#include "EuclideanRhythm.h"
#include "GeneratorUtils.h"
#include <vector>


// Bjorklund's algorithm implementation
static std::vector<bool> computeEuclidean(int pulses, int steps) {
    if (steps == 0) return {};
    if (pulses >= steps) return std::vector<bool>(steps, true);
    if (pulses == 0) return std::vector<bool>(steps, false);

    std::vector<std::vector<bool>> groups;
    for (int i = 0; i < steps; ++i) {
        if (i < pulses) groups.push_back({true});
        else groups.push_back({false});
    }

    int numRem = steps - pulses;
    int numGroups = pulses;
    while (numRem > 1) {
        int i = 0;
        int max_i = std::min(numGroups, numRem);
        for (; i < max_i; ++i) {
            groups[i].insert(groups[i].end(), groups[groups.size() - 1].begin(), groups[groups.size() - 1].end());
            groups.pop_back();
        }
        numRem = numGroups - max_i;
        numGroups = max_i;
        if (numRem <= 0) break;
    }

    std::vector<bool> result;
    for (const auto& group : groups) {
        result.insert(result.end(), group.begin(), group.end());
    }
    return result;
}

void EuclideanRhythm::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale& scale,
                                  int rootNote,
                                  float density,
                                  float complexity)
{
    int steps = 16;
    int pulses = juce::jlimit(1, 16, 2 + static_cast<int>(density * 10.0f));
    int snarePulses = juce::jlimit(1, 8, 1 + static_cast<int>(complexity * 5.0f));
    int hatPulses = juce::jlimit(2, 16, 4 + static_cast<int>(density * 8.0f));
    int rotation = static_cast<int>(complexity * 5.0f);
    
    std::vector<bool> pattern = computeEuclidean(pulses, steps);
    std::vector<bool> snarePattern = computeEuclidean(snarePulses, 8);
    std::vector<bool> hatPattern = computeEuclidean(hatPulses, steps);
    
    double start = playheadBeats * 4.0;
    double end = (playheadBeats + blockLengthBeats) * 4.0;
    
    for (int step = std::ceil(start); step < end; ++step) {
        int posInBar = step % steps;
        if (posInBar < 0) posInBar += steps;
        
        int rotPos = (posInBar + steps - rotation) % steps;
        
        double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
        int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        const int snarePos = ((posInBar / 2) + 8 - rotation) % 8;
        const bool kick = pattern[rotPos];
        const bool snare = complexity > 0.2f && snarePattern[snarePos] && (posInBar % 2 == 0);
        const bool hat = density > 0.24f && hatPattern[posInBar] && !kick;
        const bool openHat = complexity > 0.62f && density > 0.45f && (posInBar == 6 || posInBar == 14);

        if (kick) {
            const int velocity = (posInBar == 0 || posInBar == 8) ? 100 : 78;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, velocity, sampleIdx, blockSamples);
        }
        if (snare && !kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 54 + static_cast<int>(complexity * 36.0f), sampleIdx, blockSamples);
        }
        if (hat && !snare) {
            const int velocity = (posInBar % 4 == 0) ? 58 : 38;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, velocity, sampleIdx, blockSamples);
        }
        if (openHat && !kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 62, sampleIdx, blockSamples);
        }
    }
}

void EuclideanRhythm::reset() {}
