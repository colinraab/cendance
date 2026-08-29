---
title: EffectSystem
created: 2026-05-14
updated: 2026-05-14
type: concept
tags: [effect, insert-fx, spot-fx, real-time]
---

# Effect System

## What It Is
cendance has two separate effect domains: **Insert FX** (preset-based, persisted) and **Spot FX** (trigger-based, runtime-only). Both process audio on the audio thread.

## Insert FX (Preset Domain)
- **Assignable** via selector flow (`F` key) to track/master slots
- **3 slots per track** + **3 master slots**
- **Persisted** in project files via `effectPresetSlots` / `masterEffectPresetSlots`
- **Catalog** and ID mapping in [[EffectPresetCatalog]]
- **Preset types**: Single effects or composite (serial/parallel routing)
- **Processing**: `processTrackInsertEffects()` / `processMasterEffects()` in [[AudioEngine]]

### Insert FX Flow
1. User selects preset via UI → `SetTrackEffectPreset` / `SetMasterEffectPreset` command
2. [[AudioEngine::processCommands()]]: calls `applyTrackEffectPreset()` / `applyMasterEffectPreset()`
3. Effect instance configured from preset parameters
4. Each audio block: `processTrackInsertEffects()` runs the effect chain

## Spot FX (Trigger Domain)
- **Triggered** by dedicated commands (`SpotEffectOn/Off/Toggle`)
- **Runtime-only**, intentionally reset on project load (NOT persisted)
- **Metadata/defaults** in [[SpotEffectCatalog]]
- **Current types**: `TapeBrake` (0), `Stutter` (1)
- **Processing**: `processSpotEffects()` in [[AudioEngine]] on master buffer

### Spot FX Flow
1. User triggers spot FX → `SpotEffectOn/Toggle` command
2. [[AudioEngine::processCommands()]]: updates `activeSpotEffects` bitmask
3. Each audio block: `applySpotEffectsBitmask()` → `processSpotEffects()`

## Effect Categories (10 domains)
| Domain | Examples |
|--------|----------|
| 0_Dynamics | CompressorGlue, PeakLimiter, TransientShaper |
| 1_Space | DelayEcho, ReverbWash |
| 2_Distortion | AsymShaper, SaturationWaveshaper, SoftHardClip, Wavefolder |
| 3_Filters | CombFilter, FormantFilter, HighPassSweep, MultiModeEQ |
| 4_Modulation | Chorus, Flanger, Phaser, RingModulator |
| 5_Pitch | FrequencyShifter, Harmonizer, PitchShifter |
| 6_Degrade | ErosionDegrade, JitterDegrade, ReduxCrush |
| 7_Rhythm | Autopan, BeatRepeatInsert, SidechainDucker, TranceGate |
| 8_Granular | GrainDelay, TimeFreezer |
| 9_SpectralResonators | PhysicalModelingResonator |
| Spot | BeatRepeat, TapeStop |

## Involved Classes
- [[AudioEngine]] — effect processing
- [[EffectPresetCatalog]] — insert FX preset metadata
- [[SpotEffectCatalog]] — spot FX trigger metadata
- [[AppState]] — effect slot state (atomic)
- [[TuiApp]] — UI for assignment and triggering

## Rules
1. Insert FX and Spot FX are SEPARATE domains — never mix them
2. Spot FX are NOT persisted in project files
3. Effect slot indices: 0-2 (3 slots)
4. Track index 4 = master gain/commands
5. Composite presets support serial/parallel routing with up to `kCompositeMaxComponents` components

## Gotchas
- `cachedTrackEffectPresets` and `cachedMasterEffectPresets` in [[AudioEngine]] track current state
- Effect instances are pre-allocated (no allocation on audio path)
- Spot FX reset on project load — this is intentional
