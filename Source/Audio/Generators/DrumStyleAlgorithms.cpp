#include "DrumStyleAlgorithms.h"

#include "GeneratorUtils.h"

#include <cmath>
#include <initializer_list>
#include <random>


namespace {

inline int stepToSample(int step, double stepBeats, double blockStartBeats, double blockLengthBeats, int blockSamples) {
    const double hitBeat = static_cast<double>(step) * stepBeats;
    const double fraction = (hitBeat - blockStartBeats) / blockLengthBeats;
    return juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
}

inline bool anyOf(int pos, std::initializer_list<int> values) {
    for (const int value : values) {
        if (pos == value) {
            return true;
        }
    }
    return false;
}

inline int accentVelocity(int pos, int strong, int medium, int weak) {
    if (pos % 8 == 0) {
        return strong;
    }
    if (pos % 4 == 0) {
        return medium;
    }
    return weak;
}

inline bool chanceForStep(int step, float probability, uint32_t salt) {
    const uint32_t seed = (static_cast<uint32_t>(step + 1024) * 2654435761u) ^ salt;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng) < juce::jlimit(0.0f, 1.0f, probability);
}

} // namespace

void DnBBreaks::processMidi(juce::MidiBuffer& buffer,
                            double playheadBeats,
                            double blockLengthBeats,
                            int blockSamples,
                            const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);

        const bool kick = (pos == 0
                           || (density > 0.35f && pos == 7)
                           || (complexity > 0.35f && pos == 10)
                           || (complexity > 0.7f && anyOf(pos, {2, 15})));
        const bool snare = (pos == 4 || pos == 12);
        const bool ghostSnare = complexity > 0.45f && anyOf(pos, {3, 6, 11, 14});
        const bool hat = (density > 0.18f && anyOf(pos, {2, 6, 10, 14}))
                         || (density > 0.48f && pos % 2 == 1)
                         || (density > 0.76f && anyOf(pos, {1, 9, 13}));
        const bool openHat = density > 0.68f && anyOf(pos, {7, 15});

        if (kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, accentVelocity(pos, 110, 98, 84), sampleIdx, blockSamples);
        }
        if (snare) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 98, sampleIdx, blockSamples);
        }
        if (ghostSnare && chanceForStep(step, 0.25f + complexity * 0.45f, 0xD0B00u)) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 42 + static_cast<int>(complexity * 20.0f), sampleIdx, blockSamples);
        }
        if (hat) {
            const int vel = accentVelocity(pos, 76, 64, 48);
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, vel, sampleIdx, blockSamples);
        }
        if (openHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 70, sampleIdx, blockSamples);
        }
    }
}

void AfroClaveGroove::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);

        const bool clave = anyOf(pos, {0, 3, 6, 10, 12});
        const bool responseClave = complexity > 0.48f && anyOf(pos, {5, 15});
        const bool kick = (pos == 0 || pos == 8);
        const bool supportKick = complexity > 0.35f && anyOf(pos, {6, 14});
        const bool shaker = (density > 0.22f && pos % 2 == 1)
                            || (density > 0.7f && anyOf(pos, {2, 6, 10, 14}));
        const bool openShaker = density > 0.58f && anyOf(pos, {7, 15});

        if (kick || supportKick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, kick ? 98 : 76, sampleIdx, blockSamples);
        }
        if (clave || responseClave) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, clave ? 78 : 60, sampleIdx, blockSamples);
        }
        if (shaker) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, accentVelocity(pos, 58, 52, 44), sampleIdx, blockSamples);
        }
        if (openShaker) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 58, sampleIdx, blockSamples);
        }
    }
}

void HouseShuffleGroove::processMidi(juce::MidiBuffer& buffer,
                                     double playheadBeats,
                                     double blockLengthBeats,
                                     int blockSamples,
                                     const Scale&,
                                     int,
                                     float density,
                                     float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;

    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = anyOf(pos, {0, 4, 8, 12})
                          || (complexity > 0.48f && anyOf(pos, {3, 11}))
                          || (complexity > 0.75f && pos == 14);
        const bool clap = anyOf(pos, {4, 12});
        const bool offHat = density > 0.2f && anyOf(pos, {2, 6, 10, 14});
        const bool closedHat = offHat || (density > 0.65f && pos % 2 == 1);
        const bool openHat = density > 0.42f && anyOf(pos, {6, 14});
        const bool ghostClap = complexity > 0.42f && anyOf(pos, {7, 15});

        if (kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, anyOf(pos, {0, 8}) ? 108 : 88, sampleIdx, blockSamples);
        }
        if (clap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 92, sampleIdx, blockSamples);
        }
        if (ghostClap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 46, sampleIdx, blockSamples);
        }
        if (closedHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, offHat ? 70 : 46, sampleIdx, blockSamples);
        }
        if (openHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 76, sampleIdx, blockSamples);
        }
    }
}

void TrapHalfTimeGroove::processMidi(juce::MidiBuffer& buffer,
                                     double playheadBeats,
                                     double blockLengthBeats,
                                     int blockSamples,
                                     const Scale&,
                                     int,
                                     float density,
                                     float complexity) {
    int sampleIdx = 0;
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx)) {
        GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 108, sampleIdx, blockSamples);
    }
    if (complexity > 0.28f && GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx, 0.75)) {
        GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 82, sampleIdx, blockSamples);
    }
    if (density > 0.55f && GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx, 1.5)) {
        GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 74, sampleIdx, blockSamples);
    }
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx, 1.0)) {
        GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 112, sampleIdx, blockSamples);
    }

    const double start32 = playheadBeats * 8.0;
    const double end32 = (playheadBeats + blockLengthBeats) * 8.0;

    for (int step = static_cast<int>(std::ceil(start32)); step < end32; ++step) {
        int pos = step % 32;
        if (pos < 0) {
            pos += 32;
        }

        bool hat = false;
        if (density > 0.12f && pos % 4 == 2) {
            hat = true;
        }
        if (complexity > 0.32f && (pos == 11 || pos == 15 || pos == 23 || pos == 27)) {
            hat = true;
        }
        if (density > 0.7f && (pos == 6 || pos == 14 || pos == 30)) {
            hat = true;
        }
        if (complexity > 0.62f && anyOf(pos, {5, 13, 21, 29})) {
            hat = true;
        }

        if (hat) {
            const int idx = stepToSample(step, 0.125, playheadBeats, blockLengthBeats, blockSamples);
            const int vel = (pos % 8 == 0) ? 84 : 58;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, vel, idx, blockSamples);
        }
    }
}

void GlitchPulseGroove::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale&,
                                    int,
                                    float density,
                                    float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;

    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        std::mt19937 rng(static_cast<uint32_t>((step + 1) * 2654435761u));
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool anchorKick = (step % 16 == 0);
        const bool gridKick = complexity > 0.36f && anyOf(step % 16, {6, 11});
        const bool pulse = dist(rng) < (0.08f + density * 0.48f);
        const bool accent = complexity > 0.38f && dist(rng) < (0.08f + complexity * 0.26f);

        if (anchorKick || gridKick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 108, sampleIdx, blockSamples);
        } else if (pulse) {
            const int note = accent ? MAPPING_SNARE : MAPPING_CLOSED_HAT;
            const int vel = accent ? 88 : 52;
            GeneratorUtils::addNoteEvent(buffer, 10, note, vel, sampleIdx, blockSamples);
        }
    }
}

void TechnoRumbleGroove::processMidi(juce::MidiBuffer& buffer,
                                     double playheadBeats,
                                     double blockLengthBeats,
                                     int blockSamples,
                                     const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);

        const bool mainKick = (pos == 0 || pos == 4 || pos == 8 || pos == 12);
        const bool rumbleKick = complexity > 0.28f && anyOf(pos, {3, 7, 11, 15});
        const bool extraRumble = complexity > 0.58f && anyOf(pos, {2, 6, 10, 14});
        const bool clap = (pos == 4 || pos == 12);
        const bool hat = (density > 0.2f && anyOf(pos, {2, 6, 10, 14}))
                         || (density > 0.55f && pos % 2 == 1);
        const bool openHat = density > 0.48f && (pos == 7 || pos == 15);

        if (mainKick || rumbleKick || extraRumble) {
            const int vel = mainKick ? 112 : (extraRumble ? 64 : 76);
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, vel, sampleIdx, blockSamples);
        }
        if (clap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 94, sampleIdx, blockSamples);
        }
        if (hat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 62, sampleIdx, blockSamples);
        }
        if (openHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 74, sampleIdx, blockSamples);
        }
    }
}

void JerseyClubGroove::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = (pos == 0 || pos == 3 || pos == 6 || pos == 8 || pos == 11 || pos == 14)
                          || (density > 0.62f && anyOf(pos, {1, 9}))
                          || (complexity > 0.48f && anyOf(pos, {5, 13}));
        const bool clap = (pos == 4 || pos == 12 || (complexity > 0.45f && pos == 10));
        const bool hat = density > 0.25f && (pos == 2 || pos == 7 || pos == 10 || pos == 15);
        const bool tripletBurst = complexity > 0.45f && (pos == 1 || pos == 5 || pos == 9 || pos == 13);
        const bool secondLineKick = complexity > 0.58f && (pos == 15);

        if (kick || secondLineKick) {
            const int vel = (pos == 0 || pos == 8) ? 108 : 84;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, vel, sampleIdx, blockSamples);
        }
        if (clap) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 96, sampleIdx, blockSamples);
        }
        if (hat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 56, sampleIdx, blockSamples);
        }
        if (tripletBurst) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 70, sampleIdx, blockSamples);
        }
    }
}

void BrokenStepperGroove::processMidi(juce::MidiBuffer& buffer,
                                      double playheadBeats,
                                      double blockLengthBeats,
                                      int blockSamples,
                                      const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = (pos == 0 || pos == 5 || pos == 9 || (complexity > 0.45f && pos == 13));
        const bool snare = (pos == 4 || pos == 12);
        const bool ghostSnare = complexity > 0.35f && anyOf(pos, {3, 6, 11, 14});
        const bool hat = (density > 0.16f && anyOf(pos, {2, 6, 10, 14}))
                         || (density > 0.52f && (pos % 2 == 0 || pos == 15));
        const bool openHat = density > 0.7f && anyOf(pos, {7, 15});

        if (kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 102, sampleIdx, blockSamples);
        }
        if (snare) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 98, sampleIdx, blockSamples);
        }
        if (ghostSnare) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 58, sampleIdx, blockSamples);
        }
        if (hat) {
            const int vel = (pos % 4 == 0) ? 76 : 52;
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, vel, sampleIdx, blockSamples);
        }
        if (openHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 64, sampleIdx, blockSamples);
        }
    }
}

void PolyrhythmTomGroove::processMidi(juce::MidiBuffer& buffer,
                                      double playheadBeats,
                                      double blockLengthBeats,
                                      int blockSamples,
                                      const Scale&,
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

        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool pulse3 = (step % 3 == 0);
        const bool pulse5 = (step % 5 == 0);
        const bool kick = (pos == 0 || pos == 8 || (complexity > 0.55f && pos == 10));
        const bool tomA = density > 0.18f && pulse3;
        const bool tomB = density > 0.38f && pulse5;
        const bool tomC = complexity > 0.42f && anyOf(pos, {7, 13, 15});
        const bool supportHat = density > 0.62f && (pos % 2 == 1);

        if (kick) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 104, sampleIdx, blockSamples);
        }
        if (tomA) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 78, sampleIdx, blockSamples);
        }
        if (tomB || tomC) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 68, sampleIdx, blockSamples);
        }
        if (supportHat) {
            GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 50, sampleIdx, blockSamples);
        }
    }
}

void ElectroBreaksGroove::processMidi(juce::MidiBuffer& buffer,
                                      double playheadBeats,
                                      double blockLengthBeats,
                                      int blockSamples,
                                      const Scale&,
                                      int,
                                      float density,
                                      float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = pos == 0 || pos == 6 || pos == 10
                          || (complexity > 0.35f && pos == 14)
                          || (complexity > 0.58f && pos == 3);
        const bool snare = pos == 4 || pos == 12;
        const bool hat = (density > 0.16f && (pos == 2 || pos == 8 || pos == 14))
                         || (density > 0.5f && pos % 2 == 1);
        const bool openHat = density > 0.66f && anyOf(pos, {7, 15});
        if (kick) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 106, sampleIdx, blockSamples);
        if (snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 96, sampleIdx, blockSamples);
        if (hat) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 58, sampleIdx, blockSamples);
        if (openHat) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 68, sampleIdx, blockSamples);
    }
}

void GarageSwingGroove::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale&,
                                    int,
                                    float density,
                                    float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = pos == 0
                          || (density > 0.45f && anyOf(pos, {7, 10}))
                          || (complexity > 0.42f && anyOf(pos, {3, 13}));
        const bool snare = pos == 4 || pos == 12;
        const bool rim = complexity > 0.28f && anyOf(pos, {6, 11, 15});
        const bool hat = (density > 0.2f && anyOf(pos, {2, 5, 10, 13}))
                         || (density > 0.68f && pos % 2 == 1);
        const bool openHat = density > 0.5f && anyOf(pos, {6, 14});
        if (kick) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, anyOf(pos, {0, 10}) ? 102 : 82, sampleIdx, blockSamples);
        if (snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 104, sampleIdx, blockSamples);
        if (rim) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 54, sampleIdx, blockSamples);
        if (hat) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, accentVelocity(pos, 68, 58, 46), sampleIdx, blockSamples);
        if (openHat) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 70, sampleIdx, blockSamples);
    }
}

void LatinPercGroove::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale&,
                                  int,
                                  float density,
                                  float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = pos == 0 || pos == 8 || (complexity > 0.36f && anyOf(pos, {6, 14}));
        const bool rim = pos == 0 || pos == 3 || pos == 6 || pos == 10 || pos == 13;
        const bool shaker = (density > 0.18f && pos % 2 == 1)
                            || (density > 0.68f && anyOf(pos, {2, 6, 10, 14}));
        const bool open = density > 0.52f && anyOf(pos, {7, 15});
        if (kick) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 94, sampleIdx, blockSamples);
        if (rim) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, complexity > 0.6f ? 82 : 70, sampleIdx, blockSamples);
        if (shaker) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 46, sampleIdx, blockSamples);
        if (open) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 58, sampleIdx, blockSamples);
    }
}

void MinimalClicksGroove::processMidi(juce::MidiBuffer& buffer,
                                      double playheadBeats,
                                      double blockLengthBeats,
                                      int blockSamples,
                                      const Scale&,
                                      int,
                                      float density,
                                      float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        if (pos == 0 || (complexity > 0.48f && pos == 11)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, pos == 0 ? 86 : 56, sampleIdx, blockSamples);
        if (pos == 10 || (density > 0.35f && (pos == 3 || pos == 14)) || (density > 0.7f && (pos == 6 || pos == 12))) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 44, sampleIdx, blockSamples);
        if (complexity > 0.28f && (pos == 6 || (complexity > 0.65f && pos == 15))) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 48, sampleIdx, blockSamples);
    }
}

void DubSkankGroove::processMidi(juce::MidiBuffer& buffer,
                                 double playheadBeats,
                                 double blockLengthBeats,
                                 int blockSamples,
                                 const Scale&,
                                 int,
                                 float density,
                                 float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        const int sampleIdx = stepToSample(step, 0.5, playheadBeats, blockLengthBeats, blockSamples);
        if (pos == 0 || pos == 4 || (complexity > 0.34f && pos == 7)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, pos == 7 ? 70 : 96, sampleIdx, blockSamples);
        if (pos == 2 || pos == 6) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 90, sampleIdx, blockSamples);
        if (density > 0.25f && (pos == 1 || pos == 5 || (complexity > 0.45f && pos == 7) || (density > 0.72f && pos == 3))) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 50, sampleIdx, blockSamples);
        if (density > 0.58f && (pos == 3 || pos == 7)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 62, sampleIdx, blockSamples);
    }
}

void Footwork160Groove::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale&,
                                    int,
                                    float density,
                                    float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        const bool kick = pos == 0 || pos == 2 || pos == 7 || pos == 11
                          || (complexity > 0.34f && pos == 15)
                          || (complexity > 0.62f && pos == 5);
        const bool snare = pos == 4 || pos == 12 || (complexity > 0.48f && pos == 9);
        const bool hat = (density > 0.18f && anyOf(pos, {1, 3, 6, 10, 14}))
                         || (density > 0.62f && pos % 2 == 1);
        if (kick) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, 104, sampleIdx, blockSamples);
        if (snare) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 98, sampleIdx, blockSamples);
        if (hat) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 54, sampleIdx, blockSamples);
    }
}

void HalfstepGroove::processMidi(juce::MidiBuffer& buffer,
                                 double playheadBeats,
                                 double blockLengthBeats,
                                 int blockSamples,
                                 const Scale&,
                                 int,
                                 float density,
                                 float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        if (pos == 0 || (complexity > 0.3f && pos == 7) || (complexity > 0.62f && pos == 10)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, pos == 0 ? 110 : 82, sampleIdx, blockSamples);
        if (pos == 12) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 108, sampleIdx, blockSamples);
        if (complexity > 0.42f && pos == 11) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 54, sampleIdx, blockSamples);
        if (density > 0.18f && (pos == 2 || pos == 6 || pos == 10 || pos == 14 || (density > 0.7f && pos % 2 == 1))) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 52, sampleIdx, blockSamples);
        if (density > 0.58f && (pos == 6 || pos == 14)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 64, sampleIdx, blockSamples);
    }
}

void IndustrialGroove::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale&,
                                   int,
                                   float density,
                                   float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const int sampleIdx = stepToSample(step, 0.25, playheadBeats, blockLengthBeats, blockSamples);
        if (pos % 4 == 0 || (complexity > 0.62f && anyOf(pos, {7, 15}))) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_KICK, pos % 4 == 0 ? 112 : 72, sampleIdx, blockSamples);
        if (pos == 3 || pos == 11 || (complexity > 0.48f && pos == 14) || (complexity > 0.8f && pos == 6)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_SNARE, 88, sampleIdx, blockSamples);
        if (density > 0.24f && (pos % 2 == 1)) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_OPEN_HAT, 58, sampleIdx, blockSamples);
        if (density > 0.68f && anyOf(pos, {2, 6, 10, 14})) GeneratorUtils::addNoteEvent(buffer, 10, MAPPING_CLOSED_HAT, 46, sampleIdx, blockSamples);
    }
}
