---
title: GenerativeAlgorithm
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [audio, real-time, generator]
source_files:
  - Source/Audio/Generators/GenerativeAlgorithm.h
---

# GenerativeAlgorithm

## Purpose
Abstract base class for all MIDI generative algorithms. Each algorithm generates MIDI notes for a track based on current parameters (density, complexity, scale, etc.).

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| (pure virtual) | — | All methods are pure virtual |

## Relationships
- **InheritedBy:** All 20+ built-in generators (FourOnFloor, Breakbeat, EuclideanRhythm, WalkingBass, SyncBass, BlockChords, SyncStabs, Arpeggiator, MarkovMelody, DrumStyleAlgorithms, BassStyleAlgorithms, ChordStyleAlgorithms, LeadStyleAlgorithms, and ~60 more style-specific algorithms)
- **AlsoInheritedBy:** [[CustomAlgorithmInstance]]
- **UsedBy:** [[AudioEngine]] (calls `processMidi()` and `reset()`)

## Behavior
- `processMidi(buffer, playheadBeats, blockLengthBeats, blockSamples, scale, rootNote, density, complexity)` — Fill a MIDI buffer with generated notes for this audio block.
- `reset()` — Reset algorithm state (e.g., random seeds, pattern position).
- `getName()` — Return human-readable algorithm name.

## Thread Safety
- Called on the **Audio Thread**. No allocations, no mutexes.
- Each algorithm instance is owned by exactly one track.

## Gotchas
- `processMidi` receives the current `Scale` object — algorithms must respect it.
- `density` and `complexity` are [0.0, 1.0] floats from [[AppState]].
- `rootNote` is the MIDI note number of the scale root.
- Algorithms must be deterministic given the same inputs (for testing).
