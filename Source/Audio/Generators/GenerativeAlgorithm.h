#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Harmony/Scale.h"
#include <algorithm>
#include <vector>

class GenerativeAlgorithm {
public:
    virtual ~GenerativeAlgorithm() = default;

    // Fill midiBuffer with generated notes for the given block
    // playheadBeats: position in beats (fractional)
    // blockLengthBeats: duration of this block in beats
    virtual void processMidi(juce::MidiBuffer& buffer,
                             double playheadBeats,
                             double blockLengthBeats,
                             int blockSamples,
                             const Scale& scale,
                             int rootNote,
                             float density,
                             float complexity) = 0;

    virtual void reset() = 0;
    virtual juce::String getName() const = 0;

    void flushPendingNoteOffs(juce::MidiBuffer& buffer,
                              double playheadBeats,
                              double blockLengthBeats,
                              int blockSamples) {
        if (blockSamples <= 0 || blockLengthBeats <= 0.0) {
            return;
        }

        const double blockEndBeats = playheadBeats + blockLengthBeats;
        auto writeIt = pendingNoteOffs_.begin();
        for (const auto& noteOff : pendingNoteOffs_) {
            if (noteOff.beat < playheadBeats) {
                buffer.addEvent(juce::MidiMessage::noteOff(noteOff.channel, noteOff.note), 0);
            } else if (noteOff.beat < blockEndBeats) {
                const double fraction = (noteOff.beat - playheadBeats) / blockLengthBeats;
                const int sample = juce::jlimit(0, blockSamples - 1, static_cast<int>(fraction * blockSamples));
                buffer.addEvent(juce::MidiMessage::noteOff(noteOff.channel, noteOff.note), sample);
            } else {
                *writeIt++ = noteOff;
            }
        }
        pendingNoteOffs_.erase(writeIt, pendingNoteOffs_.end());
    }

    void addScheduledNote(juce::MidiBuffer& buffer,
                          int channel,
                          int note,
                          int velocity,
                          double startBeat,
                          double durationBeats,
                          double playheadBeats,
                          double blockLengthBeats,
                          int blockSamples) {
        if (blockSamples <= 0 || blockLengthBeats <= 0.0 || durationBeats <= 0.0) {
            return;
        }

        const double blockEndBeats = playheadBeats + blockLengthBeats;
        if (startBeat < playheadBeats || startBeat >= blockEndBeats) {
            return;
        }

        const double onFraction = (startBeat - playheadBeats) / blockLengthBeats;
        const int onSample = juce::jlimit(0, blockSamples - 1, static_cast<int>(onFraction * blockSamples));
        const int clampedNote = juce::jlimit(0, 127, note);
        buffer.addEvent(juce::MidiMessage::noteOn(channel,
                                                  clampedNote,
                                                  static_cast<juce::uint8>(juce::jlimit(1, 127, velocity))),
                        onSample);

        const double offBeat = startBeat + durationBeats;
        if (offBeat < blockEndBeats) {
            const double offFraction = (offBeat - playheadBeats) / blockLengthBeats;
            int offSample = juce::jlimit(0, blockSamples - 1, static_cast<int>(offFraction * blockSamples));
            if (offSample <= onSample) {
                offSample = juce::jmin(blockSamples - 1, onSample + 1);
            }
            buffer.addEvent(juce::MidiMessage::noteOff(channel, clampedNote), offSample);
        } else {
            pendingNoteOffs_.push_back(PendingNoteOff{offBeat, channel, clampedNote});
            std::sort(pendingNoteOffs_.begin(), pendingNoteOffs_.end(),
                      [](const PendingNoteOff& a, const PendingNoteOff& b) {
                          return a.beat < b.beat;
                      });
        }
    }

    void addScheduledNoteAtSample(juce::MidiBuffer& buffer,
                                  int channel,
                                  int note,
                                  int velocity,
                                  int samplePos,
                                  double durationBeats,
                                  double playheadBeats,
                                  double blockLengthBeats,
                                  int blockSamples) {
        const double startBeat = playheadBeats
            + (static_cast<double>(juce::jlimit(0, std::max(0, blockSamples - 1), samplePos))
               / static_cast<double>(std::max(1, blockSamples)))
            * blockLengthBeats;
        addScheduledNote(buffer, channel, note, velocity, startBeat, durationBeats,
                         playheadBeats, blockLengthBeats, blockSamples);
    }

    void clearPendingNoteOffs() {
        pendingNoteOffs_.clear();
    }

private:
    struct PendingNoteOff {
        double beat = 0.0;
        int channel = 1;
        int note = 60;
    };

    std::vector<PendingNoteOff> pendingNoteOffs_;
};
