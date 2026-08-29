---
title: TuiApp
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [ui, main-thread, ftxui]
source_files:
  - Source/UI/TuiApp.h
  - Source/UI/TuiApp.cpp
  - Source/UI/TuiAppRender.cpp
  - Source/UI/TuiAppInput.cpp
  - Source/UI/TuiAppDrumCommands.cpp
  - Source/UI/TuiAppNumberSelection.cpp
  - Source/UI/TuiAppArrangement.cpp
  - Source/UI/TuiAppProject.cpp
  - Source/UI/TuiAppProject.h
---

# TuiApp

## Purpose
Main terminal UI application. Runs the FTXUI `ScreenInteractive::Loop()` on the main thread. Handles keyboard input, renders the terminal UI, dispatches commands to [[CommandQueue]], reads metering from [[MeterQueue]], and manages all modal dialogs.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `appState` | `AppState&` | Shared atomic state reference |
| `cmdQueue` | `CommandQueue&` | Command queue for dispatching to audio |
| `meterQueue` | `MeterQueue&` | Metering queue for reading from audio |
| `drumSampleLibrary` | `DrumSampleLibrary*` | Drum sample library pointer |
| `contributionLibrary` | `ContributionPackage::Library*` | P2P contribution library |
| `p2pClient` | `P2PClient*` | P2P client pointer |
| `presetSerializer` | `PresetSerializer*` | Preset serializer pointer |
| `agentServer` | `unique_ptr<AgentProtocolServer>` | TCP agent protocol server |
| `selectedTrack` | `int` | Currently selected track (0-3) |
| `showHelp` | `bool` | Help popup visible |
| `undoStack` | `deque<UndoAction>` | Undo history (max 50) |
| `pendingAgentRequests` | `deque<shared_ptr<PendingAgentProtocolRequest>>` | Agent protocol request queue |

## Modal State
TuiApp manages these modal dialogs:
- **NumberSelection** — Algorithm/synth/parameter selection
- **KeySelection** — Project key selection
- **ArrangementModal** — Song arrangement editing
- **ProjectPathModal** — Save/load project paths
- **DrumSampleModal** — Drum sample import/assignment
- **SoundFileBrowser** — P2P downloaded sample browser
- **AlgorithmEditor** — Custom algorithm preset editor
- **ToSModal** — Terms of service acceptance

## Relationships
- **Uses:** [[AppState]], [[CommandQueue]], [[MeterQueue]], [[AgentCommand]], [[AgentProtocolServer]], [[P2PClient]], [[PresetSerializer]], [[DrumSampleLibrary]]
- **Creates:** [[AgentProtocolServer]] (if agentPort > 0)
- **Dispatches to:** [[AudioEngine]] (via CommandQueue)

## Behavior
- `run()` — Main FTXUI event loop
- `dispatchAndLog()` — Push command to queue + add to undo stack
- `performUndo()` — Pop undo stack and dispatch reverse command
- `executeAgentCommand()` — Parse and execute agent protocol command
- `buildUI()` — Construct FTXUI element tree from current state
- `handleEventInput()` — Process keyboard events
- `openAlgorithmEditor()` / `closeAlgorithmEditor()` — Algorithm editor modal
- `refreshCachedCustomAlgorithms()` — Reload custom algorithm list from registry

## Thread Safety
- Lives on the **Main Thread** (UI). Never touches audio thread directly.
- All cross-thread communication via [[CommandQueue]] and [[MeterQueue]].
- Agent protocol requests use mutex + condition_variable (not real-time).

## Gotchas
- Undo stack max 50 entries. Oldest dropped when full.
- Modal dialogs are mutually exclusive — only one open at a time.
- `dispatchAndLog()` takes both the command and its undo command.
- Agent input bar uses `AgentCommand::execute()` which accesses `AppState` directly (main thread only).
