---
title: ArrangementSystem
created: 2026-05-14
updated: 2026-05-14
type: concept
tags: [arrangement, transport, real-time]
---

# Arrangement System

## What It Is
Song arrangement with up to 8 sections. Each section has a length, progression, active track mask, and optional per-track parameters. Supports linear wrap and chain playback modes.

## Key Concepts

### Sections
- Up to 8 sections (`kArrangementMaxSections = 8`)
- Each section: length (1-16 bars), progression ID, track mask (4-bit), per-track parameters
- Section lengths stored in `AppState::arrangementSectionLengths[8]`
- Current section in `AppState::arrangementCurrentSection`

### Modes
- **Manual (0)**: User controls section changes
- **Auto (1)**: Automatic progression through sections
- **Mixed (2)**: Auto with manual override capability

### Chain
- Optional chain sequence for non-linear playback
- `arrangementChainSequence[8]` maps chain position → section index
- `arrangementChainLength` (1-8) controls chain size
- `arrangementChainEnabled` toggles between linear wrap and chain

### Per-Section Parameters
- `arrangementSectionParametersEnabled` — global toggle
- `arrangementSectionTrackParameters[8][4][4]` — per-section per-track parameters
- 4 parameters per track per section

### Progression
- `arrangementSectionProgressions[8]` — per-section chord progression
- `kArrangementProgressionFollowGlobal (0xFF)` — use global progression

## Involved Classes
- [[AppState]] — all arrangement state (atomic)
- [[AudioEngine]] — `updateArrangementState()`, `getArrangementProgressionStep()`
- [[TuiApp]] — arrangement modal, command dispatch
- [[Transport]] — bar boundaries for arrangement advancement

## Commands
| Command | Description |
|---------|-------------|
| `SetArrangementSectionCount` | Set number of sections |
| `SetArrangementSection` / `StepArrangementSection` | Set/step current section |
| `SetArrangementMode` / `StepArrangementMode` | Set/step playback mode |
| `SetArrangementSectionLength` | Set section length in bars |
| `SetArrangementSectionProgression` | Set section chord progression |
| `SetArrangementSectionTrackMask` | Set active tracks for section |
| `SetArrangementSectionParametersEnabled` | Enable per-section parameters |
| `SetArrangementSectionTrackParameter` | Set per-section per-track parameter |
| `SetArrangementChainEnabled` | Toggle chain mode |
| `SetArrangementChainLength` | Set chain length |
| `SetArrangementChainStep` | Set chain step mapping |

## Gotchas
- Section count clamped to [1, 8]. Current section clamped to [0, count-1].
- Section length clamped to [1, 16] bars.
- Chain indices clamped to [0, kArrangementMaxSections-1].
- Arrangement state advances on bar boundaries (from [[Transport::isNewBar()]]).
- `arrangementAnchorInitialized` flag in [[AudioEngine]] — arrangement state latches on first bar after play starts.
