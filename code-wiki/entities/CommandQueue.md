---
title: CommandQueue
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [lock-free, spsc-queue, cross-thread, real-time]
source_files:
  - Source/App/CommandQueue.h
---

# CommandQueue

## Purpose
Lock-free single-producer single-consumer (SPSC) queue for sending commands from the UI thread to the audio thread. Fixed capacity of 256 commands. Never blocks.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `buffer` | `std::array<Command, 256>` | Circular buffer |
| `writePos` | `alignas(64) std::atomic<size_t>` | Write index (UI thread) |
| `readPos` | `alignas(64) std::atomic<size_t>` | Read index (audio thread) |

## Relationships
- **WrittenBy:** [[TuiApp]] (UI thread, via `push()`)
- **ReadBy:** [[AudioEngine]] (audio thread, via `pop()`)
- **Contains:** [[Command]] structs

## Behavior
- `push(const Command&)` — Called from UI thread. Returns false if full.
- `pop(Command&)` — Called from audio thread. Returns false if empty.
- Capacity: 256 commands.

## Thread Safety
- SPSC: single producer (UI), single consumer (audio).
- Uses `memory_order_acquire`/`memory_order_release` for synchronization.
- Cache-line aligned atomics (64-byte alignment) to prevent false sharing.
- NO mutexes, NO allocations.

## Gotchas
- If queue full, command is silently dropped (returns false). UI should handle this.
- Command struct is POD-like; ensure it stays trivially copyable.

---

## Command (nested in CommandQueue.h)

### Command::Type Enum
All command types the UI can send to the audio thread:

| Type | Description |
|------|-------------|
| `SetAlgorithm` | Set track algorithm ID |
| `StepAlgorithm` | Step algorithm +/-1 with wrap |
| `SetDensity/SetDensityAbsolute` | Set track density [0,1] |
| `SetComplexity/SetComplexityAbsolute` | Set track complexity [0,1] |
| `SetSynthPreset/StepSynthPreset` | Set/step synth preset |
| `SetTone/SetToneAbsolute` | Set track tone [0,1] |
| `SetMotion/SetMotionAbsolute` | Set track motion [0,1] |
| `SetTrackGain/SetTrackGainAbsolute` | Set track gain (track 4 = master) |
| `SetChordProg` | Set chord progression ID |
| `SetGenre/RandomizeForGenre` | Set genre or randomize for genre |
| `SetArrangementSectionCount` | Set number of sections |
| `SetArrangementSection/StepArrangementSection` | Set/step current section |
| `SetArrangementMode/StepArrangementMode` | Set/step arrangement mode |
| `SetArrangementSectionLength` | Set section length in bars |
| `SetArrangementSectionProgression` | Set section progression |
| `SetArrangementSectionTrackMask` | Set section active track mask |
| `SetArrangementSectionParametersEnabled` | Enable per-section parameters |
| `SetArrangementSectionTrackParameter` | Set per-section per-track parameter |
| `SetArrangementChainEnabled/Length/Step` | Arrangement chain control |
| `SetProjectKey` | Set key root + mode |
| `SetTempo/SetTempoAbsolute` | Set BPM |
| `RebuildCustomAlgorithms` | Rebuild custom algorithm instances |
| `PlayStop/Stop` | Transport control |
| `ToggleMetronome` | Toggle metronome |
| `ToggleTrackMute` | Toggle track mute |
| `SetTrackEffectPreset` | Set track insert FX preset |
| `SetMasterEffectPreset` | Set master insert FX preset |
| `SpotEffectOn/Off/Toggle` | Spot FX trigger control |
| `SetDrumSampleAssignment` | Assign drum sample to slot |
| `ClearDrumSampleAssignment` | Clear drum slot |
| `SetDrumSampleVolume/Tune/StartOffset/Decay/VelocitySensitivity` | Drum slot parameters |

### Encoding Helpers
- `encodeEffectSlotPreset(slot, preset)` / `decodeEffectSlotIndex/Id(payload)` — Pack slot+preset into uint16
- `encodeDrumSlotSampleId(slot, sampleId)` / decode — Pack drum slot+sample
- `encodeProjectKey(root, mode)` / decode — Pack key root+mode
- `encodeArrangementSectionValue(section, value)` / decode — Pack section+value

### SpotEffectId
- `TapeBrake = 0`, `Stutter = 1`
