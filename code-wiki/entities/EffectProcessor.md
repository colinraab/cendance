---
title: EffectProcessor
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [audio, effect, real-time]
source_files:
  - Source/Audio/EffectProcessor.h
  - Source/Audio/EffectProcessor.cpp
---

# EffectProcessor

## Purpose
Owns all 40+ effect instances (track insert FX, master FX, spot FX). Handles effect configuration from presets, effect processing per audio block, and composite preset routing. Extracted from AudioEngine to reduce its size from 2,836 to ~1,100 lines.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `appState` | `AppState&` | Shared atomic state reference |
| `cachedTrackEffectPresets[4][3]` | `uint16_t` | Cached preset IDs per track/slot |
| `trackEffectTypes[4][3]` | `EffectType` | Cached effect type per track/slot |
| `trackCompositeStates[4][3]` | `CompositeSlotState` | Composite routing state per track/slot |
| `tapeStops[4][3]` | `TapeStop` | Tape stop effect instances (per track) |
| `beatRepeats[4][3]` | `BeatRepeat` | Beat repeat effect instances |
| `highPassSweeps[4][3]` | `HighPassSweep` | HPF sweep effect instances |
| `reverbWashes[4][3]` | `ReverbWash` | Reverb wash effect instances |
| `reduxCrushers[4][3]` | `ReduxCrush` | Bit crusher effect instances |
| `delayEchos[4][3]` | `DelayEcho` | Delay/echo effect instances |
| `saturators[4][3]` | `SaturationWaveshaper` | Saturation effect instances |
| `softHardClips[4][3]` | `SoftHardClip` | Soft/hard clip effect instances |
| `wavefolders[4][3]` | `Wavefolder` | Wavefolder effect instances |
| `asymShapers[4][3]` | `AsymShaper` | Asymmetric shaper effect instances |
| `compressors[4][3]` | `CompressorGlue` | Compressor effect instances |
| `limiters[4][3]` | `PeakLimiter` | Peak limiter effect instances |
| `transientShapers[4][3]` | `TransientShaper` | Transient shaper effect instances |
| `combFilters[4][3]` | `CombFilter` | Comb filter effect instances |
| `multiModeEqs[4][3]` | `MultiModeEQ` | Multi-mode EQ effect instances |
| `formantFilters[4][3]` | `FormantFilter` | Formant filter effect instances |
| `autopans[4][3]` | `Autopan` | Auto-pan effect instances |
| `ringModulators[4][3]` | `RingModulator` | Ring modulator effect instances |
| `choruses[4][3]` | `Chorus` | Chorus effect instances |
| `phasers[4][3]` | `Phaser` | Phaser effect instances |
| `flangers[4][3]` | `Flanger` | Flanger effect instances |
| `jitterDegrades[4][3]` | `JitterDegrade` | Jitter degrade effect instances |
| `erosionDegrades[4][3]` | `ErosionDegrade` | Erosion degrade effect instances |
| `tranceGates[4][3]` | `TranceGate` | Trance gate effect instances |
| `sidechainDuckers[4][3]` | `SidechainDucker` | Sidechain ducker effect instances |
| `beatRepeatInserts[4][3]` | `BeatRepeatInsert` | Beat repeat insert effect instances |
| `frequencyShifters[4][3]` | `FrequencyShifter` | Frequency shifter effect instances |
| `pitchShifters[4][3]` | `PitchShifter` | Pitch shifter effect instances |
| `harmonizers[4][3]` | `Harmonizer` | Harmonizer effect instances |
| `timeFreezers[4][3]` | `TimeFreezer` | Time freezer effect instances |
| `grainDelays[4][3]` | `GrainDelay` | Grain delay effect instances |
| `physicalModelingResonators[4][3]` | `PhysicalModelingResonator` | Physical modeling resonator instances |
| `masterTapeStops[3]` | `TapeStop` | Master tape stop effect instances |
| `masterBeatRepeats[3]` | `BeatRepeat` | Master beat repeat effect instances |
| `masterHighPassSweeps[3]` | `HighPassSweep` | Master HPF sweep effect instances |
| `masterReverbWashes[3]` | `ReverbWash` | Master reverb wash effect instances |
| `masterReduxCrushers[3]` | `ReduxCrush` | Master bit crusher effect instances |
| `masterDelayEchos[3]` | `DelayEcho` | Master delay/echo effect instances |
| `masterSaturators[3]` | `SaturationWaveshaper` | Master saturation effect instances |
| `masterSoftHardClips[3]` | `SoftHardClip` | Master soft/hard clip effect instances |
| `masterWavefolders[3]` | `Wavefolder` | Master wavefolder effect instances |
| `masterAsymShapers[3]` | `AsymShaper` | Master asymmetric shaper effect instances |
| `masterCompressors[3]` | `CompressorGlue` | Master compressor effect instances |
| `masterLimiters[3]` | `PeakLimiter` | Master peak limiter effect instances |
| `masterTransientShapers[3]` | `TransientShaper` | Master transient shaper effect instances |
| `masterCombFilters[3]` | `CombFilter` | Master comb filter effect instances |
| `masterMultiModeEqs[3]` | `MultiModeEQ` | Master multi-mode EQ effect instances |
| `masterFormantFilters[3]` | `FormantFilter` | Master formant filter effect instances |
| `masterAutopans[3]` | `Autopan` | Master auto-pan effect instances |
| `masterRingModulators[3]` | `RingModulator` | Master ring modulator effect instances |
| `masterChoruses[3]` | `Chorus` | Master chorus effect instances |
| `masterPhasers[3]` | `Phaser` | Master phaser effect instances |
| `masterFlangers[3]` | `Flanger` | Master flanger effect instances |
| `masterJitterDegrades[3]` | `JitterDegrade` | Master jitter degrade effect instances |
| `masterErosionDegrades[3]` | `ErosionDegrade` | Master erosion degrade effect instances |
| `masterTranceGates[3]` | `TranceGate` | Master trance gate effect instances |
| `masterSidechainDuckers[3]` | `SidechainDucker` | Master sidechain ducker effect instances |
| `masterBeatRepeatInserts[3]` | `BeatRepeatInsert` | Master beat repeat insert effect instances |
| `masterFrequencyShifters[3]` | `FrequencyShifter` | Master frequency shifter effect instances |
| `masterPitchShifters[3]` | `PitchShifter` | Master pitch shifter effect instances |
| `masterHarmonizers[3]` | `Harmonizer` | Master harmonizer effect instances |
| `masterTimeFreezers[3]` | `TimeFreezer` | Master time freezer effect instances |
| `masterGrainDelays[3]` | `GrainDelay` | Master grain delay effect instances |
| `masterPhysicalModelingResonators[3]` | `PhysicalModelingResonator` | Master physical modeling resonator instances |
| `spotTapeStop` | `TapeStop` | Spot FX tape stop |
| `spotBeatRepeat` | `BeatRepeat` | Spot FX beat repeat |
| `cachedSpotEffectsMask` | `uint8_t` | Cached spot FX bitmask |
| `compositeWorkBuffers` | `AudioBuffer[]` | Work buffers for parallel composite routing |

## Relationships
- **OwnedBy:** [[AudioEngine]] (as `effectProcessor` member)
- **Uses:** [[AppState]] (for effect slot state)

## Behavior
- `prepare(sampleRate, maxBlockSize)` — Initialize all effect instances, apply cached presets
- `applyTrackEffectPreset(track, slot, presetId)` — Configure track effect slot from preset
- `applyMasterEffectPreset(slot, presetId)` — Configure master effect slot from preset
- `processTrackInsertEffects(track, buffer, samples, bpm)` — Run track insert FX chain
- `processMasterEffects(buffer, samples, bpm)` — Run master FX chain
- `applySpotEffectsBitmask(mask)` — Update active spot effects
- `processSpotEffects(buffer, samples, bpm)` — Run spot effects on master buffer

## Thread Safety
- `prepare()` runs on the main thread (device setup)
- All processing methods run on the **Audio Thread** — no allocations, no mutexes
- Effect instances are pre-allocated, only parameters change at runtime

## Gotchas
- Effect slot indices: 0-2 (3 slots per track + 3 master slots)
- Spot FX are runtime-only, NOT persisted in project files
- Composite presets support serial/parallel routing with up to `kCompositeMaxComponents` components
- Cache reset pattern: set cached preset to `UINT16_MAX` before re-applying to force update
