#include "MelodicSampler.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

struct SamplerPresetSpec {
    float attackSeconds;
    float releaseSeconds;
    float stereoSpread;
    float output;
    bool preferLoop;
};

constexpr std::array<SamplerPresetSpec, 18> kSamplerPresetSpecs = {{
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
    {0.004f, 0.12f, 0.05f, 0.62f, false},
    {0.002f, 0.18f, 0.10f, 0.58f, false},
    {0.022f, 0.70f, 0.24f, 0.48f, true},
}};

const SamplerPresetSpec& getSamplerPreset(uint8_t preset) {
    return kSamplerPresetSpecs[static_cast<size_t>(preset % kSamplerPresetSpecs.size())];
}

bool isUsableRegion(const MelodicSamplerEngine::Region& region) {
    return region.audio != nullptr
        && region.audio->getNumSamples() > 1
        && region.audio->getNumChannels() > 0;
}

} // namespace

void MelodicSamplerEngine::setRegion(uint8_t regionIndex, const Region& region) {
    if (regionIndex >= RegionCount) {
        return;
    }

    Region normalized = region;
    normalized.rootNote = std::clamp(normalized.rootNote, 0, 127);
    normalized.lowNote = std::clamp(normalized.lowNote, 0, 127);
    normalized.highNote = std::clamp(normalized.highNote, 0, 127);
    if (normalized.lowNote > normalized.highNote) {
        std::swap(normalized.lowNote, normalized.highNote);
    }
    normalized.gain = std::clamp(normalized.gain, 0.0f, 2.0f);
    normalized.startOffset = std::clamp(normalized.startOffset, 0.0f, 1.0f);
    normalized.endOffset = std::clamp(normalized.endOffset, normalized.startOffset, 1.0f);
    normalized.loopStart = std::clamp(normalized.loopStart, normalized.startOffset, normalized.endOffset);
    normalized.loopEnd = std::clamp(normalized.loopEnd, normalized.loopStart, normalized.endOffset);
    normalized.sourceSampleRate = normalized.sourceSampleRate > 0.0 ? normalized.sourceSampleRate : sampleRate_;

    regions_[regionIndex] = normalized;
}

void MelodicSamplerEngine::clearRegion(uint8_t regionIndex) {
    if (regionIndex < RegionCount) {
        regions_[regionIndex] = Region{};
    }
}

void MelodicSamplerEngine::clearRegions() {
    for (auto& region : regions_) {
        region = Region{};
    }
    reset();
}

bool MelodicSamplerEngine::hasAssignedSamples() const {
    for (const auto& region : regions_) {
        if (isUsableRegion(region)) {
            return true;
        }
    }
    return false;
}

void MelodicSamplerEngine::setPreset(uint8_t preset) {
    preset_ = preset;
}

void MelodicSamplerEngine::setTone(float tone) {
    tone_ = std::clamp(tone, 0.0f, 1.0f);
}

void MelodicSamplerEngine::setMotion(float motion) {
    motion_ = std::clamp(motion, 0.0f, 1.0f);
}

void MelodicSamplerEngine::prepare(double sampleRate, int blockSize) {
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();
}

void MelodicSamplerEngine::reset() {
    for (auto& voice : voices_) {
        voice = Voice{};
    }
    lowStateL_ = 0.0f;
    lowStateR_ = 0.0f;
}

const MelodicSamplerEngine::Region* MelodicSamplerEngine::chooseRegionForNote(int midiNote) const {
    const Region* bestRegion = nullptr;
    int bestDistance = std::numeric_limits<int>::max();

    for (const auto& region : regions_) {
        if (!isUsableRegion(region) || midiNote < region.lowNote || midiNote > region.highNote) {
            continue;
        }

        const int distance = std::abs(midiNote - region.rootNote);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestRegion = &region;
        }
    }

    return bestRegion;
}

void MelodicSamplerEngine::triggerVoice(int midiNote, float velocity) {
    const Region* region = chooseRegionForNote(midiNote);
    if (region == nullptr || region->audio == nullptr) {
        return;
    }

    Voice* targetVoice = nullptr;
    for (auto& voice : voices_) {
        if (!voice.active) {
            targetVoice = &voice;
            break;
        }
    }
    if (targetVoice == nullptr) {
        targetVoice = &voices_[0];
    }

    const auto& audio = *region->audio;
    const int lastSample = std::max(1, audio.getNumSamples() - 1);
    const int startSample = std::clamp(static_cast<int>(region->startOffset * static_cast<float>(lastSample)),
                                       0,
                                       lastSample);

    const double semitones = static_cast<double>(midiNote - region->rootNote) + region->tuneSemitones;
    const double pitchRatio = std::pow(2.0, semitones / 12.0);
    const double sourceRateRatio = region->sourceSampleRate / sampleRate_;

    const auto& spec = getSamplerPreset(preset_);
    const float motion = std::clamp(motion_, 0.0f, 1.0f);
    const float attackSeconds = spec.attackSeconds * (1.4f - 0.7f * motion);
    const float releaseSeconds = spec.releaseSeconds * (0.55f + 1.4f * motion);
    const int attackSamples = std::max(1, static_cast<int>(sampleRate_ * attackSeconds));

    const float voicePan = (static_cast<float>((midiNote % 7) - 3) / 3.0f) * spec.stereoSpread;

    targetVoice->region = region;
    targetVoice->samplePosition = static_cast<double>(startSample);
    targetVoice->increment = std::max(0.001, pitchRatio * sourceRateRatio);
    targetVoice->gain = std::clamp(velocity, 0.0f, 1.0f) * region->gain * spec.output;
    targetVoice->envelope = 1.0f;
    targetVoice->attack = 0.0f;
    targetVoice->attackStep = 1.0f / static_cast<float>(attackSamples);
    targetVoice->releaseMul = std::exp(-1.0f / std::max(1.0, sampleRate_ * static_cast<double>(releaseSeconds)));
    targetVoice->pan = std::clamp(voicePan, -0.8f, 0.8f);
    targetVoice->note = midiNote;
    targetVoice->active = true;
    targetVoice->releasing = false;
}

void MelodicSamplerEngine::releaseVoicesForNote(int midiNote) {
    for (auto& voice : voices_) {
        if (voice.active && voice.note == midiNote) {
            voice.releasing = true;
        }
    }
}

void MelodicSamplerEngine::renderVoice(Voice& voice, float& leftOut, float& rightOut) {
    if (!voice.active || voice.region == nullptr || voice.region->audio == nullptr) {
        return;
    }

    const auto& region = *voice.region;
    const auto& audio = *region.audio;
    const int numSamples = audio.getNumSamples();
    const int numChannels = audio.getNumChannels();
    const int lastSample = numSamples - 1;
    const int endSample = std::clamp(static_cast<int>(region.endOffset * static_cast<float>(lastSample)),
                                     1,
                                     lastSample);

    if (voice.samplePosition < 0.0 || voice.samplePosition >= static_cast<double>(endSample)) {
        voice.active = false;
        return;
    }

    const int index = std::clamp(static_cast<int>(voice.samplePosition), 0, lastSample);
    const int nextIndex = std::min(index + 1, lastSample);
    const float frac = static_cast<float>(voice.samplePosition - static_cast<double>(index));

    const float aL = audio.getSample(0, index);
    const float bL = audio.getSample(0, nextIndex);
    const float sampleL = aL + (bL - aL) * frac;

    float sampleR = sampleL;
    if (numChannels > 1) {
        const float aR = audio.getSample(1, index);
        const float bR = audio.getSample(1, nextIndex);
        sampleR = aR + (bR - aR) * frac;
    }

    voice.attack = std::min(1.0f, voice.attack + voice.attackStep);
    if (voice.releasing) {
        voice.envelope *= voice.releaseMul;
    }

    const float env = voice.envelope * voice.attack;
    const float leftGain = 0.5f * (1.0f - voice.pan);
    const float rightGain = 0.5f * (1.0f + voice.pan);
    leftOut += sampleL * voice.gain * env * leftGain;
    rightOut += sampleR * voice.gain * env * rightGain;

    voice.samplePosition += voice.increment;

    const auto& spec = getSamplerPreset(preset_);
    const bool shouldLoop = region.loop || spec.preferLoop;
    if (shouldLoop && !voice.releasing) {
        const int loopStart = std::clamp(static_cast<int>(region.loopStart * static_cast<float>(lastSample)),
                                         0,
                                         lastSample);
        const int loopEnd = std::clamp(static_cast<int>(region.loopEnd * static_cast<float>(lastSample)),
                                       loopStart + 1,
                                       endSample);
        if (voice.samplePosition >= static_cast<double>(loopEnd)) {
            voice.samplePosition = static_cast<double>(loopStart)
                + std::fmod(voice.samplePosition - static_cast<double>(loopStart),
                            static_cast<double>(std::max(1, loopEnd - loopStart)));
        }
    }

    if (voice.samplePosition >= static_cast<double>(endSample) || voice.envelope < 1.0e-4f) {
        voice.active = false;
    }
}

void MelodicSamplerEngine::applyTone(float& leftOut, float& rightOut) {
    const float cutoff = 260.0f + tone_ * tone_ * 9800.0f;
    const float coeff = std::clamp(1.0f - std::exp(-(2.0f * juce::MathConstants<float>::pi * cutoff)
                                                   / static_cast<float>(sampleRate_)),
                                   0.0f,
                                   1.0f);

    lowStateL_ += coeff * (leftOut - lowStateL_);
    lowStateR_ += coeff * (rightOut - lowStateR_);

    const float highAmount = std::max(0.0f, (tone_ - 0.5f) * 0.7f);
    leftOut = lowStateL_ + (leftOut - lowStateL_) * highAmount;
    rightOut = lowStateR_ + (rightOut - lowStateR_) * highAmount;
}

void MelodicSamplerEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
                                           const juce::MidiBuffer& midi,
                                           int numSamples) {
    buffer.clear();

    int samplePos = 0;
    auto renderFrame = [&]() {
        float outL = 0.0f;
        float outR = 0.0f;
        for (auto& voice : voices_) {
            renderVoice(voice, outL, outR);
        }

        applyTone(outL, outR);
        buffer.addSample(0, samplePos, outL);
        if (buffer.getNumChannels() > 1) {
            buffer.addSample(1, samplePos, outR);
        }
        ++samplePos;
    };

    for (const auto meta : midi) {
        const int eventTime = std::clamp(meta.samplePosition, 0, numSamples);
        while (samplePos < eventTime) {
            renderFrame();
        }

        const auto msg = meta.getMessage();
        if (msg.isNoteOn()) {
            triggerVoice(msg.getNoteNumber(), msg.getFloatVelocity());
        } else if (msg.isNoteOff()) {
            releaseVoicesForNote(msg.getNoteNumber());
        }
    }

    while (samplePos < numSamples) {
        renderFrame();
    }
}
