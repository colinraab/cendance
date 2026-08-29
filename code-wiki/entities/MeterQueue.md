---
title: MeterQueue
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [lock-free, spsc-queue, cross-thread, real-time]
source_files:
  - Source/App/MeterQueue.h
---

# MeterQueue

## Purpose
Lock-free SPSC queue for sending metering data from the audio thread to the UI thread. Capacity 64. UI polls at ~30fps and keeps only the latest data.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `buffer` | `std::array<MeterData, 64>` | Circular buffer |
| `writePos` | `alignas(64) std::atomic<size_t>` | Write index (audio thread) |
| `readPos` | `alignas(64) std::atomic<size_t>` | Read index (UI thread) |

## Relationships
- **WrittenBy:** [[AudioEngine]] (audio thread)
- **ReadBy:** [[TuiApp]] (UI thread)
- **Contains:** [[MeterData]] structs

## Behavior
- `push(const MeterData&)` — Audio thread. Returns false if full.
- `popLatest(MeterData&)` — UI thread. Drains all available, keeps most recent. Accumulates active notes via OR.

## Thread Safety
- Same SPSC pattern as [[CommandQueue]] but smaller capacity (64 vs 256).
- Cache-line aligned atomics.

---

## MeterData (nested in MeterQueue.h)

| Field | Type | Description |
|-------|------|-------------|
| `trackLevels[4]` | `float` | RMS per track |
| `masterLevel` | `float` | Master RMS |
| `beatPosition` | `uint32_t` | Current beat in bar (0-3) |
| `barNumber` | `uint16_t` | Current bar |
| `activeAlgorithm[4]` | `uint16_t` | Active algorithm per track |
| `isPlaying` | `bool` | Transport state |
| `analyzerValid` | `bool` | Spectrum data valid |
| `analyzerFrame` | `uint32_t` | Spectrum frame counter |
| `activeNotes[4][2]` | `uint64_t` | Active note bitmask per track |
| `spectrumBins[32]` | `float` | Spectrum analyzer bins |
| `performanceProfileValid` | `bool` | Profiling data valid |
| `profileWindowCallbacks` | `uint32_t` | Profiling window size |
| `profileBufferDurationMs` | `float` | Buffer duration |
| `callbackMsAvg/Peak` | `float` | Callback timing |
| `callbackUtilizationAvg/Peak` | `float` | CPU utilization |
| `commandsMsAvg` | `float` | Command processing time |
| `generationMsAvg` | `float` | MIDI generation time |
| `trackFxMsAvg` | `float` | Track FX processing time |
| `masterFxMsAvg` | `float` | Master FX processing time |
| `meteringMsAvg` | `float` | Metering time |

### Constants
- `kOscilloscopeSampleCount = 96`
- `kSpectrumBinCount = 32`
