#include "LeadStyleAlgorithms.h"

#include "GeneratorUtils.h"

#include <cmath>
#include <random>


void TranceContourLead::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale& scale,
                                    int,
                                    float density,
                                    float complexity) {
    static constexpr int contour[] = {0, 2, 4, 7, 9, 7, 4, 2};
    const double grid = density > 0.65f ? 0.25 : 0.5;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        const int idx = step_ % static_cast<int>(std::size(contour));
        int degree = contour[idx];
        if (complexity > 0.55f && idx % 2 == 1) {
            degree += 2;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 94, sampleIdx, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);

        ++step_;
        next += grid;
    }
}

void RaveStabLead::processMidi(juce::MidiBuffer& buffer,
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

        bool hit = (pos == 0 || pos == 3 || pos == 6);
        if (density > 0.6f && pos == 7) {
            hit = true;
        }
        if (!hit) {
            continue;
        }

        const double fraction = (step * 0.5 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));

        int degree = (pos == 6 && complexity > 0.5f) ? 7 : 0;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 102, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void AfroCallResponseLead::processMidi(juce::MidiBuffer& buffer,
                                       double playheadBeats,
                                       double blockLengthBeats,
                                       int blockSamples,
                                       const Scale& scale,
                                       int,
                                       float density,
                                       float complexity) {
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    const double grid = 0.5;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        const int phase = static_cast<int>(std::floor(next)) % 2;
        int degree = (phase == 0) ? 2 : 5;
        if (complexity > 0.7f && phase == 1) {
            degree = 9;
        }

        const bool anchor = std::fmod(next, 2.0) < 1.0e-6;
        if (density > 0.25f || anchor) {
            const double fraction = (next - start) / blockLengthBeats;
            const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
            addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 88, sampleIdx, 0.45, playheadBeats, blockLengthBeats, blockSamples);
        }

        next += grid;
    }
}

void CinematicSparseLead::processMidi(juce::MidiBuffer& buffer,
                                      double playheadBeats,
                                      double blockLengthBeats,
                                      int blockSamples,
                                      const Scale& scale,
                                      int,
                                      float density,
                                      float complexity) {
    int sampleIdx = 0;
    if (!GeneratorUtils::checkHit(playheadBeats, blockLengthBeats, 2.0, blockSamples, sampleIdx)) {
        return;
    }

    const int degree = complexity > 0.6f ? 9 : 5;
    const int note = scale.getDegree(degree, 5);
    addScheduledNoteAtSample(buffer, 13, note, 84, sampleIdx, 1.25 + density * 0.5, playheadBeats, blockLengthBeats, blockSamples);
}

void EuclideanLeadGate::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale& scale,
                                    int,
                                    float density,
                                    float complexity) {
    const int steps = 16;
    const int pulses = juce::jlimit(2, 11, static_cast<int>(2 + density * 9.0f));
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;

    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        int pos = step % steps;
        if (pos < 0) {
            pos += steps;
        }

        const bool hit = ((pos * pulses) % steps) < pulses;
        if (!hit) {
            continue;
        }

        const double fraction = (step * 0.25 - playheadBeats) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        const int degree = (complexity > 0.5f && (pos % 4 == 3)) ? 7 : 0;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 90, sampleIdx, 0.18, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void PhraseMutatorLead::processMidi(juce::MidiBuffer& buffer,
                                    double playheadBeats,
                                    double blockLengthBeats,
                                    int blockSamples,
                                    const Scale& scale,
                                    int,
                                    float density,
                                    float complexity) {
    static constexpr int phrase[] = {0, 2, 4, 2, 7, 9, 7, 4};

    const double grid = density > 0.55f ? 0.25 : 0.5;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        std::mt19937 rng(static_cast<uint32_t>((phraseIndex_ + 1) * 7919));
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        int degree = phrase[phraseIndex_ % static_cast<int>(std::size(phrase))];
        if (dist(rng) < complexity * 0.5f) {
            degree += (dist(rng) > 0.5f ? 1 : -1) * 2;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 92, sampleIdx, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);

        ++phraseIndex_;
        next += grid;
    }
}

void GlideRunLead::processMidi(juce::MidiBuffer& buffer,
                               double playheadBeats,
                               double blockLengthBeats,
                               int blockSamples,
                               const Scale& scale,
                               int,
                               float density,
                               float complexity) {
    static constexpr int glidePattern[] = {0, 2, 4, 7, 11, 7, 4, 2};
    const double grid = density > 0.6f ? 0.25 : 0.5;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        if (density < 0.15f && glideStep_ % 4 != 0) {
            ++glideStep_;
            next += grid;
            continue;
        }

        int degree = glidePattern[glideStep_ % static_cast<int>(std::size(glidePattern))];
        if (complexity > 0.65f && (glideStep_ % 4 == 3)) {
            degree += 2;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 96, sampleIdx, grid * 0.8, playheadBeats, blockLengthBeats, blockSamples);

        ++glideStep_;
        next += grid;
    }
}

void MicroMotifLead::processMidi(juce::MidiBuffer& buffer,
                                 double playheadBeats,
                                 double blockLengthBeats,
                                 int blockSamples,
                                 const Scale& scale,
                                 int,
                                 float density,
                                 float complexity) {
    static constexpr int motif[] = {0, 1, 3, 1, 4, 3, 1, 0};
    const double grid = density > 0.55f ? 0.25 : 0.5;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        int degree = motif[motifCursor_ % static_cast<int>(std::size(motif))];
        if (complexity > 0.5f && (motifCursor_ % 3 == 2)) {
            degree += 2;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        const int velocity = (motifCursor_ % 4 == 0) ? 92 : 78;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), velocity, sampleIdx, grid * 0.7, playheadBeats, blockLengthBeats, blockSamples);

        ++motifCursor_;
        next += grid;
    }
}

void WideIntervalLead::processMidi(juce::MidiBuffer& buffer,
                                   double playheadBeats,
                                   double blockLengthBeats,
                                   int blockSamples,
                                   const Scale& scale,
                                   int,
                                   float density,
                                   float complexity) {
    const double grid = density > 0.65f ? 0.5 : 1.0;
    const double start = playheadBeats;
    const double end = playheadBeats + blockLengthBeats;
    double next = std::ceil(start / grid) * grid;

    while (next < end) {
        const double phase = std::fmod(next, 2.0);
        if (density < 0.2f && phase >= 1.0) {
            next += grid;
            continue;
        }
        int degree = phase < 1.0 ? 0 : 10;
        if (complexity > 0.55f && phase > 1.4) {
            degree = 14;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 90, sampleIdx, grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

void TripletRushLead::processMidi(juce::MidiBuffer& buffer,
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
        if (density < 0.2f && tripletStep % 6 != 0) {
            next += grid;
            continue;
        }
        const bool phraseRest = density < 0.55f && (tripletStep % 12 == 11);
        if (phraseRest) {
            next += grid;
            continue;
        }

        int degree = (tripletStep % 3 == 0) ? 0 : ((tripletStep % 3 == 1) ? 2 : 4);
        if (complexity > 0.6f && tripletStep % 6 >= 3) {
            degree += 5;
        }
        if (complexity > 0.75f && tripletStep % 12 >= 9) {
            degree += 7;
        }

        const double fraction = (next - start) / blockLengthBeats;
        const int sampleIdx = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
        const int velocity = (tripletStep % 3 == 0) ? 96 : 82;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), velocity, sampleIdx, 0.22, playheadBeats, blockLengthBeats, blockSamples);
        next += grid;
    }
}

namespace {

int leadSampleForBeat(double beat, double playheadBeats, double blockLengthBeats, int blockSamples) {
    const double fraction = (beat - playheadBeats) / blockLengthBeats;
    return juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
}

} // namespace

void PentatonicHookLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    static constexpr int motif[] = {0, 2, 4, 7, 4, 2, 0, 9};
    const double grid = density > 0.55f ? 0.25 : 0.5;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        if (density < 0.2f && step_ % 4 != 0) { ++step_; continue; }
        int degree = motif[step_ % static_cast<int>(std::size(motif))];
        if (complexity > 0.65f && step_ % 8 == 7) degree += 2;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 90, leadSampleForBeat(next, playheadBeats, blockLengthBeats, blockSamples), grid * 0.75, playheadBeats, blockLengthBeats, blockSamples);
        ++step_;
    }
}

void AcidLineLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.5f ? 0.25 : 0.5;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        if (density < 0.2f && step_ % 4 != 0) { ++step_; continue; }
        int degree = (step_ % 4 == 0) ? 0 : ((step_ % 4 == 1) ? 1 : 4);
        if (complexity > 0.6f && step_ % 8 >= 6) degree = 7;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 4), step_ % 4 == 0 ? 104 : 84, leadSampleForBeat(next, playheadBeats, blockLengthBeats, blockSamples), grid * 0.65, playheadBeats, blockLengthBeats, blockSamples);
        ++step_;
    }
}

void DubEchoLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start8 = playheadBeats * 2.0;
    const double end8 = (playheadBeats + blockLengthBeats) * 2.0;
    for (int step = static_cast<int>(std::ceil(start8)); step < end8; ++step) {
        const int pos = (step % 8 + 8) % 8;
        if (!(pos == 0 || pos == 5 || (density > 0.5f && pos == 7))) continue;
        const int degree = (complexity > 0.55f && pos >= 5) ? 7 : 2;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 82, leadSampleForBeat(step * 0.5, playheadBeats, blockLengthBeats, blockSamples), 0.5, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void GarageVoxLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        if (!(pos == 2 || pos == 7 || pos == 10 || (density > 0.55f && pos == 14))) continue;
        const int degree = (pos == 7 && complexity > 0.5f) ? 9 : ((pos == 10) ? 5 : 2);
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 88, leadSampleForBeat(step * 0.25, playheadBeats, blockLengthBeats, blockSamples), 0.22, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void MinimalPingLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.65f ? 1.0 : 2.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        const int degree = (complexity > 0.6f && std::fmod(next, 4.0) >= 2.0) ? 7 : 0;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 6), 78, leadSampleForBeat(next, playheadBeats, blockLengthBeats, blockSamples), 0.18, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void OrnamentRunLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    static constexpr int run[] = {0, 1, 2, 4, 5, 7, 9, 7};
    const double grid = density > 0.45f ? 0.25 : 0.5;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        if (density < 0.18f && step_ % 4 != 0) { ++step_; continue; }
        int degree = run[step_ % static_cast<int>(std::size(run))];
        if (complexity > 0.65f && step_ % 8 >= 4) degree += 2;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 92, leadSampleForBeat(next, playheadBeats, blockLengthBeats, blockSamples), grid * 0.7, playheadBeats, blockLengthBeats, blockSamples);
        ++step_;
    }
}

void SyncopatedPluckLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double start16 = playheadBeats * 4.0;
    const double end16 = (playheadBeats + blockLengthBeats) * 4.0;
    for (int step = static_cast<int>(std::ceil(start16)); step < end16; ++step) {
        const int pos = (step % 16 + 16) % 16;
        const bool hit = pos == 0 || pos == 3 || pos == 9 || (density > 0.45f && (pos == 6 || pos == 14));
        if (!hit) continue;
        const int degree = (complexity > 0.55f && pos >= 9) ? 7 : 0;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 94, leadSampleForBeat(step * 0.25, playheadBeats, blockLengthBeats, blockSamples), 0.22, playheadBeats, blockLengthBeats, blockSamples);
    }
}

void LydianFloatLead::processMidi(juce::MidiBuffer& buffer, double playheadBeats, double blockLengthBeats, int blockSamples, const Scale& scale, int, float density, float complexity) {
    const double grid = density > 0.55f ? 0.5 : 1.0;
    for (double next = std::ceil(playheadBeats / grid) * grid; next < playheadBeats + blockLengthBeats; next += grid) {
        static constexpr int tones[] = {0, 4, 7, 11, 14, 11, 7, 4};
        int degree = tones[step_ % static_cast<int>(std::size(tones))];
        if (complexity > 0.7f && step_ % 5 == 4) degree += 2;
        addScheduledNoteAtSample(buffer, 13, scale.getDegree(degree, 5), 84, leadSampleForBeat(next, playheadBeats, blockLengthBeats, blockSamples), grid * 0.9, playheadBeats, blockLengthBeats, blockSamples);
        ++step_;
    }
}
