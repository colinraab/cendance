#include "KeyEntryModal.h"
#include "../Themes/Colors.h"

ftxui::Element KeyEntryModal(const std::string& inputText,
                             const std::string& previewText,
                             bool isValid,
                             const std::string& statusMessage) {
    using namespace ftxui;

    const std::string typed = inputText.empty() ? "_" : inputText;
    const std::string preview = previewText.empty() ? "-" : previewText;
    const std::string status = statusMessage.empty()
        ? "Enter key like A, a, A#, bb, Db major, c#min"
        : statusMessage;

    auto body = vbox({
        text("Input: " + typed),
        text("Preview: " + preview) | color(isValid ? Theme::Active : Theme::Inactive),
        text(status) | color(isValid ? Theme::Foreground : Theme::Inactive),
        text("Enter=Apply  Esc=Cancel  Backspace=Delete") | color(Theme::Inactive)
    });

    return window(text(" Project Key ") | bold | center, body) | clear_under | center;
}
