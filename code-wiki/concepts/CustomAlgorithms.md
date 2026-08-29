---
title: CustomAlgorithms
created: 2026-05-14
updated: 2026-05-14
type: concept
tags: [custom-algorithm, preset, sparse-id]
---

# Custom Algorithm System

## What It Is
Users can create custom MIDI generative algorithms via the TUI algorithm editor or by importing P2P packages. Custom algorithms are stored as [[CustomAlgorithmPreset]] objects in the [[AlgorithmPresetRegistry]] and instantiated as [[CustomAlgorithmInstance]] objects at runtime.

## How It Works

### Preset Storage
- Presets are JSON files on disk, organized by track
- [[AlgorithmPresetRegistry]] manages CRUD and indexing
- Each preset has a string ID and a runtime ID (uint16_t)

### Runtime IDs (Sparse!)
- Built-in algorithms: IDs 0 through N-1 (dense)
- Custom algorithms: IDs start at `kCustomAlgorithmIdBase = 2048` (sparse)
- `isCustomAlgorithmId(id)` returns true if `id >= 2048`
- `customRuntimeIndex(id)` returns `id - 2048` (dense index into custom array)

### Instantiation
- [[AudioEngine::initializeAlgorithmMap()`]]: creates built-in algorithm instances
- [[AudioEngine::rebuildCustomAlgorithmInstances()`]: creates custom instances from registry
- Triggered by `RebuildCustomAlgorithms` command from UI
- Custom instances stored in `customTrackAlgorithms[track]` vector

### Audio Path
- [[AudioEngine::getTrackAlgorithm(track, id)]]: returns algorithm pointer
  - If `id < 2048`: looks up in `builtinTrackAlgorithms[track][id]`
  - If `id >= 2048`: looks up in `customTrackAlgorithms[track][id - 2048]`
- [[AudioEngine::getMaxAlgorithmIdForTrack(track)]: returns max valid ID
  - Uses cached `customAlgorithmCounts[track]` to avoid registry mutex

### Performance Optimization
- `customAlgorithmCounts` cached in [[AudioEngine]] per track
- Updated only when `rebuildCustomAlgorithmInstances()` is called
- Avoids locking registry mutex on the audio path

## Involved Classes
- [[AlgorithmPresetRegistry]] — preset CRUD and indexing
- [[CustomAlgorithmPreset]] — preset data model
- [[CustomAlgorithmInstance]] — runtime algorithm instance
- [[AudioEngine]] — instantiation and audio path lookup
- [[TuiApp]] — algorithm editor modal

## Rules
1. Custom algorithm IDs are SPARSE (≥2048). Never use dense math.
2. After registry changes, must dispatch `RebuildCustomAlgorithms` command
3. `getMaxAlgorithmIdForTrack()` = `kCustomAlgorithmIdBase + customCount - 1`
4. Audio path must NEVER lock the registry mutex

## Gotchas
- **CRITICAL**: `kCustomAlgorithmIdBase = 2048` means max-ID math is `2048 + count - 1`, NOT `builtinCount + customCount - 1`
- If `customCount` is 0, max custom ID is 2047 (i.e., no valid custom IDs)
- Algorithm editor modal in [[TuiApp]] saves via `saveAlgorithmEditorDraft()` which calls registry `savePreset()`
- P2P algorithm packages install into the same registry
