#include "GrooveProcessor.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace cendance {

void GrooveProcessor::prepare(double sampleRate, int maxBlockSize) {
    sampleRate_ = sampleRate;
    maxBlockSize_ = maxBlockSize;
    rng_.seed(42); // Deterministic seed for reproducibility
}

void GrooveProcessor::reset() {
    outputBuffer_.clear();
}

void GrooveProcessor::apply(juce::MidiBuffer& midi,
                            double playheadBeats,
                            double blockLengthBeats,
                            int numSamples,
                            float swingAmount,
                            float velocityHumanize,
                            float timingJitter) {
    if (numSamples <= 0) return;

    // If all effects are zero, no transformation needed
    const bool hasSwing = swingAmount > 0.001f;
    const bool hasVelocity = velocityHumanize > 0.001f;
    const bool hasJitter = timingJitter > 0.001f;
    if (!hasSwing && !hasVelocity && !hasJitter) return;

    const double actualSamplesPerBeat = (blockLengthBeats > 0.0)
        ? static_cast<double>(numSamples) / blockLengthBeats
        : 0.0;
    const double eighthNoteSamples = actualSamplesPerBeat * 0.5;

    const double maxSwingSamples = swingAmount * 0.5 * eighthNoteSamples;
    const double maxJitterSamples = timingJitter * (kMaxJitterMs / 1000.0) * sampleRate_;
    const float maxVelOffset = velocityHumanize * kMaxVelocityOffset;

    std::uniform_real_distribution<double> swingDist(0.0, 1.0);
    std::uniform_real_distribution<double> jitterDist(-1.0, 1.0);
    std::uniform_real_distribution<float> velDist(-1.0f, 1.0f);

    outputBuffer_.clear();

    // First pass: collect all events and compute shifts for NoteOns
    struct EventInfo {
        juce::MidiMessage msg;
        int samplePos;
        double absoluteBeat;
        int channel;
        int noteNumber;
        bool isNoteOn;
        bool isNoteOff;
        double timingShift = 0.0; // samples to shift (same for paired NoteOn/NoteOff)
    };

    std::vector<EventInfo> events;
    events.reserve(midi.getNumEvents());

    // Map to track NoteOn shifts by (channel, note) for pairing with NoteOffs
    // Key: (channel << 7) | noteNumber
    std::unordered_map<int, double> noteOnShifts;

    for (const auto meta : midi) {
        auto msg = meta.getMessage();
        int samplePos = meta.samplePosition;

        const double beatOffset = (numSamples > 0)
            ? (static_cast<double>(samplePos) / static_cast<double>(numSamples)) * blockLengthBeats
            : 0.0;
        const double absoluteBeat = playheadBeats + beatOffset;

        const bool isNoteOn = msg.isNoteOn();
        const bool isNoteOff = msg.isNoteOff();
        int channel = msg.getChannel();
        int noteNumber = -1;
        if (isNoteOn || isNoteOff) noteNumber = msg.getNoteNumber();

        EventInfo info;
        info.msg = msg;
        info.samplePos = samplePos;
        info.absoluteBeat = absoluteBeat;
        info.channel = channel;
        info.noteNumber = noteNumber;
        info.isNoteOn = isNoteOn;
        info.isNoteOff = isNoteOff;

        // Compute timing shift for NoteOns (swing + jitter)
        double totalShift = 0.0;

        if (isNoteOn) {
            // --- Swing: shift even 8th notes ---
            if (hasSwing && maxSwingSamples > 0.5) {
                const double eighthPos = absoluteBeat * 2.0;
                const double nearest8th = std::round(eighthPos);
                const double frac = std::abs(eighthPos - nearest8th);
                if (frac < 0.15) {
                    const int8_t eighthIndex = static_cast<int8_t>(static_cast<int>(nearest8th) & 0xFF);
                    if (eighthIndex % 2 != 0) {
                        const double swingShift = maxSwingSamples * swingDist(rng_);
                        totalShift += swingShift;
                    }
                }
            }

            // --- Timing Jitter ---
            if (hasJitter && maxJitterSamples > 0.5) {
                const double jitterShift = maxJitterSamples * jitterDist(rng_);
                totalShift += jitterShift;
            }

            info.timingShift = totalShift;

            // Store shift for pairing with NoteOff
            if (noteNumber >= 0) {
                int key = (channel << 7) | noteNumber;
                noteOnShifts[key] = totalShift;
            }
        } else if (isNoteOff && noteNumber >= 0) {
            // Look up shift from paired NoteOn
            int key = (channel << 7) | noteNumber;
            auto it = noteOnShifts.find(key);
            if (it != noteOnShifts.end()) {
                info.timingShift = it->second;
            }
        }

        events.push_back(std::move(info));
    }

    // Second pass: apply shifts and velocity humanization
    for (auto& info : events) {
        int newSamplePos = info.samplePos;

        // Apply timing shift (same for paired NoteOn/NoteOff)
        if (info.timingShift != 0.0) {
            const int offsetSamples = static_cast<int>(std::round(info.timingShift));
            newSamplePos = juce::jlimit(0, numSamples - 1, info.samplePos + offsetSamples);
        }

        // --- Velocity Humanization: random velocity offset (NoteOn only) ---
        auto msg = info.msg;
        if (hasVelocity && maxVelOffset > 0.5f && info.isNoteOn) {
            const int originalVel = msg.getVelocity();
            const float offset = maxVelOffset * velDist(rng_);
            const int newVel = juce::jlimit(1, 127, static_cast<int>(originalVel + offset));
            msg = juce::MidiMessage::noteOn(msg.getChannel(), msg.getNoteNumber(),
                                            static_cast<juce::uint8>(newVel));
        }

        outputBuffer_.addEvent(msg, newSamplePos);
    }

    // Swap: replace input with transformed output
    midi.swapWith(outputBuffer_);
}

} // namespace cendance
