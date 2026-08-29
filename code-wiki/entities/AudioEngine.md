---
title: AudioEngine
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [audio, real-time]
source_files:
  - Source/Audio/AudioEngine.h
  - Source/Audio/AudioEngine.cpp
---

# AudioEngine

## Purpose
Central audio callback class. Owns the entire DSP chain: transport, generators, synths, effects (insert + spot + master). Processes commands from [[CommandQueue]] and pushes metering data to [[MeterQueue]].

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `appState` | `AppState&` | Shared atomic state (BPM, track params, arrangement) |
| `commandQueue` | `CommandQueue&` | Lock-free UI→Audio command queue |
| `meterQueue` | `MeterQueue&` | Lock-free Audio→UI metering queue |
| `deviceManager` | `juce::AudioDeviceManager` | JUCE audio device management |
| `transport` | `Transport` | BPM clock, playhead, beat/bar tracking |
| `drumEngine` | `DrumEngine` | Drum synthesis |
| `bassEngine` | `BassEngine` | Bass synthesis |
| `chordEngine` | `ChordEngine` | Chord synthesis |
| `leadEngine` | `LeadEngine` | Lead synthesis |
| `builtinTrackAlgorithms` | `array<array<GenAlg*, N>, 4>` | Built-in algorithm instances per track |
| `customTrackAlgorithms` | `array<vector<unique_ptr<CustAlgInst>>, 4>` | Custom algorithm instances per track |
| `cachedTrackEffectPresets` | `array<array<uint16_t, 3>, 4>` | Cached effect preset IDs per track slot |
| `cachedMasterEffectPresets` | `array<uint16_t, 3>` | Cached master effect preset IDs |
| `customAlgorithmCounts` | `array<uint16_t, 4>` | Cached custom algorithm counts (avoids mutex on audio path) |
| `algorithmRegistry` | `AlgorithmPresetRegistry*` | Custom algorithm registry pointer |
| `drumSampleLibrary` | `DrumSampleLibrary*` | Drum sample library pointer |
| `melodicSampleLibrary` | `MelodicSampleLibrary*` | Melodic sample library pointer |

## Relationships
- **Owns:** [[Transport]], [[DrumEngine]], [[BassEngine]], [[ChordEngine]], [[LeadEngine]], all generator instances, all effect instances
- **Uses:** [[AppState]], [[CommandQueue]], [[MeterQueue]], [[AlgorithmPresetRegistry]], [[DrumSampleLibrary]], [[MelodicSampleLibrary]]
- **CalledBy:** JUCE audio device callback (audio thread)
- **Calls:** [[GenerativeAlgorithm::processMidi]], [[SoundEngine]] methods, [[MasterEffect]] methods, [[Transport::advance]]

## Behavior
- `audioDeviceIOCallback()` — Main audio callback (real-time). Processes commands, runs generators, mixes tracks, applies effects, pushes meters.
- `processCommands()` — Drains [[CommandQueue]] and applies each command to state/DSP.
- `initializeAlgorithmMap()` — Creates built-in algorithm instances and registers them per track.
- `rebuildCustomAlgorithmInstances()` — Rebuilds custom algorithm instances from [[AlgorithmPresetRegistry]]. Triggered by `RebuildCustomAlgorithms` command.
- `applyTrackEffectPreset()` / `applyMasterEffectPreset()` — Configures effect instances from preset catalog.
- `processTrackInsertEffects()` / `processMasterEffects()` — Runs effect chains per block.
- `updateArrangementState()` — Advances arrangement sections based on bar boundaries.
- `processSpotEffects()` — Runs active spot effects on master buffer.

## Thread Safety
- Lives on the **Audio Thread** (real-time). NO allocations, NO mutexes, NO file I/O, NO std::cout.
- Cross-thread communication only via [[CommandQueue]] (UI→Audio) and [[MeterQueue]] (Audio→UI).
- `AppState` members are all `std::atomic`.
- `customAlgorithmCounts` cached to avoid locking registry mutex on audio path.

## Gotchas
- `getMaxAlgorithmIdForTrack()` uses cached `customAlgorithmCounts` — must call `rebuildCustomAlgorithmInstances()` after registry changes.
- Effect preset slots: 3 per track + 3 master. Slot indices 0-2.
- Spot effects are runtime-only, NOT persisted in project files.
- Insert FX and Spot FX are separate domains — don't mix them.
- `kCustomAlgorithmIdBase = 2048` — custom algorithm IDs are sparse, not dense. Max-ID math uses sparse base offset.
