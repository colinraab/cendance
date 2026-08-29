#include "ParameterBar.h"
#include "../Themes/Colors.h"

ftxui::Element ParameterBar(const std::string& label, float value, int barWidth) {
    using namespace ftxui;

    if (barWidth < 1) barWidth = 1;
    int fill = static_cast<int>(value * static_cast<float>(barWidth));
    if (fill < 0) fill = 0;
    if (fill > barWidth) fill = barWidth;

    std::string barStr = "[";
    for (int i = 0; i < barWidth; ++i) {
        barStr += (i < fill) ? "█" : "░";
    }
    barStr += "]";

    // Label width based on actual label length, not bar width.
    // Full labels ("Dens","Cmpl","Tone","Move") = 4 chars → 6 (label + ":" + 1 pad space)
    // Compact labels ("Dn","Cm","Tn","Mv")       = 2 chars → 3 (label + ":" exactly)
    const int labelWidth = (static_cast<int>(label.size()) > 2) ? 6 : 3;

    return hbox({
        text(label + ":") | size(WIDTH, EQUAL, labelWidth),
        text(barStr) | color(Theme::Highlight)
    });
}
