#pragma once

#include "GenreCatalog.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>

struct AppState {
    static constexpr uint8_t kProjectKeyModeMajor = 0;
    static constexpr uint8_t kProjectKeyModeNaturalMinor = 1;
    static constexpr uint8_t kArrangementModeManual = 0;
    static constexpr uint8_t kArrangementModeAuto = 1;
    static constexpr uint8_t kArrangementModeMixed = 2;
    static constexpr uint8_t kArrangementMaxSections = 8;
    static constexpr uint8_t kArrangementTrackMaskAll = 0x0Fu;
    static constexpr uint8_t kArrangementDefaultSectionLengthBars = 4;
    static constexpr uint8_t kArrangementMinSectionLengthBars = 1;
    static constexpr uint8_t kArrangementMaxSectionLengthBars = 16;
    static constexpr uint8_t kArrangementProgressionFollowGlobal = 0xFFu;
    static constexpr uint8_t kArrangementDefaultChainLength = 4;
    static constexpr uint8_t kTrackCount = 4;
    static constexpr uint8_t kArrangementTrackParameterCount = 4;
    static constexpr float kMaxTrackGain = 2.0f;
    static constexpr float kMaxMasterGain = 4.0f;

    std::atomic<float> bpm{120.0f};
    std::atomic<bool> playing{false};
    std::atomic<bool> metronomeEnabled{false};
    std::atomic<uint8_t> chordProgression{0};
    std::atomic<uint8_t> genre{0}; // 0 = no genre, 1-8 = GenreCatalog genre ID
    std::atomic<uint8_t> projectKeyRoot{0}; // Pitch class: C=0, C#/Db=1, ... B=11
    std::atomic<uint8_t> projectKeyMode{kProjectKeyModeNaturalMinor}; // Major or NaturalMinor
    std::atomic<uint8_t> arrangementSectionCount{4};
    std::atomic<uint8_t> arrangementCurrentSection{0};
    std::atomic<uint8_t> arrangementMode{kArrangementModeMixed};
    std::atomic<uint8_t> arrangementSectionLengths[kArrangementMaxSections]{
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
        {kArrangementDefaultSectionLengthBars},
    };
    std::atomic<uint8_t> arrangementSectionProgressions[kArrangementMaxSections]{
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
        {kArrangementProgressionFollowGlobal},
    };
    std::atomic<uint8_t> arrangementSectionTrackMasks[kArrangementMaxSections]{
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
        {kArrangementTrackMaskAll},
    };
    std::atomic<bool> arrangementSectionParametersEnabled{false};
    std::atomic<float> arrangementSectionTrackParameters[kArrangementMaxSections][kTrackCount][kArrangementTrackParameterCount]{};
    std::atomic<bool> arrangementChainEnabled{false};
    std::atomic<uint8_t> arrangementChainLength{kArrangementDefaultChainLength};
    std::atomic<uint8_t> arrangementChainSequence[kArrangementMaxSections]{
        {0},
        {1},
        {2},
        {3},
        {4},
        {5},
        {6},
        {7},
    };

    struct TrackState {
        static constexpr uint8_t DrumSampleSlotCount = 4;

        struct DrumSampleSlotState {
            std::atomic<uint16_t> sampleId{0}; // 0 means no sample assignment
            std::atomic<float> volume{1.0f};
            std::atomic<float> tuneSemitones{0.0f};
            std::atomic<float> startOffset{0.0f};
            std::atomic<float> decay{1.0f};
            std::atomic<float> velocitySensitivity{1.0f};
        };

        std::atomic<uint16_t> algorithmId{0};
        std::atomic<uint8_t> synthPreset{0};
        std::atomic<float> density{0.5f};
        std::atomic<float> complexity{0.5f};
        std::atomic<float> tone{0.5f};
        std::atomic<float> motion{0.5f};
        std::atomic<bool> muted{false};
        std::atomic<bool> synthManualOverride{false};
        std::atomic<float> gain{1.0f};
        std::atomic<uint16_t> effectPresetSlots[3]{{0}, {0}, {0}};
        std::array<DrumSampleSlotState, DrumSampleSlotCount> drumSampleSlots{};

        // Helper to avoid repetitive atomic writes if value hasn't changed
        void setDensity(float val) { density.store(val, std::memory_order_relaxed); }
        void setComplexity(float val) { complexity.store(val, std::memory_order_relaxed); }
        void setAlgorithmId(uint16_t val) { algorithmId.store(val, std::memory_order_relaxed); }
        void setSynthPreset(uint8_t val) { synthPreset.store(val, std::memory_order_relaxed); }
        void setTone(float val) { tone.store(val, std::memory_order_relaxed); }
        void setMotion(float val) { motion.store(val, std::memory_order_relaxed); }
        void setMuted(bool val) { muted.store(val, std::memory_order_relaxed); }
        void setSynthManualOverride(bool val) { synthManualOverride.store(val, std::memory_order_relaxed); }
        void setGain(float val) { gain.store(val, std::memory_order_relaxed); }
        void setEffectPresetSlot(uint8_t slotIndex, uint16_t presetId) {
            if (slotIndex < 3) {
                effectPresetSlots[slotIndex].store(presetId, std::memory_order_relaxed);
            }
        }
        uint16_t getEffectPresetSlot(uint8_t slotIndex) const {
            if (slotIndex < 3) {
                return effectPresetSlots[slotIndex].load(std::memory_order_relaxed);
            }
            return 0;
        }

        void setDrumSampleSlotSampleId(uint8_t slotIndex, uint16_t sampleId) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].sampleId.store(sampleId, std::memory_order_relaxed);
            }
        }

        uint16_t getDrumSampleSlotSampleId(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].sampleId.load(std::memory_order_relaxed);
            }
            return 0;
        }

        void setDrumSampleSlotVolume(uint8_t slotIndex, float value) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].volume.store(value, std::memory_order_relaxed);
            }
        }

        float getDrumSampleSlotVolume(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].volume.load(std::memory_order_relaxed);
            }
            return 1.0f;
        }

        void setDrumSampleSlotTuneSemitones(uint8_t slotIndex, float value) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].tuneSemitones.store(value, std::memory_order_relaxed);
            }
        }

        float getDrumSampleSlotTuneSemitones(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].tuneSemitones.load(std::memory_order_relaxed);
            }
            return 0.0f;
        }

        void setDrumSampleSlotStartOffset(uint8_t slotIndex, float value) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].startOffset.store(value, std::memory_order_relaxed);
            }
        }

        float getDrumSampleSlotStartOffset(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].startOffset.load(std::memory_order_relaxed);
            }
            return 0.0f;
        }

        void setDrumSampleSlotDecay(uint8_t slotIndex, float value) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].decay.store(value, std::memory_order_relaxed);
            }
        }

        float getDrumSampleSlotDecay(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].decay.load(std::memory_order_relaxed);
            }
            return 1.0f;
        }

        void setDrumSampleSlotVelocitySensitivity(uint8_t slotIndex, float value) {
            if (slotIndex < DrumSampleSlotCount) {
                drumSampleSlots[slotIndex].velocitySensitivity.store(value, std::memory_order_relaxed);
            }
        }

        float getDrumSampleSlotVelocitySensitivity(uint8_t slotIndex) const {
            if (slotIndex < DrumSampleSlotCount) {
                return drumSampleSlots[slotIndex].velocitySensitivity.load(std::memory_order_relaxed);
            }
            return 1.0f;
        }
    };
    TrackState tracks[kTrackCount];

    struct MasterState {
        std::atomic<float> gain{2.0f};
        std::atomic<uint16_t> effectPresetSlots[3]{{0}, {0}, {0}};

        void setGain(float val) { gain.store(val, std::memory_order_relaxed); }

        void setEffectPresetSlot(uint8_t slotIndex, uint16_t presetId) {
            if (slotIndex < 3) {
                effectPresetSlots[slotIndex].store(presetId, std::memory_order_relaxed);
            }
        }

        uint16_t getEffectPresetSlot(uint8_t slotIndex) const {
            if (slotIndex < 3) {
                return effectPresetSlots[slotIndex].load(std::memory_order_relaxed);
            }
            return 0;
        }
    };
    MasterState master;

    // --- Groove / Swing / Humanization ---
    // All values 0.0–1.0. 0 = no effect, 1.0 = maximum.
    std::atomic<float> swingAmount{0.0f};       // Global swing: shifts even 8th notes (0=straight, 0.5=triplet, 1.0=max)
    std::atomic<float> velocityHumanize{0.0f};   // Per-note velocity randomization (0=none, 1.0=±50 vel)
    std::atomic<float> timingJitter{0.0f};       // Per-note timing micro-offset in ms (0=none, 1.0=±10ms)

    std::atomic<uint8_t> activeSpotEffects{0}; // Bitmask-driven spot FX runtime state

    AppState() {
        for (uint8_t section = 0; section < kArrangementMaxSections; ++section) {
            for (uint8_t track = 0; track < kTrackCount; ++track) {
                for (uint8_t parameter = 0; parameter < kArrangementTrackParameterCount; ++parameter) {
                    arrangementSectionTrackParameters[section][track][parameter].store(0.5f, std::memory_order_relaxed);
                }
            }
        }
    }

    void setBpm(float val) { bpm.store(val, std::memory_order_relaxed); }
    void setPlaying(bool val) { playing.store(val, std::memory_order_relaxed); }
    void setMetronomeEnabled(bool val) { metronomeEnabled.store(val, std::memory_order_relaxed); }
    void setChordProgression(uint8_t val) { chordProgression.store(val, std::memory_order_relaxed); }
    void setGenre(uint8_t val) {
        const uint8_t normalized = (val <= GenreCatalog::kGenreCount) ? val : 0;
        genre.store(normalized, std::memory_order_relaxed);
    }
    uint8_t getGenre() const {
        return genre.load(std::memory_order_relaxed);
    }
    void setProjectKeyRoot(uint8_t val) { projectKeyRoot.store(static_cast<uint8_t>(val % 12), std::memory_order_relaxed); }
    void setProjectKeyMode(uint8_t val) {
        const uint8_t normalized = (val == kProjectKeyModeMajor)
            ? kProjectKeyModeMajor
            : kProjectKeyModeNaturalMinor;
        projectKeyMode.store(normalized, std::memory_order_relaxed);
    }
    void setProjectKey(uint8_t root, uint8_t mode) {
        setProjectKeyRoot(root);
        setProjectKeyMode(mode);
    }
    void setArrangementSectionCount(uint8_t val) {
        const uint8_t normalized = std::clamp<uint8_t>(val, 1, kArrangementMaxSections);
        arrangementSectionCount.store(normalized, std::memory_order_relaxed);
        const uint8_t currentSection = arrangementCurrentSection.load(std::memory_order_relaxed);
        if (currentSection >= normalized) {
            arrangementCurrentSection.store(static_cast<uint8_t>(normalized - 1), std::memory_order_relaxed);
        }
    }
    void setArrangementCurrentSection(uint8_t val) {
        const uint8_t sectionCount = arrangementSectionCount.load(std::memory_order_relaxed);
        const uint8_t maxSectionIndex = static_cast<uint8_t>(std::max<uint8_t>(sectionCount, 1) - 1);
        arrangementCurrentSection.store(static_cast<uint8_t>(std::min<uint8_t>(val, maxSectionIndex)), std::memory_order_relaxed);
    }
    void setArrangementMode(uint8_t val) {
        const uint8_t normalized = (val <= kArrangementModeMixed) ? val : kArrangementModeMixed;
        arrangementMode.store(normalized, std::memory_order_relaxed);
    }
    void setArrangementSectionLength(uint8_t sectionIndex, uint8_t bars) {
        if (sectionIndex < kArrangementMaxSections) {
            const uint8_t normalizedBars = std::clamp<uint8_t>(bars,
                                                               kArrangementMinSectionLengthBars,
                                                               kArrangementMaxSectionLengthBars);
            arrangementSectionLengths[sectionIndex].store(normalizedBars, std::memory_order_relaxed);
        }
    }
    uint8_t getArrangementSectionLength(uint8_t sectionIndex) const {
        if (sectionIndex < kArrangementMaxSections) {
            const uint8_t value = arrangementSectionLengths[sectionIndex].load(std::memory_order_relaxed);
            return std::clamp<uint8_t>(value,
                                       kArrangementMinSectionLengthBars,
                                       kArrangementMaxSectionLengthBars);
        }
        return kArrangementDefaultSectionLengthBars;
    }
    void setArrangementSectionProgression(uint8_t sectionIndex, uint8_t progressionId) {
        if (sectionIndex < kArrangementMaxSections) {
            arrangementSectionProgressions[sectionIndex].store(progressionId, std::memory_order_relaxed);
        }
    }
    uint8_t getArrangementSectionProgression(uint8_t sectionIndex) const {
        if (sectionIndex < kArrangementMaxSections) {
            return arrangementSectionProgressions[sectionIndex].load(std::memory_order_relaxed);
        }
        return kArrangementProgressionFollowGlobal;
    }
    void setArrangementSectionTrackMask(uint8_t sectionIndex, uint8_t trackMask) {
        if (sectionIndex < kArrangementMaxSections) {
            arrangementSectionTrackMasks[sectionIndex].store(static_cast<uint8_t>(trackMask & kArrangementTrackMaskAll),
                                                             std::memory_order_relaxed);
        }
    }
    uint8_t getArrangementSectionTrackMask(uint8_t sectionIndex) const {
        if (sectionIndex < kArrangementMaxSections) {
            return static_cast<uint8_t>(arrangementSectionTrackMasks[sectionIndex].load(std::memory_order_relaxed)
                & kArrangementTrackMaskAll);
        }
        return kArrangementTrackMaskAll;
    }
    void setArrangementSectionParametersEnabled(bool val) {
        arrangementSectionParametersEnabled.store(val, std::memory_order_relaxed);
    }
    void setArrangementSectionTrackParameter(uint8_t sectionIndex, uint8_t trackIndex, uint8_t parameterIndex, float value) {
        if (sectionIndex < kArrangementMaxSections && trackIndex < kTrackCount && parameterIndex < kArrangementTrackParameterCount) {
            arrangementSectionTrackParameters[sectionIndex][trackIndex][parameterIndex].store(std::clamp(value, 0.0f, 1.0f),
                                                                                              std::memory_order_relaxed);
        }
    }
    float getArrangementSectionTrackParameter(uint8_t sectionIndex, uint8_t trackIndex, uint8_t parameterIndex) const {
        if (sectionIndex < kArrangementMaxSections && trackIndex < kTrackCount && parameterIndex < kArrangementTrackParameterCount) {
            return std::clamp(arrangementSectionTrackParameters[sectionIndex][trackIndex][parameterIndex].load(std::memory_order_relaxed),
                              0.0f,
                              1.0f);
        }
        return 0.5f;
    }
    void setArrangementChainEnabled(bool val) {
        arrangementChainEnabled.store(val, std::memory_order_relaxed);
    }
    void setArrangementChainLength(uint8_t val) {
        const uint8_t normalized = std::clamp<uint8_t>(val, 1, kArrangementMaxSections);
        arrangementChainLength.store(normalized, std::memory_order_relaxed);
    }
    uint8_t getArrangementChainLength() const {
        const uint8_t value = arrangementChainLength.load(std::memory_order_relaxed);
        return std::clamp<uint8_t>(value, 1, kArrangementMaxSections);
    }
    void setArrangementChainStep(uint8_t chainIndex, uint8_t sectionIndex) {
        if (chainIndex < kArrangementMaxSections) {
            arrangementChainSequence[chainIndex].store(static_cast<uint8_t>(std::min<uint8_t>(sectionIndex,
                                                                                                 static_cast<uint8_t>(kArrangementMaxSections - 1))),
                                                       std::memory_order_relaxed);
        }
    }
    uint8_t getArrangementChainStep(uint8_t chainIndex) const {
        if (chainIndex < kArrangementMaxSections) {
            const uint8_t section = arrangementChainSequence[chainIndex].load(std::memory_order_relaxed);
            return static_cast<uint8_t>(std::min<uint8_t>(section,
                                                          static_cast<uint8_t>(kArrangementMaxSections - 1)));
        }
        return 0;
    }
    void setActiveSpotEffects(uint8_t val) { activeSpotEffects.store(val, std::memory_order_relaxed); }

    // --- Groove setters/getters ---
    void setSwingAmount(float val) { swingAmount.store(std::clamp(val, 0.0f, 1.0f), std::memory_order_relaxed); }
    float getSwingAmount() const { return swingAmount.load(std::memory_order_relaxed); }
    void setVelocityHumanize(float val) { velocityHumanize.store(std::clamp(val, 0.0f, 1.0f), std::memory_order_relaxed); }
    float getVelocityHumanize() const { return velocityHumanize.load(std::memory_order_relaxed); }
    void setTimingJitter(float val) { timingJitter.store(std::clamp(val, 0.0f, 1.0f), std::memory_order_relaxed); }
    float getTimingJitter() const { return timingJitter.load(std::memory_order_relaxed); }
};
