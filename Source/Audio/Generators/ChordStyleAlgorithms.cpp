#include "ChordStyleAlgorithms.h"

#include "GeneratorUtils.h"

#include <array>
#include <cmath>
#include <random>
#include <vector>


namespace {

inline int toSampleIndex(double hitBeat, double blockStartBeats, double blockLengthBeats, int blockSamples) {
    const double fraction = (hitBeat - blockStartBeats) / blockLengthBeats;
    return juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
}

inline void addChord(GenerativeAlgorithm& algorithm,
                     juce::MidiBuffer& buffer,
                     int channel,
                     const std::vector<int>& notes,
                     int velocity,
                     int sampleOn,
                     double durationBeats,
                     double playheadBeats,
                     double blockLengthBeats,
                     int blockSamples) {
    for (int note : notes) {
        algorithm.addScheduledNoteAtSample(buffer, channel, note, velocity, sampleOn, durationBeats,
                                           playheadBeats, blockLengthBeats, blockSamples);
    }
}

} // namespace

void HousePianoStabs::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale& scale,
                                  int,
                                  float density,
                                  float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;

    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        const bool anchor = pos == 1;
        const bool offbeat = (pos == 3 || pos == 5 || pos == 7);
        if (!anchor && (!offbeat || density < 0.2f)) {
            continue;
        }

        auto tones = scale.getChordTones(0, 4, complexity > 0.35f);
        if (complexity > 0.72f) {
            tones.push_back(scale.getDegree(8, 4));
        }

        const double hitBeat = step * 0.5;
        const int on = toSampleIndex(hitBeat, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 86, on, 0.35, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void AmbientPadSwells::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    int sampleIdx = 0;
    if (!GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 4.0, blockSamples, sampleIdx)) {
        return;
    }

    auto tones = scale.getChordTones(0, 3, true);
    if (complexity > 0.4f) {
        tones.push_back(scale.getDegree(8, 4));
    }

    addChord(*this, buffer, 12, tones, 72, sampleIdx, 3.75, playheadBeats, blockLengthBeats, blockSamples);
}

void NeoSoulVoicings::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale& scale,
                                  int,
                                  float density,
                                  float complexity) {
    const double grid = density > 0.5f ? 1.0 : 2.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        std::vector<int> tones;
        tones.push_back(scale.getDegree(0, 3));
        tones.push_back(scale.getDegree(3, 3));
        tones.push_back(scale.getDegree(6, 3));
        tones.push_back(scale.getDegree(9, 4));
        if (complexity > 0.55f) {
            tones.push_back(scale.getDegree(13, 4));
        }

        const int on = toSampleIndex(next, start, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 80, on, grid * 0.85, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

void TranceGateChords::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    const double grid = density > 0.6f ? 0.25 : 0.5;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        auto tones = scale.getChordTones(0, 4, complexity > 0.4f);
        const int on = toSampleIndex(next, start, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 88, on, complexity > 0.65f ? 0.22 : 0.15, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

void QuartalComping::processMidi(juce::MidiBuffer& buffer,
                                 double playheadBeats,
                                 double blockLengthBeats,
                                 int blockSamples,
                                 const Scale& scale,
                                 int,
                                 float density,
                                 float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;

    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        const bool anchor = pos == 0;
        const bool hit = anchor || (density > 0.25f && (pos == 3 || pos == 6));
        if (!hit) {
            continue;
        }

        std::vector<int> tones;
        tones.push_back(scale.getDegree(0, 3));
        tones.push_back(scale.getDegree(3, 3));
        tones.push_back(scale.getDegree(6, 4));
        if (complexity > 0.6f) {
            tones.push_back(scale.getDegree(10, 4));
        }

        const int on = toSampleIndex(step * 0.5, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 82, on, 0.75, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void VoiceCloudChords::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    const double grid = density > 0.5f ? 0.5 : 1.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        std::mt19937 rng(static_cast<uint32_t>(next * 1000.0));
        std::uniform_int_distribution<int> jump(-2, 3);

        std::vector<int> tones;
        tones.push_back(scale.getDegree(jump(rng), 3));
        tones.push_back(scale.getDegree(jump(rng) + 2, 4));
        tones.push_back(scale.getDegree(jump(rng) + 5, 4));
        if (complexity > 0.5f) {
            tones.push_back(scale.getDegree(jump(rng) + 8, 4));
        }

        const int on = toSampleIndex(next, start, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 76, on, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

void GospelLiftChords::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    const double grid = density > 0.55f ? 1.0 : 2.0;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        std::vector<int> tones = scale.getChordTones(0, 3, true);
        tones.push_back(scale.getDegree(9, 4));
        if (complexity > 0.65f) {
            tones.push_back(scale.getDegree(13, 4));
        }

        const int on = toSampleIndex(next, start, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 88, on, grid * 0.9, playheadBeats, blockLengthBeats, blockSamples);

        if (complexity > 0.5f) {
            const int liftOn = juce::jlimit(0, blockSamples - 1, on + 160);
            const int liftNote = scale.getDegree(11, 4);
            addScheduledNoteAtSample(buffer, 12, liftNote, 74, liftOn, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        }

        next += grid;
    }
}

void DetuneStackChords::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale& scale,
                                    int,
                                    float density,
                                    float complexity) {
    const double grid = density > 0.55f ? 0.5 : 1.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        std::vector<int> tones;
        tones.push_back(scale.getDegree(0, 3));
        tones.push_back(scale.getDegree(7, 3));
        tones.push_back(scale.getDegree(12, 4));
        if (complexity > 0.45f) {
            tones.push_back(scale.getDegree(14, 4));
        }

        const int on = toSampleIndex(next, start, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 84, on, grid * 0.8, playheadBeats, blockLengthBeats, blockSamples);

        if (complexity > 0.7f) {
            const int shimmerOn = juce::jlimit(0, blockSamples - 1, on + 24);
            addScheduledNoteAtSample(buffer, 12, scale.getDegree(19, 5), 68, shimmerOn, 0.35, playheadBeats, blockLengthBeats, blockSamples);
        }

        next += grid;
    }
}

void BrokenStrumChords::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale& scale,
                                    int,
                                    float density,
                                    float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;

    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        const bool hit = (pos == 0)
            || (density > 0.2f && (pos == 3 || pos == 5))
            || (density > 0.6f && (pos == 7));
        if (!hit) {
            continue;
        }

        std::array<int, 4> tones = {
            scale.getDegree(0, 3),
            scale.getDegree(3, 3),
            scale.getDegree(7, 4),
            scale.getDegree(10, 4)
        };

        const int on = toSampleIndex(step * 0.5, playheadBeats, blockLengthBeats, blockSamples);
        const int strumStepSamples = complexity > 0.6f ? 26 : 42;
        const double holdBeats = complexity > 0.5f ? 0.42 : 0.56;
        for (size_t i = 0; i < tones.size(); ++i) {
            const int index = static_cast<int>((i + static_cast<size_t>(strumOffset_)) % tones.size());
            const int noteOnSample = juce::jlimit(0, blockSamples - 1, on + static_cast<int>(i) * strumStepSamples);
            const int velocity = 84 - static_cast<int>(i) * 4;
            addScheduledNoteAtSample(buffer, 12, tones[static_cast<size_t>(index)], velocity, noteOnSample,
                                     holdBeats, playheadBeats, blockLengthBeats, blockSamples);
        }
        strumOffset_ = (strumOffset_ + 1) % static_cast<int>(tones.size());
    }
}

void PulseClusterChords::processMidi(juce::MidiBuffer& buffer,
                                     double playheadBeats,
                                     double blockLengthBeats,
                                     int blockSamples,
                                     const Scale& scale,
                                     int,
                                     float density,
                                     float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;

    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        int pos = step % 16;
        if (pos < 0) {
            pos += 16;
        }

        const bool hit = (pos == 0 || pos == 8)
            || (density > 0.15f && pos % 4 == 0)
            || (density > 0.5f && (pos == 6 || pos == 10 || pos == 14));
        if (!hit) {
            continue;
        }

        const int on = toSampleIndex(step * 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const double durationBeats = complexity > 0.6f ? 0.18 : 0.26;

        std::vector<int> tones;
        tones.push_back(scale.getDegree(0, 4));
        tones.push_back(scale.getDegree(1, 4));
        tones.push_back(scale.getDegree(4, 4));
        if (complexity > 0.55f) {
            tones.push_back(scale.getDegree(8, 4));
        }

        addChord(*this, buffer, 12, tones, 78, on, durationBeats, playheadBeats, blockLengthBeats, blockSamples);
    }
}

namespace {

std::vector<int> triadWithColor(const Scale& scale, int octave, int colorDegree, bool addColor) {
    auto tones = scale.getChordTones(0, octave, true);
    if (addColor) {
        tones.push_back(scale.getDegree(colorDegree, octave + 1));
    }
    return tones;
}

} // namespace

void DubSkankChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (!(pos == 2 || pos == 6 || (density > 0.6f && pos == 7))) continue;
        const int on = toSampleIndex(step * 0.5, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, triadWithColor(scale, 3, 9, complexity > 0.55f), 78, on, 0.25, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void MinimalPluckChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.55f ? 1.0 : 2.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        std::vector<int> tones{scale.getDegree(0, 4), scale.getDegree(4, 4)};
        if (complexity > 0.5f) tones.push_back(scale.getDegree(7, 4));
        const int on = toSampleIndex(next, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 72, on, 0.2, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void RNBKeyChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (!(pos == 0 || pos == 3 || (density > 0.45f && pos == 6))) continue;
        std::vector<int> tones{scale.getDegree(0, 3), scale.getDegree(3, 3), scale.getDegree(6, 4), scale.getDegree(10, 4)};
        if (complexity > 0.6f) tones.push_back(scale.getDegree(13, 4));
        const int on = toSampleIndex(step * 0.5, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 82, on, 0.75, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void SuspendedPadChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    int sampleIdx = 0;
    if (!GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, density > 0.55f ? 2.0 : 4.0, blockSamples, sampleIdx)) return;
    std::vector<int> tones{scale.getDegree(0, 3), scale.getDegree(3, 4), scale.getDegree(7, 4), scale.getDegree(9, 4)};
    if (complexity > 0.65f) tones.push_back(scale.getDegree(14, 5));
    addChord(*this, buffer, 12, tones, 76, sampleIdx, density > 0.55f ? 1.85 : 3.75, playheadBeats, blockLengthBeats, blockSamples);
}

void CinematicHitChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.5f ? 2.0 : 4.0;
    int sampleIdx = 0;
    if (!GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, grid, blockSamples, sampleIdx)) return;
    std::vector<int> tones{scale.getDegree(0, 2), scale.getDegree(7, 3), scale.getDegree(12, 4)};
    if (complexity > 0.45f) tones.push_back(scale.getDegree(17, 4));
    addChord(*this, buffer, 12, tones, 96, sampleIdx, 1.25, playheadBeats, blockLengthBeats, blockSamples);
}

void FifthDroneChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.7f ? 1.0 : 2.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        std::vector<int> tones{scale.getDegree(0, 3), scale.getDegree(4, 3), scale.getDegree(7, 4)};
        if (complexity > 0.55f) tones.push_back(scale.getDegree(11, 4));
        const int on = toSampleIndex(next, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 70, on, grid * 0.95, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void GarageOrganChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        if (!(pos == 2 || pos == 7 || pos == 10 || (density > 0.55f && pos == 14))) continue;
        const int on = toSampleIndex(step * 0.25, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, triadWithColor(scale, 3, 8, complexity > 0.5f), 84, on, 0.28, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void PolychordChords::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.55f ? 1.0 : 2.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        std::vector<int> tones{scale.getDegree(0, 3), scale.getDegree(2, 3), scale.getDegree(4, 4), scale.getDegree(7, 4)};
        if (complexity > 0.5f) {
            tones.push_back(scale.getDegree(9, 4));
            tones.push_back(scale.getDegree(13, 5));
        }
        const int on = toSampleIndex(next, playheadBeats, blockLengthBeats, blockSamples);
        addChord(*this, buffer, 12, tones, 78, on, grid * 0.85, playheadBeats, blockLengthBeats, blockSamples);
    }
}
