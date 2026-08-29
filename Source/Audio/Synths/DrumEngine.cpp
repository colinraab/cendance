#include "DrumEngine.h"

#include "../../App/DrumSampleCatalog.h"

#include <algorithm>
#include <cmath>

DrumEngine::DrumEngine() {
    kickOsc_.initialise([](float x) { return std::sin(x); });
    snareOsc_.initialise([](float x) { return std::sin(x); });
    
    juce::ADSR::Parameters envParams;
    envParams.attack = 0.001f;
    envParams.decay = 0.2f;
    envParams.sustain = 0.0f;
    envParams.release = 0.1f;
    
    kickEnv_.setParameters(envParams);
    
    envParams.decay = 0.15f;
    snareEnv_.setParameters(envParams);
    
    hatFilter_.setType(juce::dsp::StateVariableTPTFilterType::highpass);
}

void DrumEngine::prepare(double sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;
    
    kickOsc_.prepare(spec);
    kickEnv_.setSampleRate(sampleRate);
    
    snareOsc_.prepare(spec);
    snareEnv_.setSampleRate(sampleRate);
    
    hatFilter_.prepare(spec);
    hatFilter_.setCutoffFrequency(8000.0f);
    
    // Convert decay times (approximate 60dB drop over time in ms)
    kickPitchEnvDecay_ = std::exp(-1.0f / (0.05f * sampleRate)); // 50ms decay
    snareNoiseDecay_ = std::exp(-1.0f / (0.1f * sampleRate));    // 100ms decay
    closedHatDecay_ = std::exp(-1.0f / (0.05f * sampleRate));    // 50ms decay
    openHatDecay_ = std::exp(-1.0f / (0.3f * sampleRate));       // 300ms decay
}

void DrumEngine::reset() {
    kickOsc_.reset();
    kickEnv_.reset();
    kickPitchEnv_ = 0.0f;
    
    snareOsc_.reset();
    snareEnv_.reset();
    snareNoiseEnv_ = 0.0f;
    
    hatFilter_.reset();
    closedHatEnv_ = 0.0f;
    openHatEnv_ = 0.0f;

    for (auto& voice : sampleVoices_) {
        voice.active = false;
        voice.sampleData = nullptr;
        voice.samplePosition = 0.0;
        voice.increment = 1.0;
        voice.gain = 1.0f;
        voice.envelope = 1.0f;
        voice.decayMul = 0.9995f;
        voice.attack = 1.0f;
        voice.attackStep = 1.0f;
        voice.slotIndex = 0;
    }

    tiltLowStateL_ = 0.0f;
    tiltLowStateR_ = 0.0f;
}

void DrumEngine::setSampleForSlot(uint8_t slotIndex, const DrumSampleData* sampleData) {
    if (slotIndex < SampleSlotCount) {
        slotSamples_[slotIndex] = sampleData;
    }
}

void DrumEngine::setSampleSlotVolume(uint8_t slotIndex, float value) {
    if (slotIndex < SampleSlotCount) {
        slotParams_[slotIndex].volume = value;
    }
}

void DrumEngine::setSampleSlotTuneSemitones(uint8_t slotIndex, float value) {
    if (slotIndex < SampleSlotCount) {
        slotParams_[slotIndex].tuneSemitones = value;
    }
}

void DrumEngine::setSampleSlotStartOffset(uint8_t slotIndex, float value) {
    if (slotIndex < SampleSlotCount) {
        slotParams_[slotIndex].startOffset = value;
    }
}

void DrumEngine::setSampleSlotDecay(uint8_t slotIndex, float value) {
    if (slotIndex < SampleSlotCount) {
        slotParams_[slotIndex].decay = value;
    }
}

void DrumEngine::setSampleSlotVelocitySensitivity(uint8_t slotIndex, float value) {
    if (slotIndex < SampleSlotCount) {
        slotParams_[slotIndex].velocitySensitivity = value;
    }
}

void DrumEngine::setTone(float tone) {
    tone_ = std::clamp(tone, 0.0f, 1.0f);
}

void DrumEngine::setMotion(float motion) {
    motion_ = std::clamp(motion, 0.0f, 1.0f);
}

void DrumEngine::refreshMoveEnvelopeState() {
    const float move = std::clamp(motion_, 0.0f, 1.0f);

    juce::ADSR::Parameters kickParams;
    kickParams.attack = 0.001f + move * 0.012f;
    kickParams.decay = 0.08f + move * 0.26f;
    kickParams.sustain = 0.0f;
    kickParams.release = 0.03f + move * 0.18f;
    kickEnv_.setParameters(kickParams);

    juce::ADSR::Parameters snareParams;
    snareParams.attack = 0.001f + move * 0.014f;
    snareParams.decay = 0.1f + move * 0.3f;
    snareParams.sustain = 0.0f;
    snareParams.release = 0.04f + move * 0.2f;
    snareEnv_.setParameters(snareParams);

    kickPitchEnvDecay_ = std::exp(-1.0f / (std::max(0.005f, 0.02f + move * 0.16f) * static_cast<float>(sampleRate_)));
    snareNoiseDecay_ = std::exp(-1.0f / (std::max(0.005f, 0.05f + move * 0.3f) * static_cast<float>(sampleRate_)));
    closedHatDecay_ = std::exp(-1.0f / (std::max(0.005f, 0.02f + move * 0.16f) * static_cast<float>(sampleRate_)));
    openHatDecay_ = std::exp(-1.0f / (std::max(0.005f, 0.08f + move * 0.8f) * static_cast<float>(sampleRate_)));
}

void DrumEngine::refreshToneTiltState() {
    const float centered = (std::clamp(tone_, 0.0f, 1.0f) - 0.5f) * 2.0f;
    const float tiltDb = centered * 6.0f;
    const float highGain = std::pow(10.0f, tiltDb / 20.0f);
    const float lowGain = std::pow(10.0f, -tiltDb / 20.0f);

    const float gainNorm = 1.0f / std::sqrt(std::max(1.0e-5f, 0.5f * (highGain * highGain + lowGain * lowGain)));
    tiltHighGain_ = highGain * gainNorm;
    tiltLowGain_ = lowGain * gainNorm;

    const float pivotHz = 1100.0f;
    const float twoPi = 2.0f * juce::MathConstants<float>::pi;
    tiltCoeff_ = 1.0f - std::exp(-(twoPi * pivotHz) / static_cast<float>(sampleRate_));
    tiltCoeff_ = std::clamp(tiltCoeff_, 0.0f, 1.0f);
}

void DrumEngine::applyToneTilt(float& leftOut, float& rightOut) {
    tiltLowStateL_ += tiltCoeff_ * (leftOut - tiltLowStateL_);
    tiltLowStateR_ += tiltCoeff_ * (rightOut - tiltLowStateR_);

    const float highL = leftOut - tiltLowStateL_;
    const float highR = rightOut - tiltLowStateR_;

    leftOut = (tiltLowStateL_ * tiltLowGain_) + (highL * tiltHighGain_);
    rightOut = (tiltLowStateR_ * tiltLowGain_) + (highR * tiltHighGain_);
}

void DrumEngine::chokeOpenHatVoices() {
    const uint8_t openHatSlot = static_cast<uint8_t>(DrumSampleCatalog::SlotId::OpenHat);
    for (auto& voice : sampleVoices_) {
        if (voice.active && voice.slotIndex == openHatSlot) {
            voice.active = false;
        }
    }
}

void DrumEngine::triggerSynthVoice(int midiNote, float velocity) {
    if (midiNote == 36) { // Kick
        kickEnv_.noteOn();
        kickPitchEnv_ = 1.0f * velocity;
    } else if (midiNote == 38) { // Snare
        snareEnv_.noteOn();
        snareOsc_.setFrequency(180.0f);
        snareNoiseEnv_ = 1.0f * velocity;
    } else if (midiNote == 42) { // Closed Hat
        closedHatEnv_ = 1.0f * velocity;
        openHatEnv_ = 0.0f;
    } else if (midiNote == 46) { // Open Hat
        openHatEnv_ = 1.0f * velocity;
        closedHatEnv_ = 0.0f;
    }
}

void DrumEngine::triggerSampleVoice(uint8_t slotIndex, float velocity) {
    if (slotIndex >= SampleSlotCount) {
        return;
    }

    const DrumSampleData* sample = slotSamples_[slotIndex];
    if (sample == nullptr || sample->audio.getNumSamples() <= 1 || sample->audio.getNumChannels() <= 0) {
        return;
    }

    if (DrumSampleCatalog::getSlotDefinition(slotIndex).chokesOpenHat) {
        chokeOpenHatVoices();
    }

    SampleVoice* targetVoice = nullptr;
    for (auto& voice : sampleVoices_) {
        if (!voice.active) {
            targetVoice = &voice;
            break;
        }
    }

    if (targetVoice == nullptr) {
        targetVoice = &sampleVoices_[0];
    }

    const auto& params = slotParams_[slotIndex];
    const int sampleLength = sample->audio.getNumSamples();
    const int startSample = std::clamp(static_cast<int>(params.startOffset * static_cast<float>(sampleLength - 1)),
                                       0,
                                       sampleLength - 1);

    const double pitchRatio = std::pow(2.0, static_cast<double>(params.tuneSemitones) / 12.0);
    const double sourceRateRatio = sampleRate_ > 0.0 ? (sample->sourceSampleRate / sampleRate_) : 1.0;
    const double increment = std::max(0.01, pitchRatio * sourceRateRatio);

    const float velocityAmount = std::clamp(params.velocitySensitivity, 0.0f, 1.0f);
    const float velBlend = (1.0f - velocityAmount) + (velocityAmount * std::clamp(velocity, 0.0f, 1.0f));

    const float move = std::clamp(motion_, 0.0f, 1.0f);
    const float decayScale = 0.45f + (move * 1.3f);
    const float decaySeconds = (0.02f + std::clamp(params.decay, 0.0f, 1.0f) * 2.0f) * decayScale;
    const float decayMul = std::exp(-1.0f / std::max(1.0f, static_cast<float>(sampleRate_) * decaySeconds));
    const int attackSamples = std::max(1,
                                       static_cast<int>(sampleRate_ * (0.0005 + static_cast<double>(move) * 0.01)));

    targetVoice->sampleData = sample;
    targetVoice->samplePosition = static_cast<double>(startSample);
    targetVoice->increment = increment;
    targetVoice->gain = std::clamp(params.volume, 0.0f, 2.0f) * velBlend;
    targetVoice->envelope = 1.0f;
    targetVoice->decayMul = std::clamp(decayMul, 0.0f, 1.0f);
    targetVoice->attack = 0.0f;
    targetVoice->attackStep = 1.0f / static_cast<float>(attackSamples);
    targetVoice->slotIndex = slotIndex;
    targetVoice->active = true;
}

void DrumEngine::renderSampleVoices(float& leftOut, float& rightOut) {
    leftOut = 0.0f;
    rightOut = 0.0f;

    for (auto& voice : sampleVoices_) {
        if (!voice.active || voice.sampleData == nullptr) {
            continue;
        }

        const auto& audio = voice.sampleData->audio;
        const int numSamples = audio.getNumSamples();
        const int numChannels = audio.getNumChannels();
        const int index = static_cast<int>(voice.samplePosition);
        if (index < 0 || index >= numSamples) {
            voice.active = false;
            continue;
        }

        const int nextIndex = std::min(index + 1, numSamples - 1);
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
        const float env = voice.envelope * voice.attack;

        leftOut += sampleL * voice.gain * env;
        rightOut += sampleR * voice.gain * env;

        voice.samplePosition += voice.increment;
        voice.envelope *= voice.decayMul;

        if (voice.samplePosition >= static_cast<double>(numSamples) || voice.envelope < 1.0e-4f) {
            voice.active = false;
        }
    }
}

void DrumEngine::handleMidiEvent(const juce::MidiMessage& msg) {
    if (!msg.isNoteOn()) {
        return;
    }

    const int midiNote = msg.getNoteNumber();
    const float velocity = msg.getFloatVelocity();

    bool handledBySample = false;
    if (midiNote >= 0 && midiNote <= 127) {
        const uint8_t noteValue = static_cast<uint8_t>(midiNote);
        if (DrumSampleCatalog::isValidMidiNote(noteValue)) {
            const uint8_t slotIndex = DrumSampleCatalog::midiNoteToSlotIndex(noteValue);
            if (slotIndex < SampleSlotCount
                && slotSamples_[slotIndex] != nullptr
                && slotSamples_[slotIndex]->audio.getNumSamples() > 1) {
                triggerSampleVoice(slotIndex, velocity);
                handledBySample = true;
            }
        }
    }

    if (!handledBySample) {
        triggerSynthVoice(midiNote, velocity);
    }
}

void DrumEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
                                 const juce::MidiBuffer& midi,
                                 int numSamples)
{
    buffer.clear();
    refreshMoveEnvelopeState();
    refreshToneTiltState();
    
    int samplePos = 0;
    auto renderFrame = [&]() {
        float kickOut = 0.0f;
        float snareOut = 0.0f;
        float hatOut = 0.0f;

        if (kickEnv_.isActive()) {
            kickOsc_.setFrequency(50.0f + 150.0f * kickPitchEnv_);
            kickPitchEnv_ *= kickPitchEnvDecay_;
            kickOut = kickOsc_.processSample(0.0f) * kickEnv_.getNextSample();
        }

        if (snareEnv_.isActive() || snareNoiseEnv_ > 0.001f) {
            const float tone = snareOsc_.processSample(0.0f) * snareEnv_.getNextSample();
            const float noise = (random_.nextFloat() * 2.0f - 1.0f) * snareNoiseEnv_;
            snareNoiseEnv_ *= snareNoiseDecay_;
            snareOut = tone * 0.4f + noise * 0.6f;
        }

        const float hatEnv = std::max(closedHatEnv_, openHatEnv_);
        if (hatEnv > 0.001f) {
            const float noise = random_.nextFloat() * 2.0f - 1.0f;
            hatOut = hatFilter_.processSample(0, noise) * hatEnv;

            closedHatEnv_ *= closedHatDecay_;
            openHatEnv_ *= openHatDecay_;
        }

        const float synthMix = kickOut + snareOut + hatOut * 0.5f;

        float sampleL = 0.0f;
        float sampleR = 0.0f;
        renderSampleVoices(sampleL, sampleR);

        float outL = synthMix + sampleL;
        float outR = synthMix + sampleR;
        applyToneTilt(outL, outR);

        buffer.addSample(0, samplePos, outL);
        if (buffer.getNumChannels() > 1) {
            buffer.addSample(1, samplePos, outR);
        }

        ++samplePos;
    };

    for (const auto meta : midi) {
        int eventTime = meta.samplePosition;
        
        // Render up to event
        while (samplePos < eventTime && samplePos < numSamples) {
            renderFrame();
        }
        
        handleMidiEvent(meta.getMessage());
    }
    
    // Render rest of block
    while (samplePos < numSamples) {
        renderFrame();
    }
}
