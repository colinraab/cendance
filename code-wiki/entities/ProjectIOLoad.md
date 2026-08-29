---
title: ProjectIOLoad
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [project, serialization, json, load, save]
source_files:
  - Source/App/ProjectIOLoad.h
  - Source/App/ProjectIOLoad.cpp
---

# ProjectIOLoad

## Purpose
JSON serialization/deserialization for cendance project files. Handles saving `ProjectSnapshot` to disk and loading/parsing it back.

## Architecture
Extracted from ProjectIO.cpp during refactoring. Owns all JSON read/write logic:
- `saveProjectFile(snapshot, path, error)` — Serialize snapshot to JSON file
- `loadProjectFile(path, snapshot, error)` — Parse JSON file into snapshot
- `migrateLoadedSnapshotToCurrent(snapshot, major, minor, error)` — Schema migration (v1→current, v2 scaffold)

Anonymous namespace helpers:
- `readStringProperty`, `readBoolProperty`, `readUIntProperty`, `readFloatProperty` — JSON property readers
- `trimCopy`, `isNormalized01`, `isFiniteInRange` — Shared utilities (duplicated from ProjectIO.cpp anonymous namespace)
- `pushCommand` — Command queue helper (duplicated from ProjectIO.cpp anonymous namespace)

## Key Functions
- `saveProjectFile(snapshot, path, error)` — Validates, serializes to JSON via juce::DynamicObject, writes to disk
- `loadProjectFile(path, snapshot, error)` — Reads file, parses JSON, populates snapshot with bounds checking
- `migrateLoadedSnapshotToCurrent(...)` — Schema version migration scaffold

## Relationships
- **UsedBy:** [[ProjectIO]] (ProjectIO.h includes ProjectIOLoad.h)
- **UsedBy:** [[Main]], [[TuiAppProject]], [[PresetSerializer]] (all include ProjectIOLoad.h directly)
- **Uses:** [[AlgorithmCatalog]], [[AlgorithmPresetRegistry]], [[EffectPresetCatalog]], [[PresetRegistry]], [[SynthCatalog]], [[ChordProgression]]

## Thread Safety
- Called from main thread only. Not real-time.
- File I/O is synchronous.

## Gotchas
- `kProjectFormat = "cendanceProject"` — format string checked on load
- Schema migration: v1 passes through, v2 is a scaffold that normalizes to v1 representation
- JSON property readers use juce::DynamicObject (not raw string parsing)
- `juce::String` required for setProperty (not std::string)
- `juce::JSON::parse` returns `var` — check `isVoid()` before use
