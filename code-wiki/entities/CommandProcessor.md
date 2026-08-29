---
title: CommandProcessor
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [audio, command, real-time]
source_files:
  - Source/Audio/CommandProcessor.h
  - Source/Audio/CommandProcessor.cpp
---

# CommandProcessor

## Purpose
Dispatches commands from the [[CommandQueue]] to the appropriate subsystems. Owns custom algorithm runtime storage. Uses a Delegate pattern to call back into [[AudioEngine]] for operations that require engine-owned state (sound presets, transport reset, drum engine, algorithm reset). Extracted from AudioEngine to reduce its size.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `appState` | `AppState&` | Shared atomic state reference |
| `commandQueue` | `CommandQueue&` | UI→Audio command queue |
| `effectProcessor` | `EffectProcessor&` | Effect processor reference |
| `algorithmRegistry` | `AlgorithmPresetRegistry*` | Custom algorithm registry pointer |
| `drumSampleLibrary` | `DrumSampleLibrary*` | Drum sample library pointer |
| `delegate` | `Delegate&` | Callback interface to AudioEngine |
| `customAlgorithmCounts[4]` | `uint16_t` | Cached custom algorithm counts per track |
| `customTrackAlgorithms[4]` | `vector<unique_ptr<CustomAlgorithmInstance>>` | Custom algorithm instances per track |

## Delegate Interface
The `Delegate` abstract base class defines callbacks into AudioEngine:
- `getMaxSynthPresetIdForTrack(track)` — Get max synth preset ID
- `applySoundPreset(track, presetId, manualOverride)` — Apply synth preset
- `resetTransportAndArrangement()` — Reset transport + arrangement anchor
- `setArrangementAnchorInitialized(initialized)` — Set arrangement anchor flag
- `resetAlgorithm(track, algorithmId)` — Reset algorithm state
- `setDrumSampleForSlot(slot, sampleData)` — Set drum sample for slot
- `setDrumSampleSlotVolume/Tune/StartOffset/Decay/VelocitySensitivity(slot, value)` — Set drum slot params

## Relationships
- **OwnedBy:** [[AudioEngine]] (as `commandProcessor` member)
- **Uses:** [[AppState]], [[CommandQueue]], [[EffectProcessor]], [[AlgorithmPresetRegistry]], [[DrumSampleLibrary]]
- **Delegates to:** [[AudioEngine::CommandDelegate]] (implementation of Delegate interface)

## Behavior
- `process()` — Drain CommandQueue and dispatch each command (40+ command types)
- `rebuildCustomAlgorithmInstances()` — Rebuild custom algorithm instances from registry
- `getMaxAlgorithmIdForTrack(track)` — Get max valid algorithm ID (built-in + custom)
- `getTrackAlgorithm(track, algorithmId, builtinAlgorithms)` — Lookup algorithm by ID

## Thread Safety
- `process()` runs on the **Audio Thread** — no allocations (except `rebuildCustomAlgorithmInstances` which allocates `unique_ptr`s)
- All state changes go through `AppState` atomics or the Delegate interface
- Cross-thread communication only via [[CommandQueue]] (SPSC)

## Gotchas
- Custom algorithm IDs are sparse (≥2048). Use `kCustomAlgorithmIdBase + index` math.
- `rebuildCustomAlgorithmInstances()` allocates — should ideally be moved off audio path
- The Delegate pattern avoids circular dependency between CommandProcessor and AudioEngine
- `process()` is a 40+ case switch statement — consider further decomposition by command domain
