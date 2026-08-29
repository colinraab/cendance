#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

ftxui::Element KeyEntryModal(const std::string& inputText,
                             const std::string& previewText,
                             bool isValid,
                             const std::string& statusMessage);
