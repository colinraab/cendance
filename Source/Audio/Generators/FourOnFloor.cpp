#include "FourOnFloor.h"
#include "GeneratorUtils.h"


void FourOnFloor::processMidi(juce::MidiBuffer& buffer,
                              double playheadBeats,
                              double blockLengthBeats,
                              int blockSamples,
                              const Scale& scale,
                              int rootNote,
                              float density,
                              float complexity)
{
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;

    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        int pos = step % 16;
        if (pos < 0) {
            pos += 16;
        }

        const double fraction = (static_cast<double>(step) * 0.25 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        const bool mainKick = (pos % 4 == 0);
        const bool pushKick = complexity > 0.35f && (pos == 3 || pos == 11);
        const bool doubleKick = complexity > 0.72f && (pos == 14);
        const bool clap = (pos == 4 || pos == 12);
        const bool ghostClap = complexity > 0.58f && (pos == 7 || pos == 15);
        const bool offHat = density > 0.18f && (pos == 2 || pos == 6 || pos == 10 || pos == 14);
        const bool sixteenthHat = density > 0.62f && (pos % 2 == 1);
        const bool openHat = density > 0.48f && (pos == 6 || pos == 14);

        if (mainKick || pushKick || doubleKick) {
            const int velocity = mainKick ? 104 : 78;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, velocity, sampleIdx, blockSamples);
        }
        if (clap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 90, sampleIdx, blockSamples);
        }
        if (ghostClap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 44, sampleIdx, blockSamples);
        }
        if (offHat || sixteenthHat) {
            const int velocity = (pos % 4 == 2) ? 74 : 48;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, velocity, sampleIdx, blockSamples);
        }
        if (openHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 72, sampleIdx, blockSamples);
        }
    }
}

void FourOnFloor::reset() {
}
