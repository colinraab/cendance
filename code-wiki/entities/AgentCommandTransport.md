---
title: AgentCommandTransport
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, transport]
source_files:
  - Source/App/AgentCommandTransport.h
  - Source/App/AgentCommandTransport.cpp
---

# AgentCommandTransport

## Purpose
Handles transport agent commands: play, pause, stop, tempo.

## Commands
- `play` → `executePlay`
- `pause` → `executePause`
- `stop` → `executeStop`
- `tempo set <bpm>` / `tempo +/-<delta>` → `executeTempo`

## Internal Helpers
- `executePlayPause` — Shared logic for play/pause

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]]
