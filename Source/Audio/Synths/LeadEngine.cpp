#include "LeadEngine.h"
#include "DspHelpers.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

enum class LeadMode : uint8_t {
    Fm,
    Pwm,
    Glide,
    Supersaw,
    Voxel,
    AcidCry,
    Folded,
    Whistle,
    Reed,
    Reso,
    Octave,
    Mutant
};

struct LeadPresetSpec {
    LeadMode mode;
    float color;
    float spread;
    float body;
    float output;
};

constexpr std::array<LeadPresetSpec, 20> kLeadPresetSpecs = {{
    {LeadMode::Fm, 0.26f, 0.20f, 0.70f, 0.32f},
    {LeadMode::Pwm, 0.52f, 0.26f, 0.58f, 0.31f},
    {LeadMode::Glide, 0.38f, 0.62f, 0.66f, 0.31f},
    {LeadMode::Supersaw, 0.44f, 0.90f, 0.56f, 0.28f},
    {LeadMode::Voxel, 0.64f, 0.42f, 0.50f, 0.30f},
    {LeadMode::AcidCry, 0.88f, 0.36f, 0.46f, 0.30f},
    {LeadMode::Folded, 0.82f, 0.24f, 0.48f, 0.29f},
    {LeadMode::Whistle, 0.20f, 0.52f, 0.40f, 0.29f},
    {LeadMode::Reed, 0.56f, 0.30f, 0.62f, 0.30f},
    {LeadMode::Reso, 0.60f, 0.72f, 0.52f, 0.29f},
    {LeadMode::Octave, 0.48f, 0.44f, 0.58f, 0.31f},
    {LeadMode::Mutant, 0.74f, 0.66f, 0.50f, 0.28f},
    {LeadMode::Supersaw, 0.52f, 0.98f, 0.54f, 0.27f},
    {LeadMode::Reso, 0.66f, 0.82f, 0.50f, 0.28f},
    {LeadMode::Folded, 0.86f, 0.38f, 0.46f, 0.28f},
    {LeadMode::Mutant, 0.80f, 0.72f, 0.48f, 0.27f},
    {LeadMode::Whistle, 0.24f, 0.64f, 0.38f, 0.28f},
    {LeadMode::Fm, 0.34f, 0.32f, 0.64f, 0.31f},
    {LeadMode::Reed, 0.62f, 0.40f, 0.66f, 0.29f},
    {LeadMode::AcidCry, 0.92f, 0.44f, 0.44f, 0.29f}
}};

constexpr uint8_t kMaxLeadPresetId = static_cast<uint8_t>(kLeadPresetSpecs.size() - 1);
constexpr uint8_t kMelodicSamplerPresetStart = 20;

} // namespace

LeadEngine::LeadEngine() {
    carrierOsc_.initialise([](float x) { return std::sin(x); });
    modulatorOsc_.initialise([](float x) { return std::sin(x); });
    auxOsc_.initialise([](float x) {
        return (x / juce::MathConstants<float>::pi) - 1.0f;
    });
    motionLfo_.initialise([](float x) { return std::sin(x); });
    
    juce::ADSR::Parameters envParams;
    envParams.attack = 0.02f;
    envParams.decay = 0.18f;
    envParams.sustain = 0.58f;
    envParams.release = 0.3f;
    env_.setParameters(envParams);
    
    juce::ADSR::Parameters modEnvParams;
    modEnvParams.attack = 0.01f;
    modEnvParams.decay = 0.24f;
    modEnvParams.sustain = 0.1f;
    modEnvParams.release = 0.18f;
    modEnv_.setParameters(modEnvParams);

    filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void LeadEngine::setPreset(uint8_t preset) {
    preset_ = preset;
    sampler_.setPreset(static_cast<uint8_t>(preset >= kMelodicSamplerPresetStart
        ? preset - kMelodicSamplerPresetStart
        : preset));
}

void LeadEngine::setTone(float tone) {
    tone_ = std::clamp(tone, 0.0f, 1.0f);
    sampler_.setTone(tone_);
}

void LeadEngine::setMotion(float motion) {
    motion_ = std::clamp(motion, 0.0f, 1.0f);
    sampler_.setMotion(motion_);
}

void LeadEngine::setSampleRegion(uint8_t regionIndex, const MelodicSamplerEngine::Region& region) {
    sampler_.setRegion(regionIndex, region);
}

void LeadEngine::clearSampleRegions() {
    sampler_.clearRegions();
}

void LeadEngine::prepare(double sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    sampler_.prepare(sampleRate, blockSize);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;
    
    carrierOsc_.prepare(spec);
    modulatorOsc_.prepare(spec);
    auxOsc_.prepare(spec);
    motionLfo_.prepare(spec);
    filter_.prepare(spec);
    
    env_.setSampleRate(sampleRate);
    modEnv_.setSampleRate(sampleRate);

    motionLfo_.setFrequency(2.0f);
    filter_.setCutoffFrequency(2200.0f);
    filter_.setResonance(0.8f);
}

void LeadEngine::reset() {
    sampler_.reset();
    carrierOsc_.reset();
    modulatorOsc_.reset();
    auxOsc_.reset();
    motionLfo_.reset();
    filter_.reset();
    env_.reset();
    modEnv_.reset();
    currentNote_ = -1;
    currentFreq_ = 0.0f;
    targetFreq_ = 0.0f;
}

void LeadEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
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

    buffer.clear();

    const float glideSeconds = 0.004f + (motion_ * 0.09f);
    glideRate_ = std::exp(-1.0f / (glideSeconds * static_cast<float>(sampleRate_)));
    motionLfo_.setFrequency(1.5f + motion_ * 8.5f);

    auto renderOneSample = [&]() {
        if (!env_.isActive()) {
            return 0.0f;
        }

        const LeadPresetSpec& spec = kLeadPresetSpecs[std::min<uint8_t>(preset_, kMaxLeadPresetId)];
        currentFreq_ = currentFreq_ * glideRate_ + targetFreq_ * (1.0f - glideRate_);
        const float lfo = motionLfo_.processSample(0.0f);
        const float amp = env_.getNextSample();
        const float modShape = modEnv_.getNextSample();

        float raw = 0.0f;
        float cutoff = 800.0f;
        float resonance = 0.8f;

        switch (spec.mode) {
            case LeadMode::Fm: {
                modulatorOsc_.setFrequency(currentFreq_ * (1.4f + tone_ * (2.0f + spec.color * 1.2f)));
                const float modulator = modulatorOsc_.processSample(0.0f);
                const float fmDepth = (0.7f + tone_ * (4.2f + spec.color * 1.8f))
                    * (0.35f + 0.65f * modShape)
                    * (0.5f + 0.5f * velocity_);
                carrierOsc_.setFrequency(std::clamp(currentFreq_ + modulator * fmDepth * currentFreq_, 20.0f, 14000.0f));
                auxOsc_.setFrequency(currentFreq_ * (1.9f + 0.22f * spec.spread));

                const float sine = carrierOsc_.processSample(0.0f);
                const float saw = auxOsc_.processSample(0.0f);
                raw = (0.7f + 0.12f * spec.body) * sine + (0.3f - 0.12f * spec.body) * saw;

                filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                cutoff = 430.0f + tone_ * (2900.0f + 500.0f * spec.color);
                cutoff += motion_ * (250.0f + 1100.0f * (0.5f + 0.5f * lfo));
                resonance = 0.8f + tone_ * 0.35f + motion_ * 0.3f + 0.2f * spec.color;
                break;
            }
            case LeadMode::Pwm:
            case LeadMode::Voxel:
            case LeadMode::Reed: {
                carrierOsc_.setFrequency(currentFreq_ * (1.0f + lfo * 0.0026f * motion_));
                auxOsc_.setFrequency(currentFreq_ * (1.001f + motion_ * (0.005f + 0.008f * spec.spread)));

                const float sine = carrierOsc_.processSample(0.0f);
                const float saw = auxOsc_.processSample(0.0f);
                const float threshold = (tone_ * (1.5f + 0.2f * spec.color)) - 0.75f;
                const float pulse = sine > threshold ? 1.0f : -1.0f;
                raw = 0.28f * sine + 0.38f * saw + 0.34f * pulse;

                if (spec.mode == LeadMode::Voxel) {
                    modulatorOsc_.setFrequency(currentFreq_ * (2.0f + 0.8f * tone_));
                    raw = 0.65f * raw + 0.35f * modulatorOsc_.processSample(0.0f);
                }
                if (spec.mode == LeadMode::Reed) {
                    raw = std::tanh(raw * (1.35f + 0.85f * tone_));
                }

                filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                cutoff = 280.0f + tone_ * (2200.0f + 900.0f * spec.color) + amp * 1100.0f;
                cutoff += motion_ * (120.0f + 650.0f * (0.5f + 0.5f * lfo));
                resonance = 0.62f + motion_ * 0.3f + 0.22f * spec.color;
                break;
            }
            case LeadMode::Glide:
            case LeadMode::Whistle:
            case LeadMode::Octave: {
                const float glideDetune = 0.002f + motion_ * (0.009f + 0.008f * spec.spread);
                carrierOsc_.setFrequency(currentFreq_ * (1.0f - glideDetune));
                auxOsc_.setFrequency(currentFreq_ * (1.0f + glideDetune));
                modulatorOsc_.setFrequency(currentFreq_ * (0.5f + tone_ * (0.6f + 0.35f * spec.color)));

                const float sawA = auxOsc_.processSample(0.0f);
                const float sine = carrierOsc_.processSample(0.0f);
                float warmth = modulatorOsc_.processSample(0.0f) * 0.2f;

                if (spec.mode == LeadMode::Whistle) {
                    warmth *= 0.35f;
                    raw = 0.74f * sine + 0.26f * sawA + warmth;
                } else if (spec.mode == LeadMode::Octave) {
                    auxOsc_.setFrequency(currentFreq_ * 2.0f);
                    raw = 0.45f * sine + 0.45f * auxOsc_.processSample(0.0f) + 0.1f * warmth;
                } else {
                    raw = 0.56f * sawA + 0.34f * sine + warmth;
                }

                filter_.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
                cutoff = 560.0f + tone_ * (2200.0f + 700.0f * spec.color) + amp * 950.0f;
                cutoff += motion_ * (140.0f + 780.0f * (0.5f + 0.5f * lfo));
                resonance = 0.85f + tone_ * 0.28f + motion_ * 0.32f;
                break;
            }
            case LeadMode::Supersaw:
            case LeadMode::AcidCry:
            case LeadMode::Folded:
            case LeadMode::Reso:
            case LeadMode::Mutant: {
                const float width = 0.0025f + motion_ * (0.01f + 0.01f * spec.spread);
                carrierOsc_.setFrequency(currentFreq_ * (1.0f - width));
                auxOsc_.setFrequency(currentFreq_ * (1.0f + width));
                modulatorOsc_.setFrequency(currentFreq_ * (1.0f + tone_ * (0.9f + 0.8f * spec.color)));

                const float sawA = carrierOsc_.processSample(0.0f);
                const float sawB = auxOsc_.processSample(0.0f);
                const float texture = modulatorOsc_.processSample(0.0f);
                raw = 0.42f * sawA + 0.42f * sawB + 0.16f * texture;

                if (spec.mode == LeadMode::Folded || spec.mode == LeadMode::Mutant) {
                    raw = DspHelpers::foldWithAmount(raw, 0.22f + tone_ * (0.75f + 0.2f * spec.color), 5.0f);
                }
                if (spec.mode == LeadMode::AcidCry) {
                    raw = std::tanh(raw * (1.3f + 1.1f * tone_));
                }
                if (spec.mode == LeadMode::Reso) {
                    raw = 0.72f * raw + 0.28f * std::sin(raw * juce::MathConstants<float>::pi);
                }

                filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                cutoff = 380.0f + tone_ * (2800.0f + 800.0f * spec.color) + modShape * 900.0f;
                cutoff += motion_ * (210.0f + 950.0f * (0.5f + 0.5f * lfo));
                resonance = 0.74f + tone_ * 0.4f + motion_ * 0.4f;
                break;
            }
        }

        filter_.setCutoffFrequency(std::clamp(cutoff, 120.0f, 9000.0f));
        filter_.setResonance(std::clamp(resonance, 0.5f, 2.2f));
        return filter_.processSample(0, raw) * amp * spec.output;
    };
    
    int samplePos = 0;
    for (const auto meta : midi) {
        int eventTime = meta.samplePosition;
        
        while (samplePos < eventTime && samplePos < numSamples) {
            if (env_.isActive()) {
                const float val = renderOneSample();
                buffer.addSample(0, samplePos, val);
                if (buffer.getNumChannels() > 1) {
                    buffer.addSample(1, samplePos, val);
                }
            }
            samplePos++;
        }
        
        auto msg = meta.getMessage();
        if (msg.isNoteOn()) {
            targetFreq_ = juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber());
            if (!env_.isActive() || currentNote_ == -1) {
                currentFreq_ = targetFreq_;
            }
            currentNote_ = msg.getNoteNumber();
            velocity_ = std::clamp(msg.getFloatVelocity(), 0.0f, 1.0f);
            
            env_.noteOn();
            modEnv_.noteOn();
        } else if (msg.isNoteOff() && msg.getNoteNumber() == currentNote_) {
            env_.noteOff();
            modEnv_.noteOff();
            currentNote_ = -1;
        }
    }
    
    while (samplePos < numSamples) {
        if (env_.isActive()) {
            const float val = renderOneSample();
            buffer.addSample(0, samplePos, val);
            if (buffer.getNumChannels() > 1) {
                buffer.addSample(1, samplePos, val);
            }
        }
        samplePos++;
    }
}
