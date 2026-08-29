#pragma once

#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>

enum class ProjectPathModalMode {
    Save,
    Load,
};

enum class SaveProjectFieldFocus {
    Directory,
    Name,
};

ftxui::Element ProjectPathModal(ProjectPathModalMode mode,
                                const std::string& saveDirectory,
                                const std::string& saveName,
                                const std::string& loadPath,
                                SaveProjectFieldFocus saveFieldFocus,
                                const std::vector<std::string>& recentPaths,
                                int selectedRecentIndex,
                                const std::string& statusMessage,
                                bool statusIsError);
