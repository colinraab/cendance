#pragma once

#include <juce_core/juce_core.h>
#include <string>
#include <vector>

#include "../App/AppState.h"
#include "../Network/P2PClient.h"
#include "../Security/PresetSerializer.h"
#include "../Security/SecurityManager.h"

class P2PToolHandler final {
public:
    P2PToolHandler(AppState& appState,
                   SecurityManager& securityManager,
                   PresetSerializer& presetSerializer,
                   P2PClient& p2pClient);

    juce::String handle(const juce::String& toolName,
                        const juce::String& argsJson);

private:
    AppState& appState;
    SecurityManager& securityManager;
    PresetSerializer& presetSerializer;
    P2PClient& p2pClient;
};
