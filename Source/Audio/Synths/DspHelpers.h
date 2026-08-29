#pragma once

#include <algorithm>
#include <cmath>

namespace DspHelpers {

inline float saturateTanh(float input, float drive = 1.0f) {
    return std::tanh(input * std::max(drive, 0.0f));
}

inline float waveFold(float input, float drive = 1.0f, float limit = 1.0f) {
    const float clampedLimit = std::max(limit, 0.0001f);
    float x = input * std::max(drive, 0.0f);

    if (x > clampedLimit) {
        x = 2.0f * clampedLimit - x;
    } else if (x < -clampedLimit) {
        x = -2.0f * clampedLimit - x;
    }

    return std::clamp(x, -clampedLimit, clampedLimit);
}

inline float foldWithAmount(float input, float amount, float maxAdditionalGain) {
    const float gain = 1.0f + std::clamp(amount, 0.0f, 1.0f) * std::max(maxAdditionalGain, 0.0f);
    return waveFold(input, gain);
}

} // namespace DspHelpers