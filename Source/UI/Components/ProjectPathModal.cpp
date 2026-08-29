#include "ProjectPathModal.h"
#include "../Themes/Colors.h"

#include <algorithm>

namespace {

std::string withCursor(const std::string& value, bool focused) {
    const std::string base = value.empty() ? "" : value;
    return focused ? (base + "|") : base;
}

} // namespace

ftxui::Element ProjectPathModal(ProjectPathModalMode mode,
                                const std::string& saveDirectory,
                                const std::string& saveName,
                                const std::string& loadPath,
                                SaveProjectFieldFocus saveFieldFocus,
                                const std::vector<std::string>& recentPaths,
                                int selectedRecentIndex,
                                const std::string& statusMessage,
                                bool statusIsError) {
    using namespace ftxui;

    const std::string title = (mode == ProjectPathModalMode::Save)
        ? " Save Project "
        : " Load Project ";
    const std::string modeName = (mode == ProjectPathModalMode::Save) ? "Save" : "Load";

    Elements bodyItems;
    bodyItems.push_back(text("Mode: " + modeName) | bold);
    bodyItems.push_back(separator());

    if (mode == ProjectPathModalMode::Save) {
        const bool directoryFocused = (saveFieldFocus == SaveProjectFieldFocus::Directory);
        const bool nameFocused = (saveFieldFocus == SaveProjectFieldFocus::Name);
        const std::string safeDirectory = saveDirectory.empty() ? "_" : withCursor(saveDirectory, directoryFocused);
        const std::string safeName = saveName.empty() ? "|" : withCursor(saveName, nameFocused);
        const std::string previewName = saveName.empty() ? "untitled" : saveName;

        bodyItems.push_back(text("Project Name (no extension):"));
        bodyItems.push_back(text(safeName) | color(nameFocused ? Theme::Active : Theme::Foreground));
        bodyItems.push_back(text("Extension: .cendance") | color(Theme::Inactive));
        bodyItems.push_back(separator());
        bodyItems.push_back(text("Directory:"));
        bodyItems.push_back(text(safeDirectory) | color(directoryFocused ? Theme::Active : Theme::Foreground));
        bodyItems.push_back(separator());
        bodyItems.push_back(text("Preview:"));
        bodyItems.push_back(text(saveDirectory + "/" + previewName + ".cendance") | color(Theme::Highlight));
    } else {
        const std::string safePath = loadPath.empty() ? "|" : withCursor(loadPath, true);
        bodyItems.push_back(text("Path:"));
        bodyItems.push_back(text(safePath) | color(Theme::Active));
        bodyItems.push_back(separator());
        bodyItems.push_back(text("Recent:"));

        Elements recentItems;
        if (recentPaths.empty()) {
            recentItems.push_back(text("(No recent projects)") | color(Theme::Inactive));
        } else {
            const int maxRows = std::min<int>(6, static_cast<int>(recentPaths.size()));
            for (int i = 0; i < maxRows; ++i) {
                const bool selected = (i == selectedRecentIndex);
                const std::string prefix = selected ? "> " : "  ";
                recentItems.push_back(
                    text(prefix + recentPaths[static_cast<size_t>(i)])
                        | color(selected ? Theme::Highlight : Theme::Foreground));
            }
        }

        bodyItems.push_back(vbox(std::move(recentItems)));
    }

    bodyItems.push_back(separator());
    const std::string defaultStatus = (mode == ProjectPathModalMode::Save)
        ? "Enter=Confirm  Esc=Cancel  Tab/Up/Down=Switch Field"
        : "Enter=Confirm  Esc=Cancel  Up/Down=Recents";
    bodyItems.push_back(
        text(statusMessage.empty() ? defaultStatus : statusMessage)
            | color(statusIsError ? Theme::Error : Theme::Foreground));

    auto body = vbox(std::move(bodyItems));

    return window(text(title) | bold | center, body) | clear_under | center;
}
