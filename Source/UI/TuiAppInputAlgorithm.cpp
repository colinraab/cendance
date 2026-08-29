#include "TuiApp.h"
#include "TuiAppInput.h"

#include <ftxui/component/event.hpp>
#include <cctype>

using namespace tui_input;

bool TuiApp::handleAlgorithmEditorInput(const ftxui::Event& event) {
  using namespace ftxui;

  if (event == Event::Escape) {
    closeAlgorithmEditor();
    return true;
  }
  if (event.is_character()) {
    const std::string chars = event.character();
    if (chars.size() == 1) {
      const char c = chars[0];
      switch (c) {
      case 's':
      case 'S':
        saveAlgorithmEditorDraft();
        return true;
      case 'd':
      case 'D':
        deleteSelectedCustomAlgorithm();
        return true;
      case 't':
      case 'T':
        if (algorithmEditorSelectedStep < algorithmEditorDraft.rhythmicPattern.size()) {
          algorithmEditorDraft.rhythmicPattern[algorithmEditorSelectedStep] =
              algorithmEditorDraft.rhythmicPattern[algorithmEditorSelectedStep] ? 0 : 1;
        }
        return true;
      case '+':
        if (algorithmEditorStepCount < CustomAlgorithmPreset::kMaxStepCount) {
          algorithmEditorStepCount++;
          algorithmEditorDraft.rhythmicPattern.resize(algorithmEditorStepCount, 0);
          algorithmEditorDraft.melodicPattern.resize(algorithmEditorStepCount, 0);
          algorithmEditorDraft.densityCurve.resize(algorithmEditorStepCount, 1.0f);
          algorithmEditorDraft.complexityCurve.resize(algorithmEditorStepCount, 1.0f);
        }
        return true;
      case '-':
        if (algorithmEditorStepCount > CustomAlgorithmPreset::kMinStepCount) {
          algorithmEditorStepCount--;
          algorithmEditorDraft.rhythmicPattern.resize(algorithmEditorStepCount, 0);
          algorithmEditorDraft.melodicPattern.resize(algorithmEditorStepCount, 0);
          algorithmEditorDraft.densityCurve.resize(algorithmEditorStepCount, 1.0f);
          algorithmEditorDraft.complexityCurve.resize(algorithmEditorStepCount, 1.0f);
          if (algorithmEditorSelectedStep >= algorithmEditorStepCount)
            algorithmEditorSelectedStep = algorithmEditorStepCount - 1;
        }
        return true;
      }
    }
  }
  if (event == Event::ArrowLeft) {
    if (algorithmEditorSelectedStep > 0)
      algorithmEditorSelectedStep--;
    return true;
  }
  if (event == Event::ArrowRight) {
    if (algorithmEditorSelectedStep < algorithmEditorStepCount - 1)
      algorithmEditorSelectedStep++;
    return true;
  }
  if (event == Event::ArrowUp) {
    if (selectedCustomAlgorithmIndex > 0)
      selectedCustomAlgorithmIndex--;
    return true;
  }
  if (event == Event::ArrowDown) {
    if (selectedCustomAlgorithmIndex < static_cast<int>(cachedCustomAlgorithms.size()) - 1)
      selectedCustomAlgorithmIndex++;
    return true;
  }
  return true;
}
