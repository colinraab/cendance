#include "NumberSelectionModal.h"
#include "../Themes/Colors.h"

#include <algorithm>

namespace {

std::string domainName(NumberSelectionDomain domain) {
    switch (domain) {
        case NumberSelectionDomain::Algorithm: return "Algorithm";
        case NumberSelectionDomain::Sound: return "Sound";
        case NumberSelectionDomain::ChordProgression: return "Chord Progression";
           case NumberSelectionDomain::EffectPreset: return "Effect Preset";
        case NumberSelectionDomain::Genre: return "Genre";
    }
    return "Selection";
}

} // namespace

ftxui::Element NumberSelectionModal(NumberSelectionDomain domain,
                                    const std::string& inputDigits,
                                    const std::string& previewName,
                                    bool isValid,
                                    uint16_t implementedCount,
                                    const std::string& statusMessage) {
    using namespace ftxui;

    const std::string typed = inputDigits.empty() ? "_" : inputDigits;
    const std::string preview = previewName.empty() ? "-" : previewName;
    const std::string status = statusMessage.empty() ? "Type a number and press Enter" : statusMessage;

    auto body = vbox({
        text("Mode: " + domainName(domain)) | bold,
        text("ID: " + typed),
        text("Preview: " + preview) | color(isValid ? Theme::Active : Theme::Inactive),
        text(status) | color(isValid ? Theme::Foreground : Theme::Inactive),
        text(std::to_string(implementedCount) + " implemented") | color(Theme::Highlight),
        text("Enter=Apply  Esc=Cancel  Backspace=Delete") | color(Theme::Inactive)
    });

    return window(text(" Number Selector ") | bold | center, body) | clear_under | center;
}
