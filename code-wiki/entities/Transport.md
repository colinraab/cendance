---
title: Transport
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [audio, real-time]
source_files:
  - Source/Audio/Transport.h
  - Source/Audio/Transport.cpp
---

# Transport

## Purpose
BPM clock and playhead position tracker. Advances on every audio block. Provides beat/bar boundaries for the sequencer and arrangement system.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `sampleRate_` | `double` | Current sample rate |
| `bpm_` | `float` | Current BPM |
| `samplesPerBeat_` | `double` | Samples per beat (derived from BPM + sample rate) |
| `samplePosition_` | `double` | Current position in samples |
| `lastBeat_` | `int` | Last beat index (-1 initially) |
| `lastBar_` | `int` | Last bar index (-1 initially) |
| `newBeat_` | `bool` | Beat boundary crossed this block |
| `newBar_` | `bool` | Bar boundary crossed this block |

## Relationships
- **OwnedBy:** [[AudioEngine]]
- **UsedBy:** [[AudioEngine]] (for sequencer timing, arrangement advancement)

## Behavior
- `prepare(sampleRate)` — Set sample rate, recalculate `samplesPerBeat_`
- `advance(numSamples)` — Called every audio block. Updates position, detects beat/bar boundaries.
- `reset()` — Reset position to 0
- `getPlayheadPosition()` — Position in beats (fractional)
- `getCurrentBeat()` — Beat within bar (0-3)
- `getCurrentBar()` — Absolute bar number
- `isNewBeat()` / `isNewBar()` — True if boundary crossed this block
- `setBpm(bpm)` — Update BPM, recalculate `samplesPerBeat_`

## Thread Safety
- Lives on the **Audio Thread** only. No atomics needed.
- BPM changes from UI go through [[CommandQueue]] → [[AudioEngine]] → `setBpm()`.

## Gotchas
- 4/4 time signature assumed (4 beats per bar).
- `newBeat_` and `newBar_` are sticky for the duration of one audio block.
- `setBpm()` recalculates `samplesPerBeat_` immediately — safe on audio thread since it's just a float multiply.
