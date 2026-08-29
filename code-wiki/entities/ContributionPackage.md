---
title: ContributionPackage
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [p2p, packages, presets, data-model]
source_files:
  - Source/App/ContributionPackage.h
  - Source/App/ContributionPackage.cpp
---

# ContributionPackage

## Purpose
P2P contribution package model. Defines data structures for packages (effect presets, sound presets, drum kits, arrangements, scenes, sample packs) and the `Library` class for managing installed packages.

## Architecture
Split into two files:
- **ContributionPackage.h/.cpp** — Data structures (`Package`, `Preview`, `Library`, etc.) + public API (`kindToString`, `kindFromString`, `Library` class)
- **ContributionPackageParse.h/.cpp** — All JSON parsing/serialization logic (`parsePackage`, `readFile`, `validateIdText`, `quoted`, `appendPackageSummary`, etc.)

## Key Classes
| Class | Description |
|-------|-------------|
| `Package` | Full package metadata + item vectors |
| `Preview` | Parse result with ok/warnings/errors |
| `Library` | Manages installed packages (preview, install, remove, export, query) |

## Key Methods
- `Library::previewFile(path)` — Parse and validate a package file
- `Library::installFile(path, preview, error)` — Install a package
- `Library::removePackage(packageId, error)` — Remove an installed package
- `Library::reloadInstalled(error)` — Reload all installed packages
- `Library::exportPackageTemplate(path, kind, packageId, name, error)` — Export a template
- `Library::find*()` — Query methods for packages and items
- `Library::packagesJson(includeItems)` — Serialize installed packages
- `Library::contributionCatalogJson()` — Serialize contribution catalog

## Relationships
- **UsedBy:** [[AgentCommandPackages]] (via `ContributionPackage::Library*`)
- **Uses:** [[ContributionPackageParse]] (JSON parsing/serialization)
- **Uses:** [[PresetRefs]], [[EffectPresetCatalog]], [[AlgorithmCatalog]], [[SynthCatalog]], [[ProjectKey]], [[ChordProgression]]

## Thread Safety
- Called from main thread only. Not real-time.
- File I/O is synchronous.

## Gotchas
- `kSchema = "cendancePackage.v1"` — schema version string
- `kMasterTrackIndex = 4` — defined in [[AgentCommandUtils]]
- Anonymous namespace in parse file contains internal helpers (readString, readFloat, parseEffectItem, etc.)
