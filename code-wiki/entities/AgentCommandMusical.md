---
title: AgentCommandMusical
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, musical]
source_files:
  - Source/App/AgentCommandMusical.h
  - Source/App/AgentCommandMusical.cpp
---

# AgentCommandMusical

## Purpose
Handles musical agent commands: key, chord progression, arrangement.

## Commands
- `key "<note> <mode>"` → `executeKey` — Set project key
- `progression <id>` → `executeProgression` — Set chord progression
- `arrangement section <id>` → `executeArrangement` — Set arrangement section

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]], [[ProjectKey]], [[ChordProgression]]
