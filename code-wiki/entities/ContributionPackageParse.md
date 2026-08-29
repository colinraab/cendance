---
title: ContributionPackageParse
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [p2p, packages, parsing, json]
source_files:
  - Source/App/ContributionPackageParse.h
  - Source/App/ContributionPackageParse.cpp
---

# ContributionPackageParse

## Purpose
JSON parsing and serialization for contribution packages. Extracted from [[ContributionPackage]] to separate parsing logic from data model/operations.

## Public API
- `parsePackage(text, sourcePath, preview)` — Parse a package from JSON text
- `readFile(path, error)` — Read a package file with size limits
- `defaultRootDirectory()` — Get the default contributions directory
- `packageFileName(packageId)` → Generate safe filename from package ID
- `validateIdText(text, label, error)` — Validate package/item ID format
- `quoted(text)` — JSON string escaping
- `lowerCopy(text)` — Case-insensitive string copy
- `appendJsonStringArray(out, values)` — Serialize string array to JSON
- `appendPackageSummary(out, package, includeItems)` — Serialize package summary

## Internal Helpers (not in header)
- JSON field readers: `readString`, `readFloat`, `readUInt`, `readDouble`, `readBool`, `readStringArray`, `readOptionalFloat`
- Item parsers: `parseEffectItem`, `parseSoundItem`, `parseDrumKitItem`, `parseArrangementItem`, `parseSceneItem`, `parseSamplePackItem`
- Validation: `validateCommonItem`, `parseTrackIndex`, `parseFxSlots`, `parseMacros`, `parseArrangementFields`
- Hashing: `fnv1a64`, `canonicalForHash`
- Effect type conversion: `effectTypeFromString`, `effectTypeToString`

## Relationships
- **UsedBy:** [[ContributionPackage]] (Library class methods)
