#include "BassStyleAlgorithms.h"

#include "GeneratorUtils.h"

#include <cmath>
#include <random>


void Sub808Bass::processMidi(juce::MidiBuffer& buffer,
                             double playheadBeats,
                             double blockLengthBeats,
                             int blockSamples,
                             const Scale& scale,
                             int,
                             float density,
                             float complexity) {
    int sampleIdx = 0;
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 1.0, blockSamples, sampleIdx)) {
        int note = scale.getDegree(0, 2);
        if (complexity > 0.65f && std::fmod(playheadBeats, 4.0) > 2.5) {
            note = scale.getDegree(4, 2);
        }
        const int vel = static_cast<int>(92 + density * 20.0f);
        addScheduledNoteAtSample(buffer, 11, note, juce::jlimit(70, 120, vel), sampleIdx, 1.5, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void UKGarageBass::processMidi(juce::MidiBuffer& buffer,
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

        const bool anchor = (pos == 0 || pos == 10);
        const bool optional = (pos == 3 || pos == 7 || pos == 14);
        const bool hit = anchor || (density > 0.25f && optional);
        if (!hit) {
            continue;
        }

        const double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        int degree = 0;
        if (pos == 7 && complexity > 0.45f) {
            degree = 4;
        } else if (pos == 14 && complexity > 0.6f) {
            degree = 6;
        }

        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 100, sampleIdx, 0.28, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void TumbaoBass::processMidi(juce::MidiBuffer& buffer,
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
        int pos = step % 8;
        if (pos < 0) {
            pos += 8;
        }

        const bool anchor = (pos == 1 || pos == 6);
        const bool optional = pos == 3;
        const bool hit = anchor || (density > 0.2f && optional);
        if (!hit) {
            continue;
        }

        const double fraction = (step * 0.5 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        int degree = 0;
        if (complexity > 0.5f && pos == 3) {
            degree = 2;
        } else if (complexity > 0.7f && pos == 6) {
            degree = 4;
        }
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 92, sampleIdx, 0.45, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void DubPedalBass::processMidi(juce::MidiBuffer& buffer,
                               double playheadBeats,
                               double blockLengthBeats,
                               int blockSamples,
                               const Scale& scale,
                               int,
                               float density,
                               float complexity) {
    int sampleIdx = 0;
    if (GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx)) {
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(0, 2), 108, sampleIdx, 1.5, playheadBeats, blockLengthBeats, blockSamples);
    }

    if (density > 0.55f && GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx, 1.5)) {
        const int degree = (complexity > 0.6f) ? 4 : 2;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 84, sampleIdx, 0.55, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void ReesePulseBass::processMidi(juce::MidiBuffer& buffer,
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
        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        int degree = 0;
        if (complexity > 0.45f && std::fmod(next, 1.0) > 0.49) {
            degree = 7;
        }
        if (complexity > 0.75f && std::fmod(next, 2.0) > 1.5) {
            degree = 10;
        }

        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 96, sampleIdx, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

void MotifBass::processMidi(juce::MidiBuffer& buffer,
                            double playheadBeats,
                            double blockLengthBeats,
                            int blockSamples,
                            const Scale& scale,
                            int,
                            float density,
                            float complexity) {
    static constexpr int motif[] = {0, 0, 3, 0, 5, 3, 0, 7};
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;

    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (density < 0.15f && pos != 0) {
            continue;
        }
        const double fraction = (step * 0.5 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        const int idx = motifIndex_ % static_cast<int>(std::size(motif));
        int degree = motif[idx];
        if (complexity > 0.65f && idx % 3 == 2) {
            degree += 2;
        }
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 94, sampleIdx, 0.38, playheadBeats, blockLengthBeats, blockSamples);
        ++motifIndex_;
    }
}

void AcidTripletBass::processMidi(juce::MidiBuffer& buffer,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples,
                                  const Scale& scale,
                                  int,
                                  float density,
                                  float complexity) {
    const double grid = 1.0 / 3.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        const int tripletStep = static_cast<int>(std::floor(next * 3.0));
        const double phase = std::fmod(next, 1.0);
        if (density < 0.2f && tripletStep % 6 != 0) {
            next += grid;
            continue;
        }
        const bool rest = density < 0.45f && (tripletStep % 6 == 5);
        if (rest) {
            next += grid;
            continue;
        }

        int degree = 0;
        if (complexity > 0.4f && phase > 0.2 && phase < 0.5) {
            degree = 2;
        } else if (complexity > 0.7f && phase >= 0.66) {
            degree = 7;
        }
        if (complexity > 0.75f && tripletStep % 12 >= 9) {
            degree += 12;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        const int velocity = (tripletStep % 3 == 0) ? 108 : 84;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), velocity, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);

        next += grid;
    }
}

void GlideCounterBass::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    static constexpr int contour[] = {0, 2, 5, 7, 10, 7, 5, 2};
    const double grid = density > 0.6f ? 0.5 : 1.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        if (density < 0.15f && step_ % 4 != 0) {
            ++step_;
            next += grid;
            continue;
        }

        int degree = contour[step_ % static_cast<int>(std::size(contour))];
        if (complexity > 0.6f && (step_ % 2 == 1)) {
            degree += 2;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 96, sampleIdx, grid * 0.8, playheadBeats, blockLengthBeats, blockSamples);

        ++step_;
        next += grid;
    }
}

void PulseChopBass::processMidi(juce::MidiBuffer& buffer,
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

        const bool anchor = (pos == 0 || pos == 8);
        const bool regular = density > 0.2f && (pos % 2 == 0);
        const bool syncopated = density > 0.2f && complexity > 0.55f && (pos == 3 || pos == 11);
        const bool hit = anchor || regular || syncopated;
        if (!hit) {
            continue;
        }

        int degree = 0;
        if (complexity > 0.45f && (pos == 6 || pos == 14)) {
            degree = 5;
        } else if (complexity > 0.75f && (pos == 3 || pos == 11)) {
            degree = 9;
        }

        const double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        const int velocity = (pos % 4 == 0) ? 102 : 78;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), velocity, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void OctaveBounceBass::processMidi(juce::MidiBuffer& buffer,
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
        int pos = step % 8;
        if (pos < 0) {
            pos += 8;
        }

        const bool hit = (pos == 0)
            || (density <= 0.2f && pos == 4)
            || (density > 0.2f && (pos == 2 || pos == 4 || pos == 6))
            || (density > 0.65f && (pos == 1 || pos == 5));
        if (!hit) {
            continue;
        }

        int octave = octaveFlip_ ? 3 : 2;
        int degree = (complexity > 0.65f && (pos == 2 || pos == 6)) ? 7 : 0;
        const double fraction = (step * 0.5 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, octave), 94, sampleIdx, 0.38, playheadBeats, blockLengthBeats, blockSamples);
        octaveFlip_ = !octaveFlip_;
    }
}

namespace {

int bassSampleForStep(int step, double stepBeats, double playheadBeats, double blockLengthBeats, int blockSamples) {
    const double fraction = (static_cast<double>(step) * stepBeats - playheadBeats) / blockLengthBeats;
    return juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
}

} // namespace

void ReggaetonSubBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const bool hit = pos == 0 || pos == 6 || pos == 10 || (density > 0.45f && pos == 14);
        if (!hit) continue;
        const int degree = (complexity > 0.55f && (pos == 10 || pos == 14)) ? 4 : 0;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 106, bassSampleForStep(step, 0.25, playheadBeats, blockLengthBeats, blockSamples), 0.35, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void ElectroFunkBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    static constexpr int pattern[] = {0, 0, 5, 0, 7, 5, 3, 0};
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (density < 0.25f && !(pos == 0 || pos == 4)) continue;
        int degree = pattern[pos];
        if (complexity > 0.65f && pos % 3 == 2) degree += 2;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, pos == 4 ? 3 : 2), 96, bassSampleForStep(step, 0.5, playheadBeats, blockLengthBeats, blockSamples), 0.32, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void MinimalDroneBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.6f ? 1.0 : 2.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        const int degree = (complexity > 0.65f && std::fmod(next, 4.0) >= 2.0) ? 5 : 0;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(((next - playheadBeats) / blockLengthBeats) * blockSamples));
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 112, sampleIdx, 1.75, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void BrokenOctaveBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const bool hit = pos == 0 || pos == 5 || pos == 8 || (density > 0.55f && (pos == 11 || pos == 14));
        if (!hit) continue;
        const int octave = (pos == 5 || pos == 11) ? 3 : 2;
        const int degree = (complexity > 0.6f && pos >= 8) ? 7 : 0;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, octave), 98, bassSampleForStep(step, 0.25, playheadBeats, blockLengthBeats, blockSamples), 0.28, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void StepperDubBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        const bool hit = pos == 0 || pos == 3 || (density > 0.5f && (pos == 5 || pos == 7));
        if (!hit) continue;
        const int degree = (complexity > 0.6f && pos >= 5) ? 2 : 0;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 104, bassSampleForStep(step, 0.5, playheadBeats, blockLengthBeats, blockSamples), 0.45, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void FunkPopBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    static constexpr int degrees[] = {0, 0, 2, 4, 5, 4, 2, 0};
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (density < 0.2f && !(pos == 0 || pos == 4)) continue;
        const int degree = degrees[pos] + ((complexity > 0.7f && pos == 6) ? 5 : 0);
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), pos == 0 ? 104 : 88, bassSampleForStep(step, 0.5, playheadBeats, blockLengthBeats, blockSamples), 0.34, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void NeuroWobbleBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.55f ? 0.25 : 0.5;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        const int subStep = static_cast<int>(std::floor(next / grid));
        if (density < 0.2f && subStep % 4 != 0) continue;
        int degree = (subStep % 4 == 2) ? 5 : 0;
        if (complexity > 0.65f && subStep % 8 >= 6) degree = 7;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(((next - playheadBeats) / blockLengthBeats) * blockSamples));
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 100, sampleIdx, grid * 0.7, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void ClaveBass::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const bool clave = pos == 0 || pos == 3 || pos == 6 || pos == 10 || pos == 12;
        if (!clave || (density < 0.2f && !(pos == 0 || pos == 10))) continue;
        const int degree = (complexity > 0.55f && pos == 12) ? 4 : 0;
        addScheduledNoteAtSample(buffer, 11, scale.getDegree(degree, 2), 94, bassSampleForStep(step, 0.25, playheadBeats, blockLengthBeats, blockSamples), 0.22, playheadBeats, blockLengthBeats, blockSamples);
    }
}
