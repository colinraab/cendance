---
title: AppState
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [app-state, real-time, cross-thread]
source_files:
  - Source/App/AppState.h
---

# AppState

## Purpose
Atomic shared state accessible from all threads. Contains all mutable application parameters: BPM, play/stop, track states (algorithm, synth, density, complexity, tone, motion, gain, mute), arrangement state, project key, genre, spot effects bitmask.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `bpm` | `std::atomic<float>` | Tempo in BPM |
| `playing` | `std::atomic<bool>` | Transport playing state |
| `metronomeEnabled` | `std::atomic<bool>` | Metronome on/off |
| `chordProgression` | `std::atomic<uint8_t>` | Active chord progression ID |
| `genre` | `std::atomic<uint8_t>` | Active genre (0=none, 1-8=GenreCatalog ID) |
| `projectKeyRoot` | `std::atomic<uint8_t>` | Key root (0-11, C=0) |
| `projectKeyMode` | `std::atomic<uint8_t>` | Key mode (Major=0, NaturalMinor=1) |
| `arrangementSectionCount` | `std::atomic<uint8_t>` | Number of arrangement sections (1-8) |
| `arrangementCurrentSection` | `std::atomic<uint8_t>` | Currently active section index |
| `arrangementMode` | `std::atomic<uint8_t>` | Arrangement mode (Manual=0, Auto=1, Mixed=2) |
| `arrangementSectionLengths` | `std::atomic<uint8_t>[8]` | Section lengths in bars (1-16) |
| `arrangementSectionProgressions` | `std::atomic<uint8_t>[8]` | Per-section progression (0xFF=follow global) |
| `arrangementSectionTrackMasks` | `std::atomic<uint8_t>[8]` | Per-section active track bitmask |
| `arrangementSectionParametersEnabled` | `std::atomic<bool>` | Enable per-section track parameters |
| `arrangementSectionTrackParameters` | `std::atomic<float>[8][4][4]` | Per-section per-track parameters |
| `arrangementChainEnabled` | `std::atomic<bool>` | Enable arrangement chain |
| `arrangementChainLength` | `std::atomic<uint8_t>` | Chain length (1-8) |
| `arrangementChainSequence` | `std::atomic<uint8_t>[8]` | Chain step sequence |
| `tracks` | `TrackState[4]` | Per-track state (see TrackState) |
| `master` | `MasterState` | Master bus state (gain + 3 effect slots) |
| `activeSpotEffects` | `std::atomic<uint8_t>` | Bitmask of active spot effects |

### TrackState
| Member | Type | Description |
|--------|------|-------------|
| `algorithmId` | `std::atomic<uint16_t>` | Active algorithm ID |
| `synthPreset` | `std::atomic<uint8_t>` | Active synth preset ID |
| `density` | `std::atomic<float>` | Note density [0,1] |
| `complexity` | `std::atomic<float>` | Pattern complexity [0,1] |
| `tone` | `std::atomic<float>` | Tone parameter [0,1] |
| `motion` | `std::atomic<float>` | Motion parameter [0,1] |
| `muted` | `std::atomic<bool>` | Track mute |
| `synthManualOverride` | `std::atomic<bool>` | Manual synth preset override |
| `gain` | `std::atomic<float>` | Track gain |
| `effectPresetSlots` | `std::atomic<uint16_t>[3]` | Insert FX preset IDs |
| `drumSampleSlots` | `DrumSampleSlotState[4]` | Drum sample assignments (track 0 only) |

### DrumSampleSlotState
| Member | Type | Description |
|--------|------|-------------|
| `sampleId` | `std::atomic<uint16_t>` | Sample ID (0=unassigned) |
| `volume` | `std::atomic<float>` | Slot volume |
| `tuneSemitones` | `std::atomic<float>` | Pitch offset in semitones |
| `startOffset` | `std::atomic<float>` | Start offset (normalized) |
| `decay` | `std::atomic<float>` | Decay (normalized) |
| `velocitySensitivity` | `std::atomic<float>` | Velocity sensitivity |

## Relationships
- **UsedBy:** [[AudioEngine]] (reads all state), [[TuiApp]] (reads/writes via commands)
- **ReferencedBy:** [[Command]] (commands target AppState fields)

## Constants
- `kTrackCount = 4`
- `kArrangementMaxSections = 8`
- `kArrangementTrackParameterCount = 4`
- `DrumSampleSlotCount = 4`
- `kArrangementProgressionFollowGlobal = 0xFF`

## Thread Safety
- ALL members are `std::atomic`. No locks needed.
- Setters use `std::memory_order_relaxed` (no ordering constraints needed for independent atomics).
- Audio thread reads; UI thread writes via [[CommandQueue]].

## Gotchas
- `setGenre()` clamps to `GenreCatalog::kGenreCount` — always validate before setting.
- `setProjectKeyRoot()` wraps with `% 12`.
- `setArrangementSectionCount()` clamps current section if it exceeds new count.
- `TrackState::setDensity()` etc. use `memory_order_relaxed` — don't rely on ordering between different atomics.
