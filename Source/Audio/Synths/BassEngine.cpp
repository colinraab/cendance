#include "BassEngine.h"
#include "DspHelpers.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

enum class BassMode : uint8_t {
    Sub,
    Acid,
    Reese,
    Pluck,
    FmGrowl,
    Folded,
    Hollow
};

struct BassPresetSpec {
    BassMode mode;
    float color;
    float spread;
    float body;
    float output;
};

constexpr std::array<BassPresetSpec, 20> kBassPresetSpecs = {{
    {BassMode::Sub, 0.12f, 0.18f, 0.78f, 0.42f},
    {BassMode::Acid, 0.68f, 0.25f, 0.52f, 0.40f},
    {BassMode::Reese, 0.55f, 0.62f, 0.60f, 0.39f},
    {BassMode::Pluck, 0.48f, 0.50f, 0.58f, 0.37f},
    {BassMode::FmGrowl, 0.76f, 0.60f, 0.64f, 0.37f},
    {BassMode::Folded, 0.82f, 0.42f, 0.50f, 0.35f},
    {BassMode::Hollow, 0.40f, 0.34f, 0.42f, 0.36f},
    {BassMode::Sub, 0.30f, 0.34f, 0.62f, 0.41f},
    {BassMode::FmGrowl, 0.42f, 0.22f, 0.48f, 0.36f},
    {BassMode::Acid, 0.84f, 0.70f, 0.54f, 0.38f},
    {BassMode::Reese, 0.66f, 0.88f, 0.46f, 0.35f},
    {BassMode::Hollow, 0.22f, 0.56f, 0.38f, 0.34f},
    {BassMode::FmGrowl, 0.90f, 0.28f, 0.52f, 0.35f},
    {BassMode::Reese, 0.74f, 0.94f, 0.40f, 0.34f},
    {BassMode::Folded, 0.86f, 0.58f, 0.44f, 0.34f},
    {BassMode::Pluck, 0.60f, 0.68f, 0.70f, 0.35f},
    {BassMode::Sub, 0.18f, 0.42f, 0.86f, 0.40f},
    {BassMode::Acid, 0.92f, 0.82f, 0.48f, 0.36f},
    {BassMode::Hollow, 0.34f, 0.72f, 0.36f, 0.33f},
    {BassMode::Reese, 0.50f, 0.52f, 0.74f, 0.37f}
}};

constexpr uint8_t kMaxBassPresetId = static_cast<uint8_t>(kBassPresetSpecs.size() - 1);
constexpr uint8_t kMelodicSamplerPresetStart = 20;

} // namespace

BassEngine::BassEngine() {
    oscMain_.initialise([](float x) {
        return (x / juce::MathConstants<float>::pi) - 1.0f;
    });
    oscSub_.initialise([](float x) { return std::sin(x); });
    oscDetune_.initialise([](float x) {
        return (x / juce::MathConstants<float>::pi) - 1.0f;
    });
    oscFmCarrier_.initialise([](float x) { return std::sin(x); });
    oscFmMod_.initialise([](float x) { return std::sin(x); });
    motionLfo_.initialise([](float x) { return std::sin(x); });
    
    juce::ADSR::Parameters ampEnv;
    ampEnv.attack = 0.005f;
    ampEnv.decay = 0.12f;
    ampEnv.sustain = 0.7f;
    ampEnv.release = 0.16f;
    env_.setParameters(ampEnv);

    juce::ADSR::Parameters modEnv;
    modEnv.attack = 0.002f;
    modEnv.decay = 0.22f;
    modEnv.sustain = 0.0f;
    modEnv.release = 0.15f;
    filterEnv_.setParameters(modEnv);
    
    filter_.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void BassEngine::setPreset(uint8_t preset) {
    preset_ = preset;
    sampler_.setPreset(static_cast<uint8_t>(preset >= kMelodicSamplerPresetStart
        ? preset - kMelodicSamplerPresetStart
        : preset));
}

void BassEngine::setTone(float tone) {
    tone_ = std::clamp(tone, 0.0f, 1.0f);
    sampler_.setTone(tone_);
}

void BassEngine::setMotion(float motion) {
    motion_ = std::clamp(motion, 0.0f, 1.0f);
    sampler_.setMotion(motion_);
}

void BassEngine::setSampleRegion(uint8_t regionIndex, const MelodicSamplerEngine::Region& region) {
    sampler_.setRegion(regionIndex, region);
}

void BassEngine::clearSampleRegions() {
    sampler_.clearRegions();
}

void BassEngine::prepare(double sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    sampler_.prepare(sampleRate, blockSize);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;
    
    oscMain_.prepare(spec);
    oscSub_.prepare(spec);
    oscDetune_.prepare(spec);
    oscFmCarrier_.prepare(spec);
    oscFmMod_.prepare(spec);
    motionLfo_.prepare(spec);

    env_.setSampleRate(sampleRate);
    filterEnv_.setSampleRate(sampleRate);
    
    filter_.prepare(spec);
    filter_.setCutoffFrequency(700.0f);
    filter_.setResonance(1.0f);
    
    motionLfo_.setFrequency(0.08f);

    portamentoRate_ = std::exp(-1.0f / (0.01f * sampleRate));
}

void BassEngine::reset() {
    sampler_.reset();
    oscMain_.reset();
    oscSub_.reset();
    oscDetune_.reset();
    oscFmCarrier_.reset();
    oscFmMod_.reset();
    motionLfo_.reset();
    env_.reset();
    filterEnv_.reset();
    filter_.reset();
    phaseDrift_ = 0.0f;
    currentNote_ = -1;
    currentFreq_ = 0.0f;
    targetFreq_ = 0.0f;
}

void BassEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
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

    const float glideSeconds = 0.006f + (motion_ * 0.08f);
    portamentoRate_ = std::exp(-1.0f / (glideSeconds * static_cast<float>(sampleRate_)));
    motionLfo_.setFrequency(0.08f + motion_ * 5.5f);

    auto renderOneSample = [&]() {
        if (!env_.isActive()) {
            return 0.0f;
        }

        const BassPresetSpec& spec = kBassPresetSpecs[std::min<uint8_t>(preset_, kMaxBassPresetId)];
        currentFreq_ = currentFreq_ * portamentoRate_ + targetFreq_ * (1.0f - portamentoRate_);

        const float lfo = motionLfo_.processSample(0.0f);
        const float modEnv = filterEnv_.getNextSample();
        const float ampEnv = env_.getNextSample();

        float oscMix = 0.0f;
        float cutoff = 400.0f;
        float resonance = 1.0f;

        switch (spec.mode) {
            case BassMode::Sub: {
                oscMain_.setFrequency(currentFreq_ * (1.0f + 0.0008f * spec.spread));
                oscSub_.setFrequency(currentFreq_ * 0.5f);
                oscDetune_.setFrequency(currentFreq_ * (1.001f + motion_ * 0.004f * spec.spread));

                const float fundamental = oscSub_.processSample(0.0f);
                const float edge = oscMain_.processSample(0.0f);
                const float shimmer = oscDetune_.processSample(0.0f);
                oscMix = (0.58f + 0.18f * spec.body) * fundamental
                       + (0.26f - 0.06f * spec.body) * edge
                       + (0.16f - 0.12f * spec.body) * shimmer;

                cutoff = 100.0f + tone_ * (900.0f + spec.color * 520.0f);
                cutoff += modEnv * (250.0f + currentVelocity_ * 260.0f + spec.color * 180.0f);
                cutoff += lfo * motion_ * (110.0f + 120.0f * spec.spread);
                resonance = 0.68f + motion_ * 0.38f + spec.color * 0.28f;
                break;
            }
            case BassMode::Acid: {
                oscMain_.setFrequency(currentFreq_);
                oscDetune_.setFrequency(currentFreq_ * (1.002f + motion_ * (0.004f + 0.008f * spec.spread)));
                oscSub_.setFrequency(currentFreq_ * (0.98f + 0.03f * spec.body));

                const float sawA = oscMain_.processSample(0.0f);
                const float sawB = oscDetune_.processSample(0.0f);
                const float body = oscSub_.processSample(0.0f);
                oscMix = 0.5f * sawA + (0.26f + 0.1f * spec.spread) * sawB + (0.24f - 0.1f * spec.spread) * body;

                cutoff = 140.0f + tone_ * (2500.0f + spec.color * 700.0f);
                cutoff += modEnv * (650.0f + motion_ * 2600.0f + spec.color * 600.0f);
                cutoff += lfo * (90.0f + motion_ * (340.0f + spec.spread * 230.0f));
                resonance = 1.0f + tone_ * 0.55f + motion_ * 0.65f + spec.color * 0.35f;
                break;
            }
            case BassMode::Reese: {
                const float detune = 0.0032f + motion_ * (0.014f + 0.01f * spec.spread);
                phaseDrift_ += 0.00005f + motion_ * (0.0002f + 0.00008f * spec.color);
                if (phaseDrift_ > juce::MathConstants<float>::twoPi) {
                    phaseDrift_ -= juce::MathConstants<float>::twoPi;
                }
                const float drift = std::sin(phaseDrift_) * (0.002f + motion_ * (0.009f + 0.004f * spec.spread));

                oscMain_.setFrequency(currentFreq_ * (1.0f - detune + drift));
                oscDetune_.setFrequency(currentFreq_ * (1.0f + detune - drift));
                oscSub_.setFrequency(currentFreq_ * (0.48f + 0.06f * spec.body));

                const float sawA = oscMain_.processSample(0.0f);
                const float sawB = oscDetune_.processSample(0.0f);
                const float sub = oscSub_.processSample(0.0f);
                oscMix = (0.36f + 0.08f * spec.body) * sawA
                       + (0.36f + 0.08f * spec.body) * sawB
                       + (0.28f - 0.16f * spec.body) * sub;

                cutoff = 220.0f + tone_ * (1750.0f + spec.color * 520.0f);
                cutoff += (0.5f + 0.5f * lfo) * (210.0f + motion_ * (1200.0f + 450.0f * spec.spread));
                resonance = 0.95f + motion_ * 0.52f + spec.color * 0.28f;
                break;
            }
            case BassMode::Pluck: {
                oscMain_.setFrequency(currentFreq_);
                oscDetune_.setFrequency(currentFreq_ * (1.96f + tone_ * (0.45f + 0.35f * spec.spread)));
                oscSub_.setFrequency(currentFreq_ * 0.5f);

                const float transient = std::clamp(modEnv * (1.15f + spec.color), 0.0f, 1.5f);
                const float root = oscMain_.processSample(0.0f);
                const float strike = oscDetune_.processSample(0.0f);
                const float sub = oscSub_.processSample(0.0f);
                oscMix = (0.46f + 0.1f * spec.body) * root
                       + (0.42f - 0.08f * spec.body) * strike * transient
                       + (0.12f - 0.04f * spec.body) * sub;

                cutoff = 260.0f + tone_ * (2200.0f + spec.color * 1400.0f);
                cutoff += transient * (3800.0f + motion_ * 2200.0f);
                cutoff += lfo * motion_ * (70.0f + 170.0f * spec.spread);
                resonance = 1.0f + tone_ * 0.4f + spec.color * 0.55f;
                break;
            }
            case BassMode::FmGrowl: {
                oscFmMod_.setFrequency(currentFreq_ * (1.0f + tone_ * (0.95f + 0.55f * spec.spread)));
                const float mod = oscFmMod_.processSample(0.0f);
                const float fmDepthOctaves = (0.08f + tone_ * (0.35f + 0.25f * spec.color))
                    * (0.35f + 0.65f * modEnv);
                const float fmFrequency = currentFreq_ * std::exp2(mod * fmDepthOctaves);

                oscFmCarrier_.setFrequency(std::clamp(fmFrequency, 20.0f, 15000.0f));
                oscMain_.setFrequency(currentFreq_ * (1.0f + lfo * motion_ * 0.003f * spec.spread));
                oscSub_.setFrequency(currentFreq_ * (0.45f + 0.1f * spec.body));

                const float carrier = oscFmCarrier_.processSample(0.0f);
                const float edge = oscMain_.processSample(0.0f);
                const float sub = oscSub_.processSample(0.0f);
                const float edgeAmount = 0.08f + tone_ * (0.16f + 0.08f * spec.color);
                oscMix = (0.58f + 0.12f * spec.body) * carrier
                       + (0.34f - 0.1f * spec.body) * sub
                       + edgeAmount * edge;
                oscMix = DspHelpers::saturateTanh(oscMix, 1.05f + tone_ * 0.55f);

                cutoff = 180.0f + tone_ * (1800.0f + spec.color * 1200.0f);
                cutoff += modEnv * (550.0f + motion_ * 1900.0f);
                cutoff += lfo * (80.0f + motion_ * (300.0f + 320.0f * spec.spread));
                resonance = 0.88f + motion_ * 0.44f + spec.color * 0.45f;
                break;
            }
            case BassMode::Folded: {
                oscMain_.setFrequency(currentFreq_ * (1.0f - 0.0007f * spec.spread));
                oscDetune_.setFrequency(currentFreq_ * (1.0f + 0.002f + motion_ * 0.007f));
                oscSub_.setFrequency(currentFreq_ * 0.5f);

                const float sawA = oscMain_.processSample(0.0f);
                const float sawB = oscDetune_.processSample(0.0f);
                const float sub = oscSub_.processSample(0.0f);
                float folded = 0.48f * sawA + 0.32f * sawB + 0.2f * sub;
                folded = DspHelpers::foldWithAmount(folded, 0.2f + tone_ * (0.72f + 0.2f * spec.color), 4.0f);
                oscMix = folded;

                cutoff = 150.0f + tone_ * (2100.0f + spec.color * 900.0f);
                cutoff += modEnv * (500.0f + motion_ * 1600.0f);
                cutoff += lfo * motion_ * (90.0f + 260.0f * spec.spread);
                resonance = 0.82f + motion_ * 0.35f + spec.color * 0.52f;
                break;
            }
            case BassMode::Hollow: {
                oscMain_.setFrequency(currentFreq_);
                oscDetune_.setFrequency(currentFreq_ * (1.45f + tone_ * (0.35f + 0.35f * spec.spread)));
                oscSub_.setFrequency(currentFreq_ * (1.0f + 0.002f * spec.body));

                const float root = oscMain_.processSample(0.0f);
                const float overtone = oscDetune_.processSample(0.0f);
                const float inverted = oscSub_.processSample(0.0f);
                oscMix = 0.52f * root + 0.34f * overtone - (0.14f + 0.1f * spec.body) * inverted;

                cutoff = 300.0f + tone_ * (2600.0f + 900.0f * spec.color);
                cutoff += modEnv * (320.0f + 820.0f * motion_);
                cutoff += lfo * motion_ * (130.0f + 280.0f * spec.spread);
                resonance = 1.1f + tone_ * 0.36f + spec.color * 0.42f;
                break;
            }
        }

        filter_.setCutoffFrequency(std::clamp(cutoff, 80.0f, 8000.0f));
        filter_.setResonance(std::clamp(resonance, 0.5f, 2.8f));

        const float filtered = filter_.processSample(0, oscMix);
        return filtered * ampEnv * spec.output;
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
            currentVelocity_ = std::clamp(msg.getFloatVelocity(), 0.0f, 1.0f);
            env_.noteOn();
            filterEnv_.noteOn();
        } else if (msg.isNoteOff() && msg.getNoteNumber() == currentNote_) {
            env_.noteOff();
            filterEnv_.noteOff();
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
