#include "../Source/App/EffectPresetCatalog.h"
#include "../Source/Audio/Effects/1_Space/ConvolutionReverb.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>

namespace {

constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 256;
constexpr int kCaptureSamples = 48000;

uint64_t hashResponse(ConvolutionReverb& reverb, bool& hasStereoDifference) {
    uint64_t hash = 1469598103934665603ULL;
    double energy = 0.0;
    hasStereoDifference = false;

    for (int offset = 0; offset < kCaptureSamples; offset += kBlockSize) {
        juce::AudioBuffer<float> block(2, kBlockSize);
        block.clear();
        if (offset == 0) {
            block.setSample(0, 0, 1.0f);
            block.setSample(1, 0, 1.0f);
        }

        reverb.processBlock(block, kBlockSize);
        for (int sample = 0; sample < kBlockSize; ++sample) {
            const float left = block.getSample(0, sample);
            const float right = block.getSample(1, sample);
            assert(std::isfinite(left));
            assert(std::isfinite(right));
            energy += static_cast<double>(left) * left + static_cast<double>(right) * right;
            hasStereoDifference = hasStereoDifference || std::abs(left - right) > 1.0e-6f;

            for (const float value : {left, right}) {
                const auto quantized = static_cast<int32_t>(std::lrint(value * 1000000.0f));
                hash ^= static_cast<uint32_t>(quantized);
                hash *= 1099511628211ULL;
            }
        }
    }

    assert(energy > 1.0e-8);
    return hash;
}

void testAllConvolutionPresetsLoadDistinctResponses() {
    std::set<std::string> resourceNames;
    std::set<uint64_t> responseHashes;
    int presetCount = 0;
    int stereoResponseCount = 0;

    for (const auto& preset : EffectPresetCatalog::kPresets) {
        if (preset.type != EffectPresetCatalog::EffectType::ConvolutionReverb) {
            continue;
        }

        ++presetCount;
        assert(!preset.irResourceName.empty());
        assert(resourceNames.emplace(preset.irResourceName).second);

        ConvolutionReverb reverb;
        assert(reverb.loadIrFromResource(preset.irResourceName));
        reverb.prepare(kSampleRate, kBlockSize);
        assert(reverb.getCurrentIrSize() > 0);
        reverb.setMix(1.0f);
        reverb.setPreDelayMs(0.0f);
        reverb.setIrGain(0.0f);
        reverb.setActive(true);

        bool hasStereoDifference = false;
        const uint64_t responseHash = hashResponse(reverb, hasStereoDifference);
        assert(responseHashes.emplace(responseHash).second);
        stereoResponseCount += hasStereoDifference ? 1 : 0;
    }

    assert(presetCount == 10);
    assert(stereoResponseCount >= 5);
}

void testPreDelayDoesNotDelayDrySignal() {
    ConvolutionReverb reverb;
    assert(reverb.loadIrFromResource(EffectPresetCatalog::kDefaultConvolutionResourceName));
    reverb.prepare(kSampleRate, kBlockSize);
    reverb.setMix(0.0f);
    reverb.setPreDelayMs(100.0f);
    reverb.setActive(true);

    juce::AudioBuffer<float> block(2, kBlockSize);
    block.clear();
    block.setSample(0, 0, 1.0f);
    block.setSample(1, 0, 1.0f);
    reverb.processBlock(block, kBlockSize);

    assert(std::abs(block.getSample(0, 0) - 1.0f) < 1.0e-6f);
    assert(std::abs(block.getSample(1, 0) - 1.0f) < 1.0e-6f);
}

} // namespace

int main() {
    testAllConvolutionPresetsLoadDistinctResponses();
    testPreDelayDoesNotDelayDrySignal();
    std::cout << "Convolution reverb tests passed!\n";
    return 0;
}
