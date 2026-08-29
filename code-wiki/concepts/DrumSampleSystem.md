---
title: DrumSampleSystem
created: 2026-05-14
updated: 2026-05-14
type: concept
tags: [drum, sample, preset]
---

# Drum Sample System

## What It Is
Drum track (track 0) supports 4 drum sample slots. Each slot can be assigned a sample with individual volume, tune, start offset, decay, and velocity sensitivity parameters. Samples are loaded from OGG files embedded as binary data or imported from disk/P2P.

## Key Concepts

### Sample Slots
- 4 slots per track (`DrumSampleSlotCount = 4`)
- Track 0 only (drum track)
- Each slot: sampleId, volume, tuneSemitones, startOffset, decay, velocitySensitivity
- State in [[AppState::TrackState::DrumSampleSlotState[4]]]

### Sample Libraries
- [[DrumSampleLibrary]] — manages drum sample files on disk
- [[MelodicSampleLibrary]] — manages melodic sample files on disk
- Binary data: `DrumKitBinaryData.h` (embedded OGG files)
- Binary data: `MelodicSampleBinaryData.h` (embedded OGG files)

### Sample Assignment
- Via TUI drum sample modal (`openDrumSampleModal()`)
- Via P2P download and import
- Commands: `SetDrumSampleAssignment`, `ClearDrumSampleAssignment`

### Sample Parameters
| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `volume` | float | [0, 1] | Slot volume |
| `tuneSemitones` | float | any | Pitch offset in semitones |
| `startOffset` | float | [0, 1] | Start position (normalized) |
| `decay` | float | [0, 1] | Decay envelope |
| `velocitySensitivity` | float | [0, 1] | Velocity response |

### Drum Synthesis
- [[DrumEngine]] handles drum sample playback
- Triggered by MIDI from drum generators (FourOnFloor, Breakbeat, etc.)
- Samples loaded from [[DrumSampleLibrary]] or embedded binary data

## Involved Classes
- [[AppState]] — slot state (atomic)
- [[DrumSampleLibrary]] — sample file management
- [[MelodicSampleLibrary]] — melodic sample management
- [[DrumEngine]] — drum synthesis
- [[TuiApp]] — drum sample modal
- [[AudioEngine]] — `applyDrumKitPreset()`, `applyMelodicSamplePreset()`

## Commands
| Command | Description |
|---------|-------------|
| `SetDrumSampleAssignment` | Assign sample to slot |
| `ClearDrumSampleAssignment` | Clear slot |
| `SetDrumSampleVolume` | Set slot volume |
| `SetDrumSampleTune` | Set slot tune |
| `SetDrumSampleStartOffset` | Set start offset |
| `SetDrumSampleDecay` | Set decay |
| `SetDrumSampleVelocitySensitivity` | Set velocity sensitivity |

## Gotchas
- Drum sample slots are track 0 only
- `sampleId == 0` means unassigned
- Sample import copies files to the drum sample directory
- P2P samples go through [[P2PClient]] → download registry → import
