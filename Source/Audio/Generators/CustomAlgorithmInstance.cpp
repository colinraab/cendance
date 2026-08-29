#include "CustomAlgorithmInstance.h"
#include "../../App/AlgorithmCatalog.h"

#include <algorithm>
#include <cmath>

CustomAlgorithmInstance::CustomAlgorithmInstance(const CustomAlgorithmPreset& preset)
    : preset_(preset)
    , rng_(42)
{
    stepDurationBeats_ = 4.0 / static_cast<double>(preset_.stepCount);
}

void CustomAlgorithmInstance::setPreset(CustomAlgorithmPreset preset) {
    preset_ = std::move(preset);
    stepDurationBeats_ = 4.0 / static_cast<double>(preset_.stepCount);
    reset();
}

void CustomAlgorithmInstance::reset() {
    clearPendingNoteOffs();
    currentStep_ = 0;
}

juce::String CustomAlgorithmInstance::getName() const {
    return juce::String(preset_.name);
}

uint8_t CustomAlgorithmInstance::getDefaultDrumNote(uint8_t step) {
    // Map steps to standard drum notes
    static constexpr uint8_t kDrumMap[] = {
        36, // kick
        38, // snare
        42, // closed hat
        46, // open hat
        39, // clap
        41, // low tom
        43, // mid tom
        45, // high tom
        49, // crash
        51, // ride
        37, // rim
        40, // snare 2
        44, // pedal hat
        47, // mid tom 2
        48, // high tom 2
        50, // high tom 3
    };
    return kDrumMap[step % 16];
}

int CustomAlgorithmInstance::mapTrackToMidiNote(uint8_t step, int rootNote, const Scale& scale) const {
    if (step >= preset_.melodicPattern.size())
        return rootNote;

    int interval = preset_.melodicPattern[step];
    int octaveShift = 0;

    if (preset_.octaveRange.first != preset_.octaveRange.second) {
        std::uniform_int_distribution<int> octDist(preset_.octaveRange.first, preset_.octaveRange.second);
        octaveShift = octDist(rng_) * 12;
    } else if (preset_.octaveRange.first != 0) {
        octaveShift = preset_.octaveRange.first * 12;
    }

    int note = rootNote + interval + octaveShift;

    // Quantize to scale
    if (preset_.scaleMask != 0) {
        note = scale.quantize(note);
    }

    return note;
}

void CustomAlgorithmInstance::processMidi(juce::MidiBuffer& buffer,
                                          double playheadBeats,
                                          double blockLengthBeats,
                                          int blockSamples,
                                          const Scale& scale,
                                          int rootNote,
                                          float density,
                                          float complexity) {
    if (preset_.stepCount == 0)
        return;

    stepDurationBeats_ = 4.0 / static_cast<double>(preset_.stepCount);

    // Determine which steps fall within this block
    double blockStartBeat = playheadBeats;
    double blockEndBeat = playheadBeats + blockLengthBeats;

    // Bar start alignment
    double barStartBeat = std::floor(blockStartBeat / 4.0) * 4.0;

    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);

    for (uint8_t step = 0; step < preset_.stepCount; ++step) {
        double stepStartBeat = barStartBeat + static_cast<double>(step) * stepDurationBeats_;
        double stepEndBeat = stepStartBeat + stepDurationBeats_ * preset_.noteLength;

        // Check if this step overlaps with the current block
        if (stepEndBeat <= blockStartBeat || stepStartBeat >= blockEndBeat)
            continue;

        // Check rhythmic gate
        uint8_t gate = 0;
        if (step < preset_.rhythmicPattern.size())
            gate = preset_.rhythmicPattern[step];

        if (gate == 0)
            continue;

        // Apply density
        float stepDensity = density;
        if (step < preset_.densityCurve.size())
            stepDensity *= preset_.densityCurve[step];
        stepDensity = std::clamp(stepDensity, 0.0f, 1.0f);

        if (probDist(rng_) > stepDensity)
            continue;

        // Calculate velocity
        float velRange = static_cast<float>(preset_.velocityRange.second - preset_.velocityRange.first);
        float baseVel = static_cast<float>(preset_.velocityRange.first) + velRange * 0.5f;
        float velVariation = velRange * 0.25f * (probDist(rng_) * 2.0f - 1.0f);
        uint8_t velocity = static_cast<uint8_t>(std::clamp(baseVel + velVariation, 1.0f, 127.0f));

        // Swing: delay even-numbered steps
        double swingOffset = 0.0;
        if (step % 2 == 1 && preset_.swing > 0.0)
            swingOffset = stepDurationBeats_ * preset_.swing;

        double noteStartBeat = stepStartBeat + swingOffset;

        // Convert beat position to sample offset
        double beatOffset = noteStartBeat - blockStartBeat;
        int sampleOffset = static_cast<int>(beatOffset / blockLengthBeats * static_cast<double>(blockSamples));
        sampleOffset = std::clamp(sampleOffset, 0, blockSamples - 1);

        int noteNumber = 0;
        bool isDrumTrack = (preset_.trackIndex == 0);

        if (isDrumTrack) {
            noteNumber = getDefaultDrumNote(step);
        } else {
            noteNumber = mapTrackToMidiNote(step, rootNote, scale);
        }

        noteNumber = std::clamp(noteNumber, 0, 127);

        double noteDuration = stepDurationBeats_ * preset_.noteLength * 0.9;
        addScheduledNoteAtSample(buffer, 1, noteNumber, velocity, sampleOffset, noteDuration,
                                 playheadBeats, blockLengthBeats, blockSamples);

        // Apply complexity: extra notes / grace notes
        float stepComplexity = complexity;
        if (step < preset_.complexityCurve.size())
            stepComplexity *= preset_.complexityCurve[step];
        stepComplexity = std::clamp(stepComplexity, 0.0f, 1.0f);

        if (stepComplexity > 0.5f && probDist(rng_) < (stepComplexity - 0.5f) * 2.0f) {
            // Add a grace note or extra hit
            int graceOffset = sampleOffset + 1;
            if (graceOffset < blockSamples) {
                int graceNote = isDrumTrack ? getDefaultDrumNote((step + 1) % 16) : noteNumber + 1;
                graceNote = std::clamp(graceNote, 0, 127);
                uint8_t graceVel = static_cast<uint8_t>(velocity * 0.7f);
                addScheduledNoteAtSample(buffer, 1, graceNote, graceVel, graceOffset,
                                         std::min(0.05, stepDurationBeats_ * 0.25),
                                         playheadBeats, blockLengthBeats, blockSamples);
            }
        }
    }
}
