#pragma once

#include "AgentCommand.h"
#include "AppState.h"
#include "CommandQueue.h"
#include "ContributionPackage.h"
#include "MeterQueue.h"

#include <string>
#include <vector>

namespace AgentCommand {

// --- Shared constants ---
constexpr uint8_t kMasterTrackIndex = 4;

// --- String utilities ---
std::string trimCopy(const std::string& text);
std::string lowerCopy(std::string text);
std::string jsonEscape(const std::string& text);
std::string quoted(const std::string& text);

// --- Response builder ---
Response makeResponse(bool ok, const std::string& message,
                      const std::string& dataJson = "");

// --- Tokenizer & parsers ---
std::vector<std::string> tokenize(const std::string& input, std::string& error);
bool parseFloat(const std::string& text, float& out);
bool parseUInt(const std::string& text, uint16_t& out);
bool parseDurationSeconds(const std::string& token, double& seconds);

// --- Display helpers ---
const char* trackName(uint8_t track);
const char* arrangementModeName(uint8_t mode);
std::string commandHelpJson();

// --- Command dispatch ---
bool dispatch(ExecutionContext& context, const Command& command,
              const std::string& description, const Command& undoCommand,
              std::string& error);
bool dispatchCommandList(ExecutionContext& context,
                         const std::vector<std::pair<Command, Command>>& commands,
                         const std::string& description,
                         std::string& error);

// --- Listen analysis ---
struct ListenScores {
    float energy = 0.0f;
    float balance = 0.0f;
    float lowMidHigh = 0.0f;
    float variation = 0.0f;
    float clippingRisk = 0.0f;
    float silenceRisk = 0.0f;
    float arrangementMotion = 0.0f;
    float overall = 0.0f;
    std::vector<std::string> notes;
};

ListenScores analyzeMeterHistory(const std::vector<MeterData>& history,
                                 double seconds);
std::string listenJson(const std::vector<MeterData>& history, double seconds);

// --- Preset registry refresh ---
void refreshPresetRegistry(ExecutionContext& context);

// --- Macro/FX/Arrangement command builders ---
void appendMacroCommands(const ContributionPackage::MacroDefaults& macros,
                         uint8_t track,
                         const AppState& appState,
                         std::vector<std::pair<Command, Command>>& commands);
void appendTrackFxCommands(const std::array<uint16_t, 3>& fxPresetIds,
                           uint8_t track,
                           const AppState& appState,
                           std::vector<std::pair<Command, Command>>& commands);
void appendMasterFxCommands(const std::array<uint16_t, 3>& fxPresetIds,
                            const AppState& appState,
                            std::vector<std::pair<Command, Command>>& commands);
void appendArrangementCommands(const ContributionPackage::ArrangementPresetItem& item,
                               const AppState& appState,
                               std::vector<std::pair<Command, Command>>& commands);

} // namespace AgentCommand
