#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace DrumSampleCatalog {

enum class SlotId : uint8_t {
    Kick = 0,
    Snare = 1,
    ClosedHat = 2,
    OpenHat = 3,
};

struct SlotDefinition {
    SlotId id;
    std::string_view name;
    uint8_t midiNote;
    bool chokesOpenHat;
};

inline constexpr std::array<SlotDefinition, 4> kSlots{{
    {SlotId::Kick, "Kick", 36, false},
    {SlotId::Snare, "Snare", 38, false},
    {SlotId::ClosedHat, "Closed Hat", 42, true},
    {SlotId::OpenHat, "Open Hat", 46, false},
}};

inline constexpr uint8_t getSlotCount() {
    return static_cast<uint8_t>(kSlots.size());
}

inline constexpr bool isValidSlotIndex(uint8_t slotIndex) {
    return slotIndex < getSlotCount();
}

inline constexpr const SlotDefinition& getSlotDefinition(uint8_t slotIndex) {
    if (isValidSlotIndex(slotIndex)) {
        return kSlots[slotIndex];
    }

    return kSlots[0];
}

inline constexpr bool isValidMidiNote(uint8_t midiNote) {
    for (const auto& slot : kSlots) {
        if (slot.midiNote == midiNote) {
            return true;
        }
    }

    return false;
}

inline constexpr uint8_t midiNoteToSlotIndex(uint8_t midiNote) {
    for (uint8_t i = 0; i < getSlotCount(); ++i) {
        if (kSlots[i].midiNote == midiNote) {
            return i;
        }
    }

    return 0;
}

} // namespace DrumSampleCatalog
