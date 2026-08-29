---
title: AgentCommandState
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, state, catalog]
source_files:
  - Source/App/AgentCommandState.h
  - Source/App/AgentCommandState.cpp
---

# AgentCommandState

## Purpose
Handles state, catalog, meters, and listen agent commands.

## Commands
- `help` — Returns available command list
- `state` / `state full` → `executeState` — Returns JSON state snapshot
- `catalog algorithms|sounds|effects|presets` → `executeCatalog` — Returns catalog JSON
- `meters` → `executeMeters` — Returns current meter data
- `listen <seconds>` → `executeListen` — Returns meter heuristic analysis

## Internal Helpers
- `stateJson` — Builds full state JSON
- `catalogAlgorithmsJson`, `catalogSoundsJson`, `catalogEffectsJson` — Build catalog JSON
- `metersJson` — Builds meter JSON

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]]
