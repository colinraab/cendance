---
title: AlgorithmPresetRegistry
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [preset, algorithm, custom-algorithm]
source_files:
  - Source/App/AlgorithmPresetRegistry.h
  - Source/App/AlgorithmPresetRegistry.cpp
---

# AlgorithmPresetRegistry

## Purpose
Manages custom algorithm presets on disk. CRUD operations for [[CustomAlgorithmPreset]] objects. Maintains per-track preset lists and a preset ID index. Thread-safe via mutex.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `mutex_` | `std::mutex` | Protects all mutable state |
| `trackPresets_` | `array<vector<PresetEntry>, 4>` | Per-track preset entries |
| `presetIdIndex_` | `map<string, pair<uint8_t, uint16_t>>` | Preset ID → (trackIndex, runtimeId) |
| `rootDir_` | `juce::File` | Root directory for preset files |
| `indexFile_` | `juce::File` | Index file path |

## Relationships
- **UsedBy:** [[AudioEngine]] (for `rebuildCustomAlgorithmInstances()` and `getMaxAlgorithmIdForTrack()`)
- **UsedBy:** [[TuiApp]] (algorithm editor modal)
- **Contains:** [[CustomAlgorithmPreset]] via PresetEntry

## Behavior
- `ensureDirectories()` — Create preset directories if missing.
- `reload()` — Reload all presets from disk.
- `savePreset(preset)` — Save new preset, assign runtime ID.
- `updatePreset(id, preset)` — Update existing preset.
- `deletePreset(id)` — Delete preset by runtime ID.
- `findByRuntimeId(track, id)` — Lookup by track + runtime ID.
- `findByPresetId(id)` — Lookup by preset string ID.
- `listForTrack(track)` — List all presets for a track.
- `listByGenre(track, genreId)` — List presets filtered by genre.
- `getCustomAlgorithmCountForTrack(track)` — Count of custom algorithms per track.
- `getTotalAlgorithmCountForTrack(track)` — Built-in + custom count.
- `isCustomAlgorithmId(id)` — Check if ID is in custom range (≥2048).
- `customRuntimeIndex(id)` — Convert sparse ID to dense index.
- `rebuildIndex()` — Rebuild the preset ID index.

## Constants
- `kCustomAlgorithmIdBase = 2048` — Custom algorithm IDs start at 2048 (sparse).

## Thread Safety
- All public methods lock `mutex_`.
- [[AudioEngine]] caches `customAlgorithmCounts` to avoid locking on audio path.

## Gotchas
- **Sparse IDs**: Custom algorithm IDs are NOT dense. ID 2048 is index 0, ID 2049 is index 1, etc. Use `customRuntimeIndex()` to convert.
- `getMaxAlgorithmIdForTrack()` should use `kCustomAlgorithmIdBase + customCount - 1`, NOT `builtinCount + customCount - 1`.
- Preset files are JSON on disk. Index file caches the directory listing.
- `globalAlgorithmPresetRegistry()` returns a global singleton reference.
