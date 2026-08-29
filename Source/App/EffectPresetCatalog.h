#pragma once

#include <array>
#include <cstdint>
#include <string_view>

// Named display IDs for built-in FX presets (1-based, matching kPresets array index + 1).
// Use these instead of raw fx(N) calls to make FX chains self-documenting.
namespace FxDisplayId {
    inline constexpr uint16_t Bypass = 1;
    inline constexpr uint16_t TapeBrakeShort = 2;
    inline constexpr uint16_t TapeBrakeLong = 3;
    inline constexpr uint16_t Stutter1_4Blend = 4;
    inline constexpr uint16_t Stutter1_8Tight = 5;
    inline constexpr uint16_t Stutter1_16Hard = 6;
    inline constexpr uint16_t HPSweepWarm = 7;
    inline constexpr uint16_t HPSweepModern = 8;
    inline constexpr uint16_t HPSweepRazor = 9;
    inline constexpr uint16_t RoomGlue = 10;
    inline constexpr uint16_t HallBloom = 11;
    inline constexpr uint16_t InfiniteWash = 12;
    inline constexpr uint16_t PlateBright = 13;
    inline constexpr uint16_t DarkChamber = 14;
    inline constexpr uint16_t Redux12bitClean = 15;
    inline constexpr uint16_t Redux8bitCrunch = 16;
    inline constexpr uint16_t Redux6bitGrit = 17;
    inline constexpr uint16_t EchoSlapback = 18;
    inline constexpr uint16_t EchoPingMid = 19;
    inline constexpr uint16_t EchoLongWide = 20;
    inline constexpr uint16_t EchoDubThrow = 21;
    inline constexpr uint16_t EchoMicroWide = 22;
    inline constexpr uint16_t SaturateWarm = 23;
    inline constexpr uint16_t SaturateDrive = 24;
    inline constexpr uint16_t SaturateFuzz = 25;
    inline constexpr uint16_t CompTight = 26;
    inline constexpr uint16_t CompGlue = 27;
    inline constexpr uint16_t CompSquash = 28;
    inline constexpr uint16_t LimiterTransparent = 29;
    inline constexpr uint16_t LimiterLoud = 30;
    inline constexpr uint16_t LimiterBrickwall = 31;
    inline constexpr uint16_t TransientSnap = 32;
    inline constexpr uint16_t TransientBody = 33;
    inline constexpr uint16_t TransientTight = 34;
    inline constexpr uint16_t ClipSoftGlue = 35;
    inline constexpr uint16_t ClipCrunch = 36;
    inline constexpr uint16_t ClipHardEdge = 37;
    inline constexpr uint16_t ClipBrickBite = 38;
    inline constexpr uint16_t FoldWarm = 39;
    inline constexpr uint16_t FoldMetallic = 40;
    inline constexpr uint16_t FoldAcid = 41;
    inline constexpr uint16_t FoldChaos = 42;
    inline constexpr uint16_t CombTight = 43;
    inline constexpr uint16_t CombHollow = 44;
    inline constexpr uint16_t CombResonant = 45;
    inline constexpr uint16_t CombMetallic = 46;
    inline constexpr uint16_t PanGentle = 47;
    inline constexpr uint16_t PanWideSync = 48;
    inline constexpr uint16_t PanChopper = 49;
    inline constexpr uint16_t PanHyper = 50;
    inline constexpr uint16_t AsymVelvet = 51;
    inline constexpr uint16_t AsymPunch = 52;
    inline constexpr uint16_t AsymSnarl = 53;
    inline constexpr uint16_t AsymScream = 54;
    inline constexpr uint16_t RingGentle = 55;
    inline constexpr uint16_t RingBell = 56;
    inline constexpr uint16_t RingRobot = 57;
    inline constexpr uint16_t RingHarsh = 58;
    inline constexpr uint16_t ChorusSoft = 59;
    inline constexpr uint16_t ChorusWide = 60;
    inline constexpr uint16_t ChorusEnsemble = 61;
    inline constexpr uint16_t ChorusWobble = 62;
    inline constexpr uint16_t PhaserSlow = 63;
    inline constexpr uint16_t PhaserSweep = 64;
    inline constexpr uint16_t PhaserDeep = 65;
    inline constexpr uint16_t PhaserPulse = 66;
    inline constexpr uint16_t FlangeSoft = 67;
    inline constexpr uint16_t FlangeJet = 68;
    inline constexpr uint16_t FlangeHollow = 69;
    inline constexpr uint16_t FlangeMetal = 70;
    inline constexpr uint16_t JitterSoft = 71;
    inline constexpr uint16_t JitterLoose = 72;
    inline constexpr uint16_t JitterBroken = 73;
    inline constexpr uint16_t JitterWarp = 74;
    inline constexpr uint16_t ErodeDust = 75;
    inline constexpr uint16_t ErodeCracks = 76;
    inline constexpr uint16_t ErodeCollapse = 77;
    inline constexpr uint16_t ErodeVoid = 78;
    inline constexpr uint16_t GatePump1_8 = 79;
    inline constexpr uint16_t GateChop1_16 = 80;
    inline constexpr uint16_t GateTight1_32 = 81;
    inline constexpr uint16_t GateSwing1_8 = 82;
    inline constexpr uint16_t DuckSoft = 83;
    inline constexpr uint16_t DuckClub = 84;
    inline constexpr uint16_t DuckHard = 85;
    inline constexpr uint16_t DuckHyper = 86;
    inline constexpr uint16_t RepeatBlend1_8 = 87;
    inline constexpr uint16_t RepeatTight1_16 = 88;
    inline constexpr uint16_t RepeatHard1_16 = 89;
    inline constexpr uint16_t RepeatMelt1_4 = 90;
    inline constexpr uint16_t FreqShiftTightPos = 91;
    inline constexpr uint16_t FreqShiftBellPos = 92;
    inline constexpr uint16_t FreqShiftSwirlNeg = 93;
    inline constexpr uint16_t FreqShiftWidePosPos = 94;
    inline constexpr uint16_t PitchOctDown = 95;
    inline constexpr uint16_t PitchFifthUp = 96;
    inline constexpr uint16_t PitchMicroDetune = 97;
    inline constexpr uint16_t PitchOctUp = 98;
    inline constexpr uint16_t HarmonyFifth = 99;
    inline constexpr uint16_t HarmonyOctave = 100;
    inline constexpr uint16_t HarmonyDown = 101;
    inline constexpr uint16_t HarmonyStack = 102;
    inline constexpr uint16_t FreezeMicroLoop = 103;
    inline constexpr uint16_t FreezeHalfSpeed = 104;
    inline constexpr uint16_t FreezeGlass = 105;
    inline constexpr uint16_t FreezeDrone = 106;
    inline constexpr uint16_t GrainEcho = 107;
    inline constexpr uint16_t GrainScatter = 108;
    inline constexpr uint16_t GrainReverseIsh = 109;
    inline constexpr uint16_t GrainCloud = 110;
    inline constexpr uint16_t ResonatorBody = 111;
    inline constexpr uint16_t ResonatorString = 112;
    inline constexpr uint16_t ResonatorGlass = 113;
    inline constexpr uint16_t ResonatorTube = 114;
    inline constexpr uint16_t EQHPTight = 115;
    inline constexpr uint16_t EQLPSmooth = 116;
    inline constexpr uint16_t EQBPVocal = 117;
    inline constexpr uint16_t EQNotchHum = 118;
    inline constexpr uint16_t EQBellBodyPlus = 119;
    inline constexpr uint16_t EQBellAirMinus = 120;
    inline constexpr uint16_t EQDrumClean = 121;
    inline constexpr uint16_t EQTelephone = 122;
    inline constexpr uint16_t FormantA = 123;
    inline constexpr uint16_t FormantE = 124;
    inline constexpr uint16_t FormantI = 125;
    inline constexpr uint16_t FormantSweep = 126;
    inline constexpr uint16_t OTTGentle = 127;
    inline constexpr uint16_t OTTStandard = 128;
    inline constexpr uint16_t OTTAggressive = 129;
    inline constexpr uint16_t OTTMastering = 130;
    inline constexpr uint16_t OTTPunch = 131;
    inline constexpr uint16_t IRSmallLiveRoom = 132;
    inline constexpr uint16_t IRAttic = 133;
    inline constexpr uint16_t IRConcertHall = 134;
    inline constexpr uint16_t IRCathedral = 135;
    inline constexpr uint16_t IRParkingHall = 136;
    inline constexpr uint16_t IRSteelTunnel = 137;
    inline constexpr uint16_t IRUnderpass = 138;
    inline constexpr uint16_t IRHotelShower = 139;
    inline constexpr uint16_t IROutdoorCourtyard = 140;
    inline constexpr uint16_t IRReverseDigital = 141;
    inline constexpr uint16_t TapeSlapback = 142;
    inline constexpr uint16_t TapeEcho = 143;
    inline constexpr uint16_t TapeDub = 144;
    inline constexpr uint16_t TapeWarble = 145;
    inline constexpr uint16_t TapeLong = 146;
    inline constexpr uint16_t PingPongShort = 147;
    inline constexpr uint16_t PingPongMid = 148;
    inline constexpr uint16_t PingPongLong = 149;
    inline constexpr uint16_t PingPongWide = 150;
    inline constexpr uint16_t PingPongTight = 151;
    inline constexpr uint16_t CloudShimmer = 152;
    inline constexpr uint16_t CloudTexture = 153;
    inline constexpr uint16_t CloudDrone = 154;
    inline constexpr uint16_t CloudRhythmic = 155;
    inline constexpr uint16_t CloudWash = 156;
    inline constexpr uint16_t SpectralBlurSoft = 157;
    inline constexpr uint16_t SpectralBlurMedium = 158;
    inline constexpr uint16_t SpectralBlurHeavy = 159;
    inline constexpr uint16_t SpectralBlurSubtle = 160;
    inline constexpr uint16_t SpectralBlurFull = 161;
    inline constexpr uint16_t SpectralDelayShort = 162;
    inline constexpr uint16_t SpectralDelayMid = 163;
    inline constexpr uint16_t SpectralDelayLong = 164;
    inline constexpr uint16_t SpectralDelayFeedback = 165;
    inline constexpr uint16_t SpectralDelayWash = 166;
    inline constexpr uint16_t DrumTightRoom = 191;
    inline constexpr uint16_t DrumSoftPlate = 192;
    inline constexpr uint16_t DrumDarkRoom = 193;
}

namespace EffectPresetCatalog {

enum class EffectType : uint8_t {
    None = 0,
    TapeStop,
    BeatRepeat,
    HighPassSweep,
    ReverbWash,
    ReduxCrush,
    DelayEcho,
    SaturationWaveshaper,
    SoftHardClip,
    Wavefolder,
    AsymShaper,
    CompressorGlue,
    PeakLimiter,
    TransientShaper,
    CombFilter,
    MultiModeEQ,
    FormantFilter,
    Autopan,
    RingModulator,
    Chorus,
    Phaser,
    Flanger,
    JitterDegrade,
    ErosionDegrade,
    TranceGate,
    SidechainDucker,
    BeatRepeatInsert,
    FrequencyShifter,
    PitchShifter,
    Harmonizer,
    TimeFreezer,
    GrainDelay,
    PhysicalModelingResonator,
    MultibandOtt,
    ConvolutionReverb,
    TapeDelay,
    PingPongDelay,
    CloudGenerator,
    SpectralBlur,
    SpectralDelay,
    CompositeCategory,
};

struct EffectPreset {
    std::string_view name;
    EffectType type;
    float paramA;
    float paramB;
    float paramC;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
    std::string_view irResourceName = {};
};

inline constexpr std::string_view kDefaultConvolutionResourceName = "small_live_room_wav";

enum class CompositeRouting : uint8_t {
    Serial = 0,
    Parallel,
};

inline constexpr uint8_t kCompositeMaxComponents = 3;
inline constexpr uint16_t kCompositePresetIdBase = 1024;

struct CompositeComponent {
    EffectType type;
    float paramA;
    float paramB;
    float paramC;
    float level;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
};

struct CompositePreset {
    std::string_view name;
    uint8_t categoryDigit;
    CompositeRouting routing;
    uint8_t componentCount;
    std::array<CompositeComponent, kCompositeMaxComponents> components;
};

// Preset IDs are zero-based internally and one-based in the UI.
inline constexpr std::array<EffectPreset, 166> kPresets{{
    {"Bypass", EffectType::None, 0.0f, 0.0f, 0.0f},
    {"Tape Brake Short", EffectType::TapeStop, 0.25f, 0.0f, 0.0f},
    {"Tape Brake Long", EffectType::TapeStop, 0.75f, 0.0f, 0.0f},
    {"Stutter 1/4 Blend", EffectType::BeatRepeat, 0.25f, 0.60f, 0.0f},
    {"Stutter 1/8 Tight", EffectType::BeatRepeat, 0.125f, 0.80f, 0.0f},
    {"Stutter 1/16 Hard", EffectType::BeatRepeat, 0.0625f, 1.0f, 0.0f},
    {"HP Sweep Warm", EffectType::HighPassSweep, 1200.0f, 0.02f, 0.0f},
    {"HP Sweep Modern", EffectType::HighPassSweep, 2800.0f, 0.05f, 0.0f},
    {"HP Sweep Razor", EffectType::HighPassSweep, 7000.0f, 0.09f, 0.0f},
    {"Room Glue", EffectType::ReverbWash, 0.35f, 0.45f, 0.55f},
    {"Hall Bloom", EffectType::ReverbWash, 0.55f, 0.75f, 0.35f},
    {"Infinite Wash", EffectType::ReverbWash, 0.65f, 0.80f, 0.20f},
    {"Plate Bright", EffectType::ReverbWash, 0.42f, 0.58f, 0.18f},
    {"Dark Chamber", EffectType::ReverbWash, 0.45f, 0.55f, 0.60f},
    {"Redux 12-bit Clean", EffectType::ReduxCrush, 12.0f, 3.0f, 0.28f},
    {"Redux 8-bit Crunch", EffectType::ReduxCrush, 8.0f, 6.0f, 0.38f},
    {"Redux 6-bit Grit", EffectType::ReduxCrush, 6.0f, 10.0f, 0.48f},
    {"Echo Slapback", EffectType::DelayEcho, 95.0f, 0.18f, 0.22f},
    {"Echo Ping Mid", EffectType::DelayEcho, 280.0f, 0.38f, 0.30f},
    {"Echo Long Wide", EffectType::DelayEcho, 520.0f, 0.58f, 0.42f},
    {"Echo Dub Throw", EffectType::DelayEcho, 640.0f, 0.72f, 0.48f},
    {"Echo Micro Wide", EffectType::DelayEcho, 45.0f, 0.12f, 0.35f},
    {"Saturate Warm", EffectType::SaturationWaveshaper, 2.3f, 0.42f, 0.0f, 0.0f, -3.0f},
    {"Saturate Drive", EffectType::SaturationWaveshaper, 4.5f, 0.62f, 0.0f, 0.0f, -5.0f},
    {"Saturate Fuzz", EffectType::SaturationWaveshaper, 8.5f, 0.80f, 0.0f, 0.0f, -9.5f},
    {"Comp Tight", EffectType::CompressorGlue, -16.0f, 3.0f, 0.0f},
    {"Comp Glue", EffectType::CompressorGlue, -22.0f, 4.0f, 0.5f},
    {"Comp Squash", EffectType::CompressorGlue, -28.0f, 8.0f, 1.0f},
    {"Limiter Transparent", EffectType::PeakLimiter, -1.0f, 90.0f, 0.0f},
    {"Limiter Loud", EffectType::PeakLimiter, -0.6f, 45.0f, 0.0f},
    {"Limiter Brickwall", EffectType::PeakLimiter, -0.3f, 25.0f, 0.0f},
    {"Transient Snap", EffectType::TransientShaper, 0.55f, -0.10f, 0.75f},
    {"Transient Body", EffectType::TransientShaper, -0.20f, 0.55f, 0.75f},
    {"Transient Tight", EffectType::TransientShaper, 0.35f, -0.45f, 0.70f},
    {"Clip Soft Glue", EffectType::SoftHardClip, 2.2f, 0.15f, 0.40f, 0.0f, -2.0f},
    {"Clip Crunch", EffectType::SoftHardClip, 4.0f, 0.45f, 0.45f, 0.0f, -4.5f},
    {"Clip Hard Edge", EffectType::SoftHardClip, 6.0f, 0.8f, 0.50f, 0.0f, -6.5f},
    {"Clip Brick Bite", EffectType::SoftHardClip, 9.0f, 1.0f, 0.55f, 0.0f, -8.0f},
    {"Fold Warm", EffectType::Wavefolder, 1.6f, 0.20f, 0.24f, 0.0f, -3.0f},
    {"Fold Metallic", EffectType::Wavefolder, 2.6f, 0.40f, 0.28f, 0.0f, -6.0f},
    {"Fold Acid", EffectType::Wavefolder, 3.8f, 0.55f, 0.32f, 0.0f, -8.5f},
    {"Fold Chaos", EffectType::Wavefolder, 5.2f, 0.72f, 0.36f, 0.0f, -10.0f},
    {"Comb Tight", EffectType::CombFilter, 8.0f, 0.45f, 0.30f},
    {"Comb Hollow", EffectType::CombFilter, 14.0f, 0.62f, 0.40f},
    {"Comb Resonant", EffectType::CombFilter, 24.0f, 0.78f, 0.52f},
    {"Comb Metallic", EffectType::CombFilter, 38.0f, -0.7f, 0.55f},
    {"Pan Gentle", EffectType::Autopan, 0.45f, 0.45f, 0.60f},
    {"Pan Wide Sync", EffectType::Autopan, 0.75f, 0.70f, 1.0f},
    {"Pan Chopper", EffectType::Autopan, 1.80f, 0.85f, 1.0f},
    {"Pan Hyper", EffectType::Autopan, 3.50f, 1.0f, 0.90f},
    {"Asym Velvet", EffectType::AsymShaper, 2.4f, 0.20f, 0.38f, 0.0f, -2.5f},
    {"Asym Punch", EffectType::AsymShaper, 4.3f, 0.42f, 0.46f, 0.0f, -5.0f},
    {"Asym Snarl", EffectType::AsymShaper, 6.8f, 0.65f, 0.52f, 0.0f, -7.0f},
    {"Asym Scream", EffectType::AsymShaper, 9.5f, 0.88f, 0.58f, 0.0f, -6.0f},
    {"Ring Gentle", EffectType::RingModulator, 1.5f, 0.28f, 0.26f},
    {"Ring Bell", EffectType::RingModulator, 12.0f, 0.48f, 0.38f},
    {"Ring Robot", EffectType::RingModulator, 45.0f, 0.62f, 0.50f},
    {"Ring Harsh", EffectType::RingModulator, 140.0f, 0.78f, 0.62f},
    {"Chorus Soft", EffectType::Chorus, 0.35f, 6.0f, 0.30f},
    {"Chorus Wide", EffectType::Chorus, 0.55f, 11.0f, 0.38f},
    {"Chorus Ensemble", EffectType::Chorus, 0.85f, 15.0f, 0.48f},
    {"Chorus Wobble", EffectType::Chorus, 1.60f, 18.0f, 0.55f},
    {"Phaser Slow", EffectType::Phaser, 0.22f, 0.45f, 0.34f},
    {"Phaser Sweep", EffectType::Phaser, 0.55f, 0.62f, 0.45f},
    {"Phaser Deep", EffectType::Phaser, 0.90f, 0.74f, 0.52f},
    {"Phaser Pulse", EffectType::Phaser, 1.45f, 0.82f, 0.55f},
    {"Flange Soft", EffectType::Flanger, 0.18f, 0.18f, 0.30f},
    {"Flange Jet", EffectType::Flanger, 0.35f, 0.42f, 0.44f},
    {"Flange Hollow", EffectType::Flanger, 0.65f, -0.42f, 0.46f},
    {"Flange Metal", EffectType::Flanger, 1.05f, 0.62f, 0.55f},
    {"Jitter Soft", EffectType::JitterDegrade, 7.0f, 2.0f, 0.20f},
    {"Jitter Loose", EffectType::JitterDegrade, 12.0f, 4.5f, 0.29f},
    {"Jitter Broken", EffectType::JitterDegrade, 14.0f, 6.0f, 0.38f},
    {"Jitter Warp", EffectType::JitterDegrade, 18.0f, 9.0f, 0.47f},
    {"Erode Dust", EffectType::ErosionDegrade, 0.12f, 5.0f, 0.26f},
    {"Erode Cracks", EffectType::ErosionDegrade, 0.24f, 9.0f, 0.36f},
    {"Erode Collapse", EffectType::ErosionDegrade, 0.44f, 13.0f, 0.50f},
    {"Erode Void", EffectType::ErosionDegrade, 0.62f, 18.0f, 0.62f},
    {"Gate Pump 1/8", EffectType::TranceGate, 0.125f, 0.75f, 0.50f},
    {"Gate Chop 1/16", EffectType::TranceGate, 0.0625f, 0.90f, 0.45f},
    {"Gate Tight 1/32", EffectType::TranceGate, 0.0625f, 0.85f, 0.35f},
    {"Gate Swing 1/8", EffectType::TranceGate, 0.125f, 0.65f, 0.62f},
    {"Duck Soft", EffectType::SidechainDucker, 0.25f, 0.45f, 0.25f},
    {"Duck Club", EffectType::SidechainDucker, 0.25f, 0.72f, 0.50f},
    {"Duck Hard", EffectType::SidechainDucker, 0.125f, 0.90f, 0.72f},
    {"Duck Hyper", EffectType::SidechainDucker, 0.15f, 0.80f, 0.65f},
    {"Repeat Blend 1/8", EffectType::BeatRepeatInsert, 0.125f, 0.55f, 0.20f},
    {"Repeat Tight 1/16", EffectType::BeatRepeatInsert, 0.0625f, 0.72f, 0.28f},
    {"Repeat Hard 1/16", EffectType::BeatRepeatInsert, 0.0625f, 0.70f, 0.40f},
    {"Repeat Melt 1/4", EffectType::BeatRepeatInsert, 0.25f, 0.60f, 0.35f},
    {"Freq Shift Tight +", EffectType::FrequencyShifter, 80.0f, 0.42f, 0.15f},
    {"Freq Shift Bell +", EffectType::FrequencyShifter, 240.0f, 0.62f, 0.30f},
    {"Freq Shift Swirl -", EffectType::FrequencyShifter, -180.0f, 0.58f, 0.75f},
    {"Freq Shift Wide ++", EffectType::FrequencyShifter, 520.0f, 0.82f, 1.0f},
    {"Pitch Oct Down", EffectType::PitchShifter, -12.0f, 90.0f, 0.62f},
    {"Pitch Fifth Up", EffectType::PitchShifter, 7.0f, 70.0f, 0.48f},
    {"Pitch Micro Detune", EffectType::PitchShifter, 0.35f, 120.0f, 0.38f},
    {"Pitch Oct Up", EffectType::PitchShifter, 12.0f, 55.0f, 0.50f},
    {"Harmony Fifth", EffectType::Harmonizer, 7.0f, 0.25f, 0.42f},
    {"Harmony Octave", EffectType::Harmonizer, 12.0f, 0.45f, 0.48f},
    {"Harmony Down", EffectType::Harmonizer, -5.0f, 0.35f, 0.44f},
    {"Harmony Stack", EffectType::Harmonizer, 4.0f, 0.70f, 0.55f},
    {"Freeze Micro Loop", EffectType::TimeFreezer, 120.0f, 1.0f, 0.55f},
    {"Freeze Half Speed", EffectType::TimeFreezer, 360.0f, 0.5f, 0.72f},
    {"Freeze Glass", EffectType::TimeFreezer, 180.0f, 1.4f, 0.68f},
    {"Freeze Drone", EffectType::TimeFreezer, 600.0f, 0.75f, 0.65f},
    {"Grain Echo", EffectType::GrainDelay, 140.0f, 0.25f, 0.45f},
    {"Grain Scatter", EffectType::GrainDelay, 90.0f, 0.72f, 0.58f},
    {"Grain Reverse-ish", EffectType::GrainDelay, 220.0f, 0.85f, 0.62f},
    {"Grain Cloud", EffectType::GrainDelay, 300.0f, 0.75f, 0.70f},
    {"Resonator Body", EffectType::PhysicalModelingResonator, 160.0f, 0.72f, 0.45f},
    {"Resonator String", EffectType::PhysicalModelingResonator, 280.0f, 0.82f, 0.55f},
    {"Resonator Glass", EffectType::PhysicalModelingResonator, 620.0f, 0.88f, 0.68f},
    {"Resonator Tube", EffectType::PhysicalModelingResonator, 980.0f, 0.93f, 0.75f},
    {"EQ HP Tight", EffectType::MultiModeEQ, 850.0f, 0.90f, 0.00f},
    {"EQ LP Smooth", EffectType::MultiModeEQ, 4200.0f, 0.80f, 0.30f},
    {"EQ BP Vocal", EffectType::MultiModeEQ, 1800.0f, 1.40f, 0.50f},
    {"EQ Notch Hum", EffectType::MultiModeEQ, 420.0f, 3.00f, 0.70f},
    {"EQ Bell Body +", EffectType::MultiModeEQ, 240.0f, 6.0f, 1.0f},
    {"EQ Bell Air -", EffectType::MultiModeEQ, 5200.0f, -7.0f, 1.0f},
    {"EQ Drum Clean", EffectType::MultiModeEQ, 120.0f, 0.90f, 0.00f},
    {"EQ Telephone", EffectType::MultiModeEQ, 1200.0f, 1.10f, 0.50f},
    {"Formant A", EffectType::FormantFilter, 0.00f, 0.62f, 0.58f},
    {"Formant E", EffectType::FormantFilter, 0.34f, 0.72f, 0.62f},
    {"Formant I", EffectType::FormantFilter, 0.67f, 0.78f, 0.66f},
    {"Formant Sweep", EffectType::FormantFilter, 1.00f, 0.86f, 0.72f},

    // === Multiband OTT (Category 1 - Dynamics) ===
    {"OTT Gentle", EffectType::MultibandOtt, -20.0f, 2.2f, 2.2f},
    {"OTT Standard", EffectType::MultibandOtt, -16.0f, 3.0f, 3.0f},
    {"OTT Aggressive", EffectType::MultibandOtt, -12.0f, 4.5f, 4.5f},
    {"OTT Mastering", EffectType::MultibandOtt, -14.0f, 2.6f, 2.6f},
    {"OTT Punch", EffectType::MultibandOtt, -18.0f, 3.5f, 3.5f},

    // === Convolution Reverb (Category 2 - Space) ===
    {"IR Small Live Room", EffectType::ConvolutionReverb, 0.30f, 3.0f, -2.0f, 0.0f, 0.0f, "small_live_room_wav"},
    {"IR Attic", EffectType::ConvolutionReverb, 0.32f, 4.0f, -2.0f, 0.0f, 0.0f, "attic_wav"},
    {"IR Concert Hall", EffectType::ConvolutionReverb, 0.50f, 10.0f, -3.0f, 0.0f, 0.0f, "concert_hall_wav"},
    {"IR Cathedral", EffectType::ConvolutionReverb, 0.55f, 12.0f, -3.0f, 0.0f, 0.0f, "cathedral_wav"},
    {"IR Parking Hall", EffectType::ConvolutionReverb, 0.42f, 8.0f, -2.0f, 0.0f, 0.0f, "parking_hall_wav"},
    {"IR Steel Tunnel", EffectType::ConvolutionReverb, 0.35f, 5.0f, -4.0f, 0.0f, 0.0f, "steel_tunnel_wav"},
    {"IR Underpass", EffectType::ConvolutionReverb, 0.28f, 3.0f, -1.5f, 0.0f, 0.0f, "underpass_wav"},
    {"IR Hotel Shower", EffectType::ConvolutionReverb, 0.30f, 2.0f, -2.0f, 0.0f, 0.0f, "hotel_shower_wav"},
    {"IR Outdoor Courtyard", EffectType::ConvolutionReverb, 0.25f, 4.0f, -1.5f, 0.0f, 0.0f, "outdoor_courtyard_wav"},
    {"IR Reverse Digital", EffectType::ConvolutionReverb, 0.40f, 0.0f, -3.0f, 0.0f, 0.0f, "reverse_digital_wav"},

    // === Tape Delay (Category 2 - Space) ===
    {"Tape Slapback", EffectType::TapeDelay, 95.0f, 0.20f, 1.5f},
    {"Tape Echo", EffectType::TapeDelay, 350.0f, 0.40f, 2.5f},
    {"Tape Dub", EffectType::TapeDelay, 640.0f, 0.55f, 3.5f},
    {"Tape Warble", EffectType::TapeDelay, 280.0f, 0.35f, 2.0f},
    {"Tape Long", EffectType::TapeDelay, 600.0f, 0.45f, 3.0f},

    // === Ping-Pong Delay (Category 2 - Space) ===
    {"PingPong Short", EffectType::PingPongDelay, 180.0f, 0.30f, 0.30f},
    {"PingPong Mid", EffectType::PingPongDelay, 350.0f, 0.40f, 0.35f},
    {"PingPong Long", EffectType::PingPongDelay, 520.0f, 0.50f, 0.40f},
    {"PingPong Wide", EffectType::PingPongDelay, 280.0f, 0.35f, 0.45f},
    {"PingPong Tight", EffectType::PingPongDelay, 140.0f, 0.25f, 0.25f},

    // === Cloud Generator (Category 9 - Granular) ===
    {"Cloud Shimmer", EffectType::CloudGenerator, 150.0f, 12.0f, 0.45f},
    {"Cloud Texture", EffectType::CloudGenerator, 80.0f, 6.0f, 0.35f},
    {"Cloud Drone", EffectType::CloudGenerator, 300.0f, 12.0f, 0.60f},
    {"Cloud Rhythmic", EffectType::CloudGenerator, 60.0f, 4.0f, 0.30f},
    {"Cloud Wash", EffectType::CloudGenerator, 200.0f, 10.0f, 0.50f},

    // === Spectral Blur (Category 0 - Spectral/Resonators) ===
    {"Spectral Blur Soft", EffectType::SpectralBlur, 100.0f, 6000.0f, 0.30f},
    {"Spectral Blur Medium", EffectType::SpectralBlur, 250.0f, 8000.0f, 0.45f},
    {"Spectral Blur Heavy", EffectType::SpectralBlur, 500.0f, 9000.0f, 0.60f},
    {"Spectral Blur Subtle", EffectType::SpectralBlur, 50.0f, 4000.0f, 0.20f},
    {"Spectral Blur Full", EffectType::SpectralBlur, 400.0f, 12000.0f, 0.55f},

    // === Spectral Delay (Category 0 - Spectral/Resonators) ===
    {"Spectral Delay Short", EffectType::SpectralDelay, 150.0f, 0.25f, 0.30f},
    {"Spectral Delay Mid", EffectType::SpectralDelay, 350.0f, 0.35f, 0.35f},
    {"Spectral Delay Long", EffectType::SpectralDelay, 600.0f, 0.45f, 0.40f},
    {"Spectral Delay Feedback", EffectType::SpectralDelay, 400.0f, 0.60f, 0.45f},
    {"Spectral Delay Wash", EffectType::SpectralDelay, 500.0f, 0.50f, 0.40f},
}};

inline constexpr std::array<CompositePreset, 27> kCompositePresets{{
        {"Dyn Glue Serial", 1, CompositeRouting::Serial, 2,
                {{{EffectType::CompressorGlue, -18.0f, 3.5f, 0.5f, 1.0f},
                    {EffectType::PeakLimiter, -0.8f, 60.0f, 0.0f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Dyn Slam Serial", 1, CompositeRouting::Serial, 2,
                {{{EffectType::CompressorGlue, -26.0f, 6.0f, 1.0f, 1.0f},
                    {EffectType::PeakLimiter, -0.4f, 30.0f, 0.0f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Dyn Glue Parallel", 1, CompositeRouting::Parallel, 2,
                {{{EffectType::CompressorGlue, -20.0f, 4.0f, 0.75f, 0.7f},
                    {EffectType::PeakLimiter, -1.2f, 90.0f, 0.0f, 0.35f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Dyn Loud Parallel", 1, CompositeRouting::Parallel, 2,
                {{{EffectType::CompressorGlue, -28.0f, 8.0f, 1.0f, 0.85f},
                    {EffectType::PeakLimiter, -0.3f, 22.0f, 0.0f, 0.65f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},

        {"Space Glide Serial", 2, CompositeRouting::Serial, 2,
                {{{EffectType::DelayEcho, 180.0f, 0.32f, 0.24f, 1.0f},
                    {EffectType::ReverbWash, 0.38f, 0.55f, 0.48f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Space Vast Serial", 2, CompositeRouting::Serial, 2,
                {{{EffectType::DelayEcho, 320.0f, 0.42f, 0.30f, 1.0f},
                    {EffectType::ReverbWash, 0.55f, 0.72f, 0.28f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Space Cloud Parallel", 2, CompositeRouting::Parallel, 2,
                {{{EffectType::DelayEcho, 260.0f, 0.42f, 0.30f, 0.50f},
                    {EffectType::ReverbWash, 0.62f, 0.82f, 0.30f, 0.75f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Space Diffuse Parallel", 2, CompositeRouting::Parallel, 2,
                {{{EffectType::DelayEcho, 95.0f, 0.22f, 0.18f, 0.35f},
                    {EffectType::ReverbWash, 0.58f, 0.75f, 0.24f, 0.70f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},

        {"Dist Color Serial", 3, CompositeRouting::Serial, 2,
                {{{EffectType::SaturationWaveshaper, 2.8f, 0.45f, 0.0f, 1.0f, 0.0f, -4.0f},
                    {EffectType::SoftHardClip, 3.0f, 0.20f, 0.38f, 1.0f, 0.0f, -3.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Dist Melt Serial", 3, CompositeRouting::Serial, 3,
                {{{EffectType::SaturationWaveshaper, 4.2f, 0.52f, 0.0f, 1.0f, 0.0f, -8.0f},
                    {EffectType::Wavefolder, 2.8f, 0.38f, 0.26f, 1.0f, -1.0f, -8.0f},
                    {EffectType::AsymShaper, 4.2f, 0.32f, 0.30f, 1.0f, 0.0f, -6.0f}}}},
        {"Dist Blend Parallel", 3, CompositeRouting::Parallel, 2,
                {{{EffectType::SoftHardClip, 4.5f, 0.42f, 0.45f, 0.50f, 0.0f, -4.5f},
                    {EffectType::Wavefolder, 4.2f, 0.58f, 0.42f, 0.40f, 0.0f, -8.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Dist Tri Parallel", 3, CompositeRouting::Parallel, 3,
                {{{EffectType::SaturationWaveshaper, 3.6f, 0.50f, 0.0f, 0.38f, 0.0f, -5.0f},
                    {EffectType::Wavefolder, 5.2f, 0.72f, 0.48f, 0.42f, 0.0f, -9.0f},
                    {EffectType::AsymShaper, 7.0f, 0.62f, 0.52f, 0.46f, 0.0f, -7.0f}}}},

        {"Filter Sweep Serial", 4, CompositeRouting::Serial, 2,
                {{{EffectType::HighPassSweep, 1800.0f, 0.04f, 0.0f, 1.0f},
                    {EffectType::CombFilter, 12.0f, 0.55f, 0.28f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Filter Razor Serial", 4, CompositeRouting::Serial, 2,
                {{{EffectType::HighPassSweep, 5200.0f, 0.08f, 0.0f, 1.0f},
                    {EffectType::CombFilter, 26.0f, 0.74f, 0.44f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Filter Blend Parallel", 4, CompositeRouting::Parallel, 2,
                {{{EffectType::HighPassSweep, 2800.0f, 0.05f, 0.0f, 0.55f},
                    {EffectType::CombFilter, 18.0f, 0.60f, 0.40f, 0.60f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Filter Hollow Parallel", 4, CompositeRouting::Parallel, 2,
                {{{EffectType::HighPassSweep, 1100.0f, 0.03f, 0.0f, 0.40f},
                    {EffectType::CombFilter, 34.0f, -0.72f, 0.50f, 0.85f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},

        {"Degrade Dirt Serial", 7, CompositeRouting::Serial, 2,
                {{{EffectType::ReduxCrush, 9.0f, 7.0f, 0.34f, 0.85f},
                    {EffectType::JitterDegrade, 12.0f, 6.0f, 0.36f, 0.85f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Degrade Ruin Serial", 7, CompositeRouting::Serial, 3,
                {{{EffectType::ReduxCrush, 6.0f, 10.0f, 0.42f, 0.80f},
                    {EffectType::JitterDegrade, 16.0f, 8.0f, 0.46f, 0.82f},
                    {EffectType::ErosionDegrade, 0.44f, 13.0f, 0.48f, 0.78f}}}},
        {"Degrade Blend Parallel", 7, CompositeRouting::Parallel, 2,
                {{{EffectType::JitterDegrade, 14.0f, 6.0f, 0.38f, 0.48f},
                    {EffectType::ErosionDegrade, 0.28f, 9.0f, 0.38f, 0.44f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Degrade Dust Parallel", 7, CompositeRouting::Parallel, 3,
                {{{EffectType::ReduxCrush, 8.0f, 5.0f, 0.36f, 0.32f},
                    {EffectType::JitterDegrade, 16.0f, 7.0f, 0.40f, 0.38f},
                    {EffectType::ErosionDegrade, 0.36f, 10.0f, 0.42f, 0.42f}}}},

        {"Rhythm Pump Serial", 8, CompositeRouting::Serial, 2,
                {{{EffectType::TranceGate, 0.125f, 0.72f, 0.48f, 1.0f},
                    {EffectType::SidechainDucker, 0.25f, 0.70f, 0.48f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Rhythm Chop Serial", 8, CompositeRouting::Serial, 3,
                {{{EffectType::TranceGate, 0.08f, 0.80f, 0.42f, 1.0f},
                    {EffectType::SidechainDucker, 0.18f, 0.75f, 0.55f, 1.0f},
                    {EffectType::BeatRepeatInsert, 0.08f, 0.58f, 0.26f, 1.0f}}}},
        {"Rhythm Pulse Parallel", 8, CompositeRouting::Parallel, 2,
                {{{EffectType::Autopan, 0.90f, 0.70f, 1.0f, 0.50f},
                    {EffectType::TranceGate, 0.125f, 0.65f, 0.55f, 0.55f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Rhythm Stutter Parallel", 8, CompositeRouting::Parallel, 3,
                {{{EffectType::Autopan, 1.2f, 0.75f, 1.0f, 0.42f},
                    {EffectType::SidechainDucker, 0.18f, 0.72f, 0.58f, 0.55f},
                    {EffectType::BeatRepeatInsert, 0.08f, 0.60f, 0.34f, 0.66f}}}},

        {"Drum Tight Room", 2, CompositeRouting::Serial, 1,
                {{{EffectType::ReverbWash, 0.16f, 0.30f, 0.66f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Drum Soft Plate", 2, CompositeRouting::Serial, 1,
                {{{EffectType::ReverbWash, 0.20f, 0.44f, 0.42f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
        {"Drum Dark Room", 2, CompositeRouting::Serial, 1,
                {{{EffectType::ReverbWash, 0.18f, 0.36f, 0.78f, 1.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f},
                    {EffectType::None, 0.0f, 0.0f, 0.0f, 0.0f}}}},
}};

inline constexpr uint16_t kDefaultMasterLimiterPresetId = 28;

inline constexpr uint16_t getPresetCount() {
    return static_cast<uint16_t>(kPresets.size());
}

inline constexpr uint16_t getCompositePresetCount() {
    return static_cast<uint16_t>(kCompositePresets.size());
}

inline constexpr uint16_t compositePresetIndexToPresetId(uint16_t index) {
    return static_cast<uint16_t>(kCompositePresetIdBase + index);
}

inline constexpr bool isValidCompositePresetId(uint16_t presetId) {
    return presetId >= kCompositePresetIdBase
        && presetId < static_cast<uint16_t>(kCompositePresetIdBase + getCompositePresetCount());
}

inline constexpr uint16_t compositePresetIdToIndex(uint16_t presetId) {
    return static_cast<uint16_t>(presetId - kCompositePresetIdBase);
}

inline constexpr const CompositePreset& getCompositePresetById(uint16_t presetId) {
    return isValidCompositePresetId(presetId)
        ? kCompositePresets[compositePresetIdToIndex(presetId)]
        : kCompositePresets[0];
}

inline constexpr uint16_t getTotalPresetCount() {
    return static_cast<uint16_t>(getPresetCount() + getCompositePresetCount());
}

inline constexpr bool isValidDisplayId(uint16_t displayId) {
    return displayId >= 1 && displayId <= getTotalPresetCount();
}

inline constexpr bool isValidPresetId(uint16_t presetId) {
    return presetId < getPresetCount() || isValidCompositePresetId(presetId);
}

inline constexpr uint16_t displayIdToPresetId(uint16_t displayId) {
    if (displayId == 0) {
        return 0;
    }

    if (displayId <= getPresetCount()) {
        return static_cast<uint16_t>(displayId - 1);
    }

    const uint16_t compositeIndex = static_cast<uint16_t>(displayId - getPresetCount() - 1);
    return compositePresetIndexToPresetId(compositeIndex);
}

inline constexpr uint16_t presetIdToDisplayId(uint16_t presetId) {
    if (isValidCompositePresetId(presetId)) {
        return static_cast<uint16_t>(getPresetCount() + compositePresetIdToIndex(presetId) + 1);
    }

    return static_cast<uint16_t>(presetId + 1);
}

inline constexpr const EffectPreset& getPresetById(uint16_t presetId) {
    return isValidPresetId(presetId) ? kPresets[presetId] : kPresets[0];
}

inline constexpr std::string_view getPresetName(uint16_t presetId) {
    if (isValidCompositePresetId(presetId)) {
        return getCompositePresetById(presetId).name;
    }

    return getPresetById(presetId).name;
}

inline constexpr std::string_view getPresetNameByDisplayId(uint16_t displayId) {
    if (!isValidDisplayId(displayId)) {
        return std::string_view{"Invalid"};
    }

    return getPresetName(displayIdToPresetId(displayId));
}

inline constexpr bool isSpotOnlyEffectType(EffectType type) {
    return type == EffectType::TapeStop || type == EffectType::BeatRepeat;
}

inline constexpr bool isValidCategoryDigit(uint8_t categoryDigit) {
    return categoryDigit <= 9;
}

inline constexpr std::string_view getCategoryName(uint8_t categoryDigit) {
    switch (categoryDigit) {
        case 0: return "Spectral/Resonators";
        case 1: return "Dynamics";
        case 2: return "Space";
        case 3: return "Distortion";
        case 4: return "Filters";
        case 5: return "Modulation";
        case 6: return "Pitch";
        case 7: return "Degrade";
        case 8: return "Rhythm";
        case 9: return "Granular";
        default: return "Invalid";
    }
}

inline constexpr bool tryGetCategoryDigitForType(EffectType type, uint8_t& categoryDigit) {
    switch (type) {
        case EffectType::CompressorGlue:
        case EffectType::PeakLimiter:
        case EffectType::TransientShaper:
            categoryDigit = 1;
            return true;
        case EffectType::ReverbWash:
        case EffectType::DelayEcho:
            categoryDigit = 2;
            return true;
        case EffectType::SaturationWaveshaper:
        case EffectType::SoftHardClip:
        case EffectType::Wavefolder:
        case EffectType::AsymShaper:
            categoryDigit = 3;
            return true;
        case EffectType::HighPassSweep:
        case EffectType::CombFilter:
        case EffectType::MultiModeEQ:
        case EffectType::FormantFilter:
            categoryDigit = 4;
            return true;
        case EffectType::RingModulator:
        case EffectType::Chorus:
        case EffectType::Phaser:
        case EffectType::Flanger:
            categoryDigit = 5;
            return true;
        case EffectType::FrequencyShifter:
        case EffectType::PitchShifter:
        case EffectType::Harmonizer:
            categoryDigit = 6;
            return true;
        case EffectType::ReduxCrush:
        case EffectType::JitterDegrade:
        case EffectType::ErosionDegrade:
            categoryDigit = 7;
            return true;
        case EffectType::Autopan:
        case EffectType::TranceGate:
        case EffectType::SidechainDucker:
        case EffectType::BeatRepeatInsert:
            categoryDigit = 8;
            return true;
        case EffectType::TimeFreezer:
        case EffectType::GrainDelay:
            categoryDigit = 9;
            return true;
        case EffectType::PhysicalModelingResonator:
        case EffectType::SpectralBlur:
        case EffectType::SpectralDelay:
            categoryDigit = 0;
            return true;
        case EffectType::MultibandOtt:
            categoryDigit = 1;
            return true;
        case EffectType::ConvolutionReverb:
        case EffectType::TapeDelay:
        case EffectType::PingPongDelay:
            categoryDigit = 2;
            return true;
        case EffectType::CloudGenerator:
            categoryDigit = 9;
            return true;
        default:
            break;
    }

    return false;
}

inline constexpr bool isPresetAssignableToSlot(uint16_t presetId) {
    if (isValidCompositePresetId(presetId)) {
        return true;
    }

    if (!isValidPresetId(presetId)) {
        return false;
    }

    const auto type = kPresets[presetId].type;
    return !isSpotOnlyEffectType(type);
}

inline constexpr uint16_t getSlotAssignablePresetCount() {
    uint16_t count = 0;
    for (uint16_t presetId = 0; presetId < getPresetCount(); ++presetId) {
        if (isPresetAssignableToSlot(presetId)) {
            ++count;
        }
    }

    count = static_cast<uint16_t>(count + getCompositePresetCount());

    return count;
}

inline constexpr bool isValidSlotAssignableDisplayId(uint16_t displayId) {
    return displayId >= 1 && displayId <= getSlotAssignablePresetCount();
}

inline constexpr uint16_t slotAssignableDisplayIdToPresetId(uint16_t displayId) {
    if (!isValidSlotAssignableDisplayId(displayId)) {
        return 0;
    }

    uint16_t currentDisplayId = 0;
    for (uint16_t presetId = 0; presetId < getPresetCount(); ++presetId) {
        if (!isPresetAssignableToSlot(presetId)) {
            continue;
        }

        ++currentDisplayId;
        if (currentDisplayId == displayId) {
            return presetId;
        }
    }

    for (uint16_t compositeIndex = 0; compositeIndex < getCompositePresetCount(); ++compositeIndex) {
        ++currentDisplayId;
        if (currentDisplayId == displayId) {
            return compositePresetIndexToPresetId(compositeIndex);
        }
    }

    return 0;
}

inline constexpr std::string_view getSlotAssignablePresetNameByDisplayId(uint16_t displayId) {
    if (!isValidSlotAssignableDisplayId(displayId)) {
        return "Invalid";
    }

    return getPresetName(slotAssignableDisplayIdToPresetId(displayId));
}

inline constexpr bool isPresetAllowedForTrack(uint16_t presetId) {
    return isPresetAssignableToSlot(presetId);
}

inline constexpr uint16_t getCategoryPresetCount(uint8_t categoryDigit) {
    if (!isValidCategoryDigit(categoryDigit)) {
        return 0;
    }

    uint16_t count = 0;
    for (uint16_t presetId = 0; presetId < getPresetCount(); ++presetId) {
        if (!isPresetAssignableToSlot(presetId)) {
            continue;
        }

        uint8_t presetCategory = 0;
        if (!tryGetCategoryDigitForType(kPresets[presetId].type, presetCategory)) {
            continue;
        }

        if (presetCategory == categoryDigit) {
            ++count;
        }
    }

    for (uint16_t compositeIndex = 0; compositeIndex < getCompositePresetCount(); ++compositeIndex) {
        if (kCompositePresets[compositeIndex].categoryDigit == categoryDigit) {
            ++count;
        }
    }

    return count;
}

inline constexpr bool isCategoryImplemented(uint8_t categoryDigit) {
    return getCategoryPresetCount(categoryDigit) > 0;
}

inline constexpr uint16_t getCategoryMappedPresetCount() {
    uint16_t count = 0;
    for (uint16_t categoryDigit = 0; categoryDigit <= 9; ++categoryDigit) {
        count = static_cast<uint16_t>(count + getCategoryPresetCount(static_cast<uint8_t>(categoryDigit)));
    }
    return count;
}

inline constexpr bool isValidCategoryPresetDisplayId(uint8_t categoryDigit, uint16_t displayId) {
    return displayId >= 1 && displayId <= getCategoryPresetCount(categoryDigit);
}

inline constexpr uint16_t categoryPresetDisplayIdToPresetId(uint8_t categoryDigit, uint16_t displayId) {
    if (!isValidCategoryPresetDisplayId(categoryDigit, displayId)) {
        return 0;
    }

    uint16_t currentDisplayId = 0;
    for (uint16_t presetId = 0; presetId < getPresetCount(); ++presetId) {
        if (!isPresetAssignableToSlot(presetId)) {
            continue;
        }

        uint8_t presetCategory = 0;
        if (!tryGetCategoryDigitForType(kPresets[presetId].type, presetCategory)) {
            continue;
        }

        if (presetCategory != categoryDigit) {
            continue;
        }

        ++currentDisplayId;
        if (currentDisplayId == displayId) {
            return presetId;
        }
    }

    for (uint16_t compositeIndex = 0; compositeIndex < getCompositePresetCount(); ++compositeIndex) {
        if (kCompositePresets[compositeIndex].categoryDigit != categoryDigit) {
            continue;
        }

        ++currentDisplayId;
        if (currentDisplayId == displayId) {
            return compositePresetIndexToPresetId(compositeIndex);
        }
    }

    return 0;
}

inline constexpr std::string_view getCategoryPresetNameByDisplayId(uint8_t categoryDigit, uint16_t displayId) {
    if (!isValidCategoryPresetDisplayId(categoryDigit, displayId)) {
        return "Invalid";
    }

    return getPresetName(categoryPresetDisplayIdToPresetId(categoryDigit, displayId));
}

inline constexpr uint16_t getTrackPresetCount() {
    return getSlotAssignablePresetCount();
}

inline constexpr bool isValidTrackDisplayId(uint16_t displayId) {
    return isValidSlotAssignableDisplayId(displayId);
}

inline constexpr uint16_t trackDisplayIdToPresetId(uint16_t displayId) {
    return slotAssignableDisplayIdToPresetId(displayId);
}

inline constexpr std::string_view getTrackPresetNameByDisplayId(uint16_t displayId) {
    return getSlotAssignablePresetNameByDisplayId(displayId);
}

} // namespace EffectPresetCatalog
