#include "ChordEngine.h"
#include "DspHelpers.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

enum class ChordMode : uint8_t {
    Warm,
    Stab,
    Glass,
    Velvet,
    Organ,
    MetalPluck,
    AirChoir,
    DetuneCloud,
    PwmKeys,
    FmBloom,
    Sweep,
    Dust
};

struct ChordPresetSpec {
    ChordMode mode;
    float color;
    float spread;
    float body;
};

constexpr std::array<ChordPresetSpec, 20> kChordPresetSpecs = {{
    {ChordMode::Warm, 0.22f, 0.20f, 0.76f},
    {ChordMode::Stab, 0.66f, 0.28f, 0.50f},
    {ChordMode::Glass, 0.78f, 0.54f, 0.42f},
    {ChordMode::Velvet, 0.32f, 0.30f, 0.84f},
    {ChordMode::Organ, 0.46f, 0.18f, 0.74f},
    {ChordMode::MetalPluck, 0.90f, 0.45f, 0.46f},
    {ChordMode::AirChoir, 0.26f, 0.64f, 0.72f},
    {ChordMode::DetuneCloud, 0.38f, 0.88f, 0.70f},
    {ChordMode::PwmKeys, 0.62f, 0.36f, 0.56f},
    {ChordMode::FmBloom, 0.70f, 0.58f, 0.52f},
    {ChordMode::Sweep, 0.58f, 0.74f, 0.54f},
    {ChordMode::Dust, 0.18f, 0.48f, 0.40f},
    {ChordMode::Velvet, 0.28f, 0.40f, 0.88f},
    {ChordMode::Organ, 0.54f, 0.24f, 0.78f},
    {ChordMode::FmBloom, 0.82f, 0.62f, 0.46f},
    {ChordMode::Sweep, 0.64f, 0.86f, 0.50f},
    {ChordMode::DetuneCloud, 0.44f, 0.96f, 0.66f},
    {ChordMode::MetalPluck, 0.94f, 0.52f, 0.42f},
    {ChordMode::PwmKeys, 0.68f, 0.48f, 0.52f},
    {ChordMode::Dust, 0.24f, 0.58f, 0.36f}
}};

constexpr uint8_t kMaxChordPresetId = static_cast<uint8_t>(kChordPresetSpecs.size() - 1);
constexpr uint8_t kMelodicSamplerPresetStart = 20;

const ChordPresetSpec& getChordPreset(uint8_t preset) {
    return kChordPresetSpecs[std::min<uint8_t>(preset, kMaxChordPresetId)];
}

} // namespace

// SynthVoice Implementation
SynthVoice::SynthVoice() {
    osc1_.initialise([](float x) { return (x / juce::MathConstants<float>::pi) - 1.0f; });
    osc2_.initialise([](float x) { return std::asin(std::sin(x)) * (2.0f / juce::MathConstants<float>::pi); });
    slowLfo_.initialise([](float x) { return std::sin(x); });
    
    juce::ADSR::Parameters envParams;
    envParams.attack = 0.06f;
    envParams.decay = 0.5f;
    envParams.sustain = 0.72f;
    envParams.release = 0.9f;
    env_.setParameters(envParams);
    
    filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void SynthVoice::prepare(const juce::dsp::ProcessSpec& spec) {
    osc1_.prepare(spec);
    osc2_.prepare(spec);
    slowLfo_.prepare(spec);
    filter_.prepare(spec);
    env_.setSampleRate(spec.sampleRate);
    slowLfo_.setFrequency(0.25f);
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound) {
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) {
    juce::ignoreUnused(currentPitchWheelPosition);

    currentNote_ = midiNoteNumber;
    baseFreq_ = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    velocity_ = std::clamp(velocity, 0.0f, 1.0f);

    const ChordPresetSpec& spec = getChordPreset(preset_);

    juce::ADSR::Parameters envParams;
    switch (spec.mode) {
        case ChordMode::Warm:
        case ChordMode::Velvet:
        case ChordMode::AirChoir:
        case ChordMode::DetuneCloud:
        case ChordMode::Dust:
            envParams.attack = 0.03f + (1.0f - tone_) * (0.07f + 0.05f * spec.body);
            envParams.decay = 0.32f + 0.32f * spec.body;
            envParams.sustain = 0.58f + 0.24f * spec.body;
            envParams.release = 0.5f + motion_ * (0.8f + 0.6f * spec.spread);
            filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            break;
        case ChordMode::Stab:
        case ChordMode::Organ:
        case ChordMode::MetalPluck:
        case ChordMode::PwmKeys:
            envParams.attack = 0.001f + (1.0f - spec.body) * 0.01f;
            envParams.decay = 0.1f + (1.0f - motion_) * (0.18f + 0.08f * spec.color);
            envParams.sustain = 0.18f + 0.32f * spec.body;
            envParams.release = 0.14f + 0.26f * spec.body;
            filter_.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            break;
        case ChordMode::Glass:
        case ChordMode::FmBloom:
        case ChordMode::Sweep:
            envParams.attack = 0.006f + (1.0f - tone_) * 0.02f;
            envParams.decay = 0.18f + 0.26f * (1.0f - spec.body);
            envParams.sustain = 0.46f + 0.14f * spec.body;
            envParams.release = 0.3f + motion_ * 0.55f;
            filter_.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            break;
    }
    env_.setParameters(envParams);

    osc1_.setFrequency(baseFreq_);
    osc2_.setFrequency(baseFreq_ * (1.001f + motion_ * (0.008f + 0.01f * spec.spread)));

    env_.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff) {
    if (allowTailOff) {
        env_.noteOff();
    } else {
        clearCurrentNote();
        env_.reset();
    }
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue) {}
void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue) {}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!env_.isActive()) {
        clearCurrentNote();
        currentNote_ = -1;
        return;
    }

    for (int i = 0; i < numSamples; ++i) {
        const float envVal = env_.getNextSample();
        const float lfo = slowLfo_.processSample(0.0f);

        float raw = 0.0f;
        float cutoff = 1000.0f;
        float resonance = 0.9f;

        const ChordPresetSpec& spec = getChordPreset(preset_);

        switch (spec.mode) {
            case ChordMode::Warm:
            case ChordMode::Velvet:
            case ChordMode::AirChoir:
            case ChordMode::DetuneCloud:
            case ChordMode::Dust: {
                const float detune = 0.002f + (0.002f + 0.011f * spec.spread) * (0.5f + 0.5f * lfo);
                osc2_.setFrequency(baseFreq_ * (1.0f + detune));

                const float saw = osc1_.processSample(0.0f);
                const float tri = osc2_.processSample(0.0f);
                raw = (0.46f + 0.2f * spec.body) * saw + (0.54f - 0.2f * spec.body) * tri;

                if (spec.mode == ChordMode::Dust) {
                    raw = DspHelpers::saturateTanh(raw + 0.15f * tri, 1.6f);
                }

                cutoff = 420.0f + tone_ * (1300.0f + 1600.0f * spec.color);
                cutoff += envVal * (700.0f + 600.0f * spec.body);
                cutoff += motion_ * (140.0f + 420.0f * (0.5f + 0.5f * lfo));
                resonance = 0.65f + motion_ * 0.28f + spec.color * 0.34f;
                break;
            }
            case ChordMode::Stab:
            case ChordMode::Organ:
            case ChordMode::MetalPluck:
            case ChordMode::PwmKeys: {
                osc1_.setFrequency(baseFreq_ * (1.0f + lfo * 0.0011f * motion_));
                osc2_.setFrequency(baseFreq_ * (1.5f + 0.55f * spec.color));

                const float bright = osc1_.processSample(0.0f);
                const float strike = osc2_.processSample(0.0f);
                raw = 0.58f * bright + 0.42f * strike;

                if (spec.mode == ChordMode::PwmKeys) {
                    const float threshold = tone_ * 1.5f - 0.75f;
                    const float pulse = bright > threshold ? 1.0f : -1.0f;
                    raw = 0.44f * bright + 0.36f * strike + 0.2f * pulse;
                }

                if (spec.mode == ChordMode::MetalPluck) {
                    raw = DspHelpers::saturateTanh(raw, 1.2f + 0.6f * tone_);
                }

                cutoff = 720.0f + tone_ * (2100.0f + 900.0f * spec.color) + envVal * 1300.0f;
                cutoff += motion_ * 320.0f * (0.5f + 0.5f * lfo);
                resonance = 0.82f + tone_ * 0.35f + 0.25f * spec.color;
                break;
            }
            case ChordMode::Glass:
            case ChordMode::FmBloom:
            case ChordMode::Sweep: {
                if (spec.mode == ChordMode::FmBloom) {
                    osc2_.setFrequency(baseFreq_ * (1.0f + tone_ * (1.5f + spec.spread)));
                    const float mod = osc2_.processSample(0.0f);
                    const float fmDepth = (0.35f + tone_ * (1.9f + spec.color)) * (0.35f + 0.65f * envVal);
                    osc1_.setFrequency(std::clamp(baseFreq_ + mod * fmDepth * baseFreq_, 20.0f, 14000.0f));
                } else {
                    const float shimmer = 1.0f + lfo * (0.003f + motion_ * (0.011f + 0.01f * spec.spread));
                    osc1_.setFrequency(baseFreq_ * shimmer);
                    osc2_.setFrequency(baseFreq_ * (1.45f + tone_ * (0.45f + 0.45f * spec.color)));
                }

                const float root = osc1_.processSample(0.0f);
                const float overtone = osc2_.processSample(0.0f);
                raw = 0.5f * root + 0.5f * overtone;

                if (spec.mode == ChordMode::Sweep) {
                    raw = DspHelpers::saturateTanh(
                        raw + 0.14f * std::sin(juce::MathConstants<float>::twoPi * envVal),
                        1.6f);
                }

                cutoff = 650.0f + tone_ * (2500.0f + 900.0f * spec.color);
                cutoff += motion_ * (280.0f + 980.0f * (0.5f + 0.5f * lfo));
                cutoff += envVal * 900.0f;
                resonance = 0.92f + motion_ * 0.5f + spec.color * 0.3f;
                break;
            }
        }

        filter_.setCutoffFrequency(std::clamp(cutoff, 120.0f, 8000.0f));
        filter_.setResonance(std::clamp(resonance, 0.5f, 2.4f));

        float val = filter_.processSample(0, raw) * envVal;
        val *= (0.22f + velocity_ * 0.12f);

        if (!env_.isActive()) {
            clearCurrentNote();
            currentNote_ = -1;
            break;
        }

        outputBuffer.addSample(0, startSample + i, val);
        if (outputBuffer.getNumChannels() > 1) {
            outputBuffer.addSample(1, startSample + i, val);
        }
    }
}

void SynthVoice::setTimbre(uint8_t preset, float tone, float motion) {
    preset_ = static_cast<uint8_t>(std::min<uint8_t>(preset, kMaxChordPresetId));
    tone_ = std::clamp(tone, 0.0f, 1.0f);
    motion_ = std::clamp(motion, 0.0f, 1.0f);
    const ChordPresetSpec& spec = getChordPreset(preset_);
    slowLfo_.setFrequency(0.12f + motion_ * (2.4f + 2.2f * spec.spread));
}


// ChordEngine Implementation
ChordEngine::ChordEngine() {
    for (int i = 0; i < 8; ++i) {
        synth_.addVoice(new SynthVoice());
    }
    synth_.addSound(new SynthSound());
}

void ChordEngine::setPreset(uint8_t preset) {
    preset_ = preset;
    sampler_.setPreset(static_cast<uint8_t>(preset >= kMelodicSamplerPresetStart
        ? preset - kMelodicSamplerPresetStart
        : preset));
}

void ChordEngine::setTone(float tone) {
    tone_ = std::clamp(tone, 0.0f, 1.0f);
    sampler_.setTone(tone_);
}

void ChordEngine::setMotion(float motion) {
    motion_ = std::clamp(motion, 0.0f, 1.0f);
    sampler_.setMotion(motion_);
}

void ChordEngine::setSampleRegion(uint8_t regionIndex, const MelodicSamplerEngine::Region& region) {
    sampler_.setRegion(regionIndex, region);
}

void ChordEngine::clearSampleRegions() {
    sampler_.clearRegions();
}

void ChordEngine::prepare(double sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    sampler_.prepare(sampleRate, blockSize);
    synth_.setCurrentPlaybackSampleRate(sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;
    
    for (int i = 0; i < synth_.getNumVoices(); ++i) {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth_.getVoice(i))) {
            voice->prepare(spec);
        }
    }
}

void ChordEngine::reset() {
    sampler_.reset();
    synth_.allNotesOff(0, false);
}

void ChordEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
                                  const juce::MidiBuffer& midi,
                                  int numSamples)
{
    if (preset_ >= kMelodicSamplerPresetStart && sampler_.hasAssignedSamples()) {
        sampler_.setPreset(static_cast<uint8_t>(preset_ - kMelodicSamplerPresetStart));
        sampler_.setTone(tone_);
        sampler_.setMotion(motion_);
        sampler_.renderNextBlock(buffer, midi, numSamples);
        return;
    }

    for (int i = 0; i < synth_.getNumVoices(); ++i) {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth_.getVoice(i))) {
            voice->setTimbre(preset_, tone_, motion_);
        }
    }

    synth_.renderNextBlock(buffer, midi, 0, numSamples);
}
