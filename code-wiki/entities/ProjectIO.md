---
title: ProjectIO
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [project, save, load, serialization]
source_files:
  - Source/App/ProjectIO.h
  - Source/App/ProjectIO.cpp
---

# ProjectIO

## Purpose
Project file save/load system. Manages project snapshots, validation, applying to command queue, and path/recent file management.

## Architecture
Split into two files:
- **ProjectIO.h/.cpp** — Core types (`ProjectSnapshot`, `TrackSnapshot`, etc.), `snapshotFromState`, `validateSnapshot`, `applySnapshotToCommandQueue`, path helpers, recent projects
- **ProjectIOLoad.h/.cpp** — JSON serialization/deserialization (`saveProjectFile`, `loadProjectFile`) with anonymous namespace helpers

## Key Functions
- `snapshotFromState(appState)` — Create snapshot from current app state
- `validateSnapshot(snapshot, error)` — Validate snapshot integrity
- `applySnapshotToCommandQueue(snapshot, appState, commandQueue, error, forceStop)` — Apply snapshot via commands
- `loadProjectFile(path, snapshot, error)` — Load from JSON (in ProjectIOLoad)
- `saveProjectFile(snapshot, path, error)` — Save to JSON (in ProjectIOLoad)
- `normalizeProjectPath(input, output, error)` — Normalize project file paths
- `getDefaultProjectsDirectory()` — Get default projects directory
- `loadRecentProjectPaths(error)` / `saveRecentProjectPaths(paths, error)` — Recent projects

## Relationships
- **UsedBy:** [[Main]] (load/save), [[TuiAppProject]] (UI save/load), [[PresetSerializer]] (preset export)
- **Uses:** [[ProjectIOLoad]] (JSON serialization)
- **Uses:** [[AlgorithmCatalog]], [[AlgorithmPresetRegistry]], [[EffectPresetCatalog]], [[PresetRegistry]], [[SynthCatalog]], [[ChordProgression]]

## Thread Safety
- Called from main thread only. Not real-time.
- File I/O is synchronous.

## Gotchas
- `kProjectSchemaMajor = 1`, `kProjectSchemaMinor = 8` — schema version
- `kProjectForwardCompatibleSchemaMajor = 2` — forward compatibility
- Anonymous namespace in ProjectIOLoad.cpp contains JSON property readers (readStringProperty, readFloatProperty, etc.)
- `pushCommand` helper is duplicated in both ProjectIO.cpp and ProjectIOLoad.cpp anonymous namespaces
