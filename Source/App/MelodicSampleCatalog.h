#pragma once

#include "../Audio/Synths/MelodicSampler.h"
#include "SynthCatalog.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace MelodicSampleCatalog {

enum class InstrumentId : uint8_t {
    None = 0,
    Spectrum,
    GrandPiano,
    FluteC3,
};

struct RegionDefinition {
    std::string_view resourceName;
    std::string_view displayName;
    int rootNote = 60;
    int lowNote = 0;
    int highNote = 127;
    float gain = 1.0f;
    float tuneSemitones = 0.0f;
    float startOffset = 0.0f;
    float endOffset = 1.0f;
    bool loop = false;
    float loopStart = 0.0f;
    float loopEnd = 1.0f;
};

struct InstrumentDefinition {
    InstrumentId id = InstrumentId::None;
    std::string_view name;
    std::array<RegionDefinition, MelodicSamplerEngine::RegionCount> regions{};
};

inline constexpr int note(int octave, int semitone) {
    return (octave + 1) * 12 + semitone;
}

inline constexpr RegionDefinition emptyRegion() {
    return RegionDefinition{};
}

inline constexpr InstrumentDefinition kSpectrum{InstrumentId::Spectrum, "Spectrum", {}};
inline constexpr InstrumentDefinition kGrandPiano{InstrumentId::GrandPiano, "Grand Piano", {}};
inline constexpr InstrumentDefinition kFluteC3{InstrumentId::FluteC3, "Flute C3", {}};

inline constexpr std::array<InstrumentDefinition, 6> kImportedBassInstruments{{
    {
        InstrumentId::Spectrum,
        "808",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_808_c0_ogg", "808 C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_dsharp0_ogg", "808 D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_f0_ogg", "808 F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_a0_ogg", "808 A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_c1_ogg", "808 C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_dsharp1_ogg", "808 D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_f1_ogg", "808 F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_808_a1_ogg", "808 A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
    {
        InstrumentId::Spectrum,
        "Buzz",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_buzz_c0_ogg", "Buzz C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_dsharp0_ogg", "Buzz D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_f0_ogg", "Buzz F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_a0_ogg", "Buzz A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_c1_ogg", "Buzz C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_dsharp1_ogg", "Buzz D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_f1_ogg", "Buzz F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_buzz_a1_ogg", "Buzz A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
    {
        InstrumentId::Spectrum,
        "FM Wow",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_fm_wow_c0_ogg", "FM Wow C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_dsharp0_ogg", "FM Wow D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_f0_ogg", "FM Wow F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_a0_ogg", "FM Wow A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_c1_ogg", "FM Wow C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_dsharp1_ogg", "FM Wow D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_f1_ogg", "FM Wow F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_fm_wow_a1_ogg", "FM Wow A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
    {
        InstrumentId::Spectrum,
        "Hard",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_hard_c0_ogg", "Hard C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_dsharp0_ogg", "Hard D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_f0_ogg", "Hard F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_a0_ogg", "Hard A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_c1_ogg", "Hard C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_dsharp1_ogg", "Hard D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_f1_ogg", "Hard F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_hard_a1_ogg", "Hard A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
    {
        InstrumentId::Spectrum,
        "Redux",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_redux_c0_ogg", "Redux C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_dsharp0_ogg", "Redux D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_f0_ogg", "Redux F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_a0_ogg", "Redux A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_c1_ogg", "Redux C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_dsharp1_ogg", "Redux D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_f1_ogg", "Redux F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_redux_a1_ogg", "Redux A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
    {
        InstrumentId::Spectrum,
        "Tonal",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"bass_tonal_c0_ogg", "Tonal C0", note(0, 0), 0, 13, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_dsharp0_ogg", "Tonal D#0", note(0, 3), 14, 16, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_f0_ogg", "Tonal F0", note(0, 5), 17, 19, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_a0_ogg", "Tonal A0", note(0, 9), 20, 22, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_c1_ogg", "Tonal C1", note(1, 0), 23, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_dsharp1_ogg", "Tonal D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_f1_ogg", "Tonal F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"bass_tonal_a1_ogg", "Tonal A1", note(1, 9), 32, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
            emptyRegion(),
        }}
    },
}};
inline constexpr std::array<InstrumentDefinition, 6> kImportedChordInstruments{{
    {
        InstrumentId::GrandPiano,
        "Analog",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_analog_c1_ogg", "Analog C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_dsharp1_ogg", "Analog D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_f1_ogg", "Analog F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_a1_ogg", "Analog A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_c2_ogg", "Analog C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_dsharp2_ogg", "Analog D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_f2_ogg", "Analog F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_a2_ogg", "Analog A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_c3_ogg", "Analog C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_dsharp3_ogg", "Analog D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_f3_ogg", "Analog F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_analog_a3_ogg", "Analog A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::GrandPiano,
        "Cobra",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_cobra_c1_ogg", "Cobra C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_dsharp1_ogg", "Cobra D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_f1_ogg", "Cobra F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_a1_ogg", "Cobra A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_c2_ogg", "Cobra C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_dsharp2_ogg", "Cobra D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_f2_ogg", "Cobra F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_a2_ogg", "Cobra A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_c3_ogg", "Cobra C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_dsharp3_ogg", "Cobra D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_f3_ogg", "Cobra F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_cobra_a3_ogg", "Cobra A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::GrandPiano,
        "FM Piano",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_fm_piano_c1_ogg", "FM Piano C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_dsharp1_ogg", "FM Piano D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_f1_ogg", "FM Piano F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_a1_ogg", "FM Piano A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_c2_ogg", "FM Piano C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_dsharp2_ogg", "FM Piano D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_f2_ogg", "FM Piano F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_a2_ogg", "FM Piano A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_c3_ogg", "FM Piano C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_dsharp3_ogg", "FM Piano D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_f3_ogg", "FM Piano F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_fm_piano_a3_ogg", "FM Piano A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::GrandPiano,
        "Retro Tube",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_retro_tube_c1_ogg", "Retro Tube C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_dsharp1_ogg", "Retro Tube D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_f1_ogg", "Retro Tube F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_a1_ogg", "Retro Tube A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_c2_ogg", "Retro Tube C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_dsharp2_ogg", "Retro Tube D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_f2_ogg", "Retro Tube F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_a2_ogg", "Retro Tube A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_c3_ogg", "Retro Tube C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_dsharp3_ogg", "Retro Tube D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_f3_ogg", "Retro Tube F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_retro_tube_a3_ogg", "Retro Tube A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::GrandPiano,
        "Sine",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_sine_c1_ogg", "Sine C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_dsharp1_ogg", "Sine D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_f1_ogg", "Sine F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_a1_ogg", "Sine A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_c2_ogg", "Sine C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_dsharp2_ogg", "Sine D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_f2_ogg", "Sine F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_a2_ogg", "Sine A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_c3_ogg", "Sine C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_dsharp3_ogg", "Sine D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_f3_ogg", "Sine F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_sine_a3_ogg", "Sine A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::GrandPiano,
        "Supersaw",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"chords_supersaw_c1_ogg", "Supersaw C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_dsharp1_ogg", "Supersaw D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_f1_ogg", "Supersaw F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_a1_ogg", "Supersaw A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_c2_ogg", "Supersaw C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_dsharp2_ogg", "Supersaw D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_f2_ogg", "Supersaw F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_a2_ogg", "Supersaw A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_c3_ogg", "Supersaw C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_dsharp3_ogg", "Supersaw D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_f3_ogg", "Supersaw F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"chords_supersaw_a3_ogg", "Supersaw A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
}};
inline constexpr std::array<InstrumentDefinition, 6> kImportedLeadInstruments{{
    {
        InstrumentId::FluteC3,
        "Bell Pluck",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_bell_pluck_c1_ogg", "Bell Pluck C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_dsharp1_ogg", "Bell Pluck D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_f1_ogg", "Bell Pluck F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_a1_ogg", "Bell Pluck A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_c2_ogg", "Bell Pluck C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_dsharp2_ogg", "Bell Pluck D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_f2_ogg", "Bell Pluck F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_a2_ogg", "Bell Pluck A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_c3_ogg", "Bell Pluck C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_dsharp3_ogg", "Bell Pluck D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_f3_ogg", "Bell Pluck F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bell_pluck_a3_ogg", "Bell Pluck A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::FluteC3,
        "Bottle String",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_bottle_string_c1_ogg", "Bottle String C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_dsharp1_ogg", "Bottle String D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_f1_ogg", "Bottle String F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_a1_ogg", "Bottle String A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_c2_ogg", "Bottle String C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_dsharp2_ogg", "Bottle String D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_f2_ogg", "Bottle String F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_a2_ogg", "Bottle String A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_c3_ogg", "Bottle String C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_dsharp3_ogg", "Bottle String D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_f3_ogg", "Bottle String F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_bottle_string_a3_ogg", "Bottle String A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::FluteC3,
        "FM Squares",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_fm_squares_c1_ogg", "FM Squares C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_dsharp1_ogg", "FM Squares D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_f1_ogg", "FM Squares F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_a1_ogg", "FM Squares A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_c2_ogg", "FM Squares C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_dsharp2_ogg", "FM Squares D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_f2_ogg", "FM Squares F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_a2_ogg", "FM Squares A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_c3_ogg", "FM Squares C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_dsharp3_ogg", "FM Squares D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_f3_ogg", "FM Squares F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_fm_squares_a3_ogg", "FM Squares A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::FluteC3,
        "Rave Stab",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_rave_stab_c1_ogg", "Rave Stab C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_dsharp1_ogg", "Rave Stab D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_f1_ogg", "Rave Stab F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_a1_ogg", "Rave Stab A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_c2_ogg", "Rave Stab C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_dsharp2_ogg", "Rave Stab D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_f2_ogg", "Rave Stab F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_a2_ogg", "Rave Stab A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_c3_ogg", "Rave Stab C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_dsharp3_ogg", "Rave Stab D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_f3_ogg", "Rave Stab F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_rave_stab_a3_ogg", "Rave Stab A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::FluteC3,
        "Saw Pluck",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_saw_pluck_c1_ogg", "Saw Pluck C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_dsharp1_ogg", "Saw Pluck D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_f1_ogg", "Saw Pluck F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_a1_ogg", "Saw Pluck A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_c2_ogg", "Saw Pluck C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_dsharp2_ogg", "Saw Pluck D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_f2_ogg", "Saw Pluck F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_a2_ogg", "Saw Pluck A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_c3_ogg", "Saw Pluck C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_dsharp3_ogg", "Saw Pluck D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_f3_ogg", "Saw Pluck F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_saw_pluck_a3_ogg", "Saw Pluck A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
    {
        InstrumentId::FluteC3,
        "Tube Delay",
        std::array<RegionDefinition, MelodicSamplerEngine::RegionCount>{{
            {"lead_tube_delay_c1_ogg", "Tube Delay C1", note(1, 0), 0, 25, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_dsharp1_ogg", "Tube Delay D#1", note(1, 3), 26, 28, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_f1_ogg", "Tube Delay F1", note(1, 5), 29, 31, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_a1_ogg", "Tube Delay A1", note(1, 9), 32, 34, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_c2_ogg", "Tube Delay C2", note(2, 0), 35, 37, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_dsharp2_ogg", "Tube Delay D#2", note(2, 3), 38, 40, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_f2_ogg", "Tube Delay F2", note(2, 5), 41, 43, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_a2_ogg", "Tube Delay A2", note(2, 9), 44, 46, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_c3_ogg", "Tube Delay C3", note(3, 0), 47, 49, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_dsharp3_ogg", "Tube Delay D#3", note(3, 3), 50, 52, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_f3_ogg", "Tube Delay F3", note(3, 5), 53, 55, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
            {"lead_tube_delay_a3_ogg", "Tube Delay A3", note(3, 9), 56, 127, 0.70f, 0.0f, 0.0f, 1.0f, true, 0.18f, 0.82f},
        }}
    },
}};

inline constexpr const InstrumentDefinition& getInstrument(InstrumentId id) {
    switch (id) {
        case InstrumentId::Spectrum: return kSpectrum;
        case InstrumentId::GrandPiano: return kGrandPiano;
        case InstrumentId::FluteC3: return kFluteC3;
        case InstrumentId::None: break;
    }

    return kGrandPiano;
}

inline constexpr const InstrumentDefinition* getInstrumentForPreset(uint8_t trackIndex, uint8_t presetId) {
    if (!SynthCatalog::isMelodicSamplerPreset(trackIndex, presetId)) {
        return nullptr;
    }

    const uint8_t samplePresetIndex = static_cast<uint8_t>(presetId - SynthCatalog::kMelodicSamplerPresetStart);
    const uint8_t instrumentIndex = static_cast<uint8_t>(samplePresetIndex / 3);

    if (trackIndex == 1) {
        return instrumentIndex < kImportedBassInstruments.size()
            ? &kImportedBassInstruments[instrumentIndex]
            : nullptr;
    }

    if (trackIndex == 2) {
        return instrumentIndex < kImportedChordInstruments.size()
            ? &kImportedChordInstruments[instrumentIndex]
            : nullptr;
    }

    if (trackIndex == 3) {
        return instrumentIndex < kImportedLeadInstruments.size()
            ? &kImportedLeadInstruments[instrumentIndex]
            : nullptr;
    }

    return nullptr;
}

} // namespace MelodicSampleCatalog
