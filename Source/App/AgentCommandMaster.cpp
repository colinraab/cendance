#include "AgentCommandMaster.h"

#include "EffectPresetCatalog.h"

namespace AgentCommand {
namespace {

} // namespace

Response executeMaster(ExecutionContext& context, const std::vector<std::string>& tokens) {
    if (tokens.size() != 4 || lowerCopy(tokens[1]) != "fx") {
        return makeResponse(false, "Usage: master fx <slot 1-3> <effect id>.");
    }

    uint16_t slotDisplay = 0;
    uint16_t effectDisplay = 0;
    if (!parseUInt(tokens[2], slotDisplay) || slotDisplay < 1 || slotDisplay > 3) {
        return makeResponse(false, "FX slot must be 1-3.");
    }
    if (!parseUInt(tokens[3], effectDisplay) ||
        !EffectPresetCatalog::isValidDisplayId(effectDisplay)) {
        return makeResponse(false, "Effect id is out of range.");
    }

    const uint8_t slot = static_cast<uint8_t>(slotDisplay - 1);
    const uint16_t preset = EffectPresetCatalog::displayIdToPresetId(effectDisplay);
    const uint16_t previous = context.appState.master.getEffectPresetSlot(slot);
    const Command command{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                          Command::encodeEffectSlotPreset(slot, preset), 0.0f};
    const Command undo{Command::Type::SetMasterEffectPreset, kMasterTrackIndex,
                       Command::encodeEffectSlotPreset(slot, previous), 0.0f};
    std::string error;
    if (!dispatch(context, command, "Agent Master FX", undo, error)) {
        return makeResponse(false, error);
    }
    return makeResponse(true, "Master FX command accepted.");
}

} // namespace AgentCommand
