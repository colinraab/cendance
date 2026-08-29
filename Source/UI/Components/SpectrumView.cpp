#include "SpectrumView.h"
#include "../Themes/Colors.h"
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

float applyLogAmplitudeScaling(float normalizedAmplitude) {
    // Apply logarithmic compression to the top of the range so hot bins do not all hit max height.
    constexpr float kHeadroomLogScale = 2.0f;
    const float clamped = std::clamp(normalizedAmplitude, 0.0f, 1.0f);
    const float headroom = 1.0f - clamped;
    const float compressedHeadroom = std::log1pf(kHeadroomLogScale * headroom) / std::log1pf(kHeadroomLogScale);
    return 1.0f - compressedHeadroom;
}

} // namespace

ftxui::Element SpectrumView(const MeterData& meterData) {
    using namespace ftxui;

    constexpr int height = 8;
    constexpr int minWidth = 8;
    const int terminalWidth = std::max(minWidth, Terminal::Size().dimx);
    const int innerWidth = std::max(minWidth, terminalWidth - 4);

    std::vector<std::string> rows(static_cast<size_t>(height), std::string(static_cast<size_t>(innerWidth), ' '));

    if (meterData.analyzerValid) {
        for (int x = 0; x < innerWidth; ++x) {
            const size_t sourceIndex = static_cast<size_t>((x * (kSpectrumBinCount - 1)) / std::max(1, innerWidth - 1));
            const float binValue = applyLogAmplitudeScaling(meterData.spectrumBins[sourceIndex]);
            const int barHeight = std::clamp(static_cast<int>(std::lround(binValue * static_cast<float>(height))), 0, height);

            for (int y = 0; y < barHeight; ++y) {
                const int row = (height - 1) - y;
                rows[static_cast<size_t>(row)][static_cast<size_t>(x)] = '#';
            }
        }
    }

    Elements lines;
    for (int row = 0; row < height; ++row) {
        auto colorRef = Theme::MeterLow;
        if (row < height / 2) {
            colorRef = Theme::MeterMid;
        }
        if (row < height / 4) {
            colorRef = Theme::MeterHigh;
        }
        lines.push_back(text(rows[static_cast<size_t>(row)]) | color(colorRef));
    }

    auto footer = hbox({
        text("L") | color(Theme::Inactive),
        filler(),
        text("M") | color(Theme::Inactive),
        filler(),
        text("H") | color(Theme::Inactive)
    });

    return window(text(" Spectrum "), vbox({vbox(std::move(lines)), separator(), footer})) | xflex;
}
