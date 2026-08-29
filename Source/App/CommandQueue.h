#pragma once

#include <atomic>
#include <array>
#include <cstdint>
#include <optional>

struct Command {
    static constexpr uint16_t kEffectSlotBits = 2;
    static constexpr uint16_t kEffectPresetBits = 14;
    static constexpr uint16_t kEffectPresetMask = static_cast<uint16_t>((1u << kEffectPresetBits) - 1u);
    static constexpr uint16_t kDrumSlotBits = 2;
    static constexpr uint16_t kDrumSampleIdBits = 14;
    static constexpr uint16_t kDrumSampleIdMask = static_cast<uint16_t>((1u << kDrumSampleIdBits) - 1u);
    static constexpr uint16_t kProjectKeyRootBits = 4;
    static constexpr uint16_t kProjectKeyModeBits = 4;
    static constexpr uint16_t kProjectKeyRootMask = static_cast<uint16_t>((1u << kProjectKeyRootBits) - 1u);
    static constexpr uint16_t kProjectKeyModeMask = static_cast<uint16_t>((1u << kProjectKeyModeBits) - 1u);
    static constexpr uint16_t kProjectKeyModeShift = kProjectKeyRootBits;
    static constexpr uint16_t kArrangementSectionBits = 4;
    static constexpr uint16_t kArrangementValueBits = 12;
    static constexpr uint16_t kArrangementSectionMask = static_cast<uint16_t>((1u << kArrangementSectionBits) - 1u);
    static constexpr uint16_t kArrangementValueMask = static_cast<uint16_t>((1u << kArrangementValueBits) - 1u);
    static constexpr uint16_t kArrangementSectionShift = kArrangementValueBits;

    static constexpr uint16_t encodeEffectSlotPreset(uint8_t slotIndex, uint16_t presetId) {
        return static_cast<uint16_t>(((static_cast<uint16_t>(slotIndex) & 0x3u) << kEffectPresetBits)
            | (presetId & kEffectPresetMask));
    }

    static constexpr uint8_t decodeEffectSlotIndex(uint16_t payload) {
        return static_cast<uint8_t>((payload >> kEffectPresetBits) & 0x3u);
    }

    static constexpr uint16_t decodeEffectPresetId(uint16_t payload) {
        return static_cast<uint16_t>(payload & kEffectPresetMask);
    }

    static constexpr uint16_t encodeDrumSlotSampleId(uint8_t slotIndex, uint16_t sampleId) {
        return static_cast<uint16_t>(((static_cast<uint16_t>(slotIndex) & 0x3u) << kDrumSampleIdBits)
            | (sampleId & kDrumSampleIdMask));
    }

    static constexpr uint8_t decodeDrumSlotIndex(uint16_t payload) {
        return static_cast<uint8_t>((payload >> kDrumSampleIdBits) & 0x3u);
    }

    static constexpr uint16_t decodeDrumSampleId(uint16_t payload) {
        return static_cast<uint16_t>(payload & kDrumSampleIdMask);
    }

    static constexpr uint16_t encodeProjectKey(uint8_t keyRoot, uint8_t keyMode) {
        return static_cast<uint16_t>(((static_cast<uint16_t>(keyMode) & kProjectKeyModeMask) << kProjectKeyModeShift)
            | (static_cast<uint16_t>(keyRoot) & kProjectKeyRootMask));
    }

    static constexpr uint8_t decodeProjectKeyRoot(uint16_t payload) {
        return static_cast<uint8_t>(payload & kProjectKeyRootMask);
    }

    static constexpr uint8_t decodeProjectKeyMode(uint16_t payload) {
        return static_cast<uint8_t>((payload >> kProjectKeyModeShift) & kProjectKeyModeMask);
    }

    static constexpr uint16_t encodeArrangementSectionValue(uint8_t sectionIndex, uint16_t value) {
        return static_cast<uint16_t>(((static_cast<uint16_t>(sectionIndex) & kArrangementSectionMask) << kArrangementSectionShift)
            | (value & kArrangementValueMask));
    }

    static constexpr uint8_t decodeArrangementSectionIndex(uint16_t payload) {
        return static_cast<uint8_t>((payload >> kArrangementSectionShift) & kArrangementSectionMask);
    }

    static constexpr uint16_t decodeArrangementValue(uint16_t payload) {
        return static_cast<uint16_t>(payload & kArrangementValueMask);
    }

    enum class SpotEffectId : uint16_t {
        TapeBrake = 0,
        Stutter = 1,
    };

    static constexpr bool isValidSpotEffectId(uint16_t effectId) {
        return effectId <= static_cast<uint16_t>(SpotEffectId::Stutter);
    }

    enum class Type : uint8_t {
        SetAlgorithm,   // track + algorithm ID
        StepAlgorithm,  // track + value +/-1 algorithm step with wrap
        SetDensity,     // track + value [0.0, 1.0]
        SetComplexity,  // track + value [0.0, 1.0]
        SetSynthPreset, // track + preset index
        StepSynthPreset,// track + value +/-1 preset step with wrap
        SetTone,        // track + value delta [0.0, 1.0]
        SetMotion,      // track + value delta [0.0, 1.0]
        SetTrackGain,   // track + gain delta; trackIndex 4 targets master
        SetChordProg,   // progression ID
        SetGenre,       // genre ID 1-8
        RandomizeForGenre, // genre ID 1-8: randomizes tempo + algorithms
        SetArrangementSectionCount, // section count [1, AppState::kArrangementMaxSections]
        SetArrangementSection, // absolute section index
        SetArrangementMode, // absolute arrangement mode [manual, auto, mixed]
        StepArrangementSection, // value +/-1 section step with wrap
        StepArrangementMode, // cycle arrangement mode (manual -> auto -> mixed)
        SetArrangementSectionLength, // encoded section/value payload (bars)
        SetArrangementSectionProgression, // encoded section/value payload (progression or follow-global sentinel)
        SetArrangementSectionTrackMask, // encoded section/value payload (4-bit active track mask)
        SetArrangementSectionParametersEnabled, // bool in paramId
        SetArrangementSectionTrackParameter, // trackIndex = track + parameterIndex * 4, paramId = section, value [0, 1]
        SetArrangementChainEnabled, // bool in paramId (0 = linear wrap, non-zero = use chain)
        SetArrangementChainLength, // chain length [1, AppState::kArrangementMaxSections]
        SetArrangementChainStep, // encoded chain-index/section-index payload
        SetProjectKey,  // encoded key root/mode payload
        SetTempo,       // BPM delta
        RebuildCustomAlgorithms, // no params: rebuild custom algorithm instances from registry
        PlayStop,       // toggle play/pause
        Stop,           // stop + reset transport position
        ToggleMetronome,// toggle
        ToggleTrackMute,// toggle selected track mute
        SetTrackEffectPreset, // track + encoded slot/preset payload
        SetMasterEffectPreset, // encoded slot/preset payload on master bus
        SpotEffectOn,   // SpotEffectId in paramId
        SpotEffectOff,  // SpotEffectId in paramId
        SpotEffectToggle, // SpotEffectId in paramId
        SetDrumSampleAssignment, // encoded drum slot/sample payload (track 0 only)
        ClearDrumSampleAssignment, // drum slot in paramId (track 0 only)
        SetDrumSampleVolume, // drum slot in paramId, absolute value in cmd.value
        SetDrumSampleTune, // drum slot in paramId, absolute semitone value in cmd.value
        SetDrumSampleStartOffset, // drum slot in paramId, absolute normalized value in cmd.value
        SetDrumSampleDecay, // drum slot in paramId, absolute normalized value in cmd.value
        SetDrumSampleVelocitySensitivity, // drum slot in paramId, absolute normalized value in cmd.value
        SetDensityAbsolute,     // track + absolute value [0.0, 1.0]
        SetComplexityAbsolute,  // track + absolute value [0.0, 1.0]
        SetToneAbsolute,        // track + absolute value [0.0, 1.0]
        SetMotionAbsolute,      // track + absolute value [0.0, 1.0]
        SetTrackGainAbsolute,   // track + absolute gain value; trackIndex 4 targets master
        SetTempoAbsolute,       // absolute BPM
    };
    Type type;
    uint8_t trackIndex;    // 0-3 for musical tracks, 4 for master gain; ignored for other global/master-domain commands
    uint16_t paramId;
    float value;
};

class CommandQueue {
public:
    static constexpr size_t Capacity = 256;

    // Push from UI thread
    bool push(const Command& cmd) {
        auto currentWrite = writePos.load(std::memory_order_relaxed);
        auto nextWrite = increment(currentWrite);
        
        if (nextWrite != readPos.load(std::memory_order_acquire)) {
            buffer[currentWrite] = cmd;
            writePos.store(nextWrite, std::memory_order_release);
            return true;
        }
        return false; // Queue full
    }

    // Pop from Audio thread
    bool pop(Command& cmd) {
        auto currentRead = readPos.load(std::memory_order_relaxed);
        
        if (currentRead == writePos.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }
        
        cmd = buffer[currentRead];
        readPos.store(increment(currentRead), std::memory_order_release);
        return true;
    }

private:
    static constexpr size_t increment(size_t pos) {
        return (pos + 1) % Capacity;
    }

    std::array<Command, Capacity> buffer;
    alignas(64) std::atomic<size_t> writePos{0};
    alignas(64) std::atomic<size_t> readPos{0};
};
