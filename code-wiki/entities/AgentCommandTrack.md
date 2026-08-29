---
title: AgentCommandTrack
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, track]
source_files:
  - Source/App/AgentCommandTrack.h
  - Source/App/AgentCommandTrack.cpp
---

# AgentCommandTrack

## Purpose
Handles track parameter agent commands.

## Commands
- `track <1-4> density|complexity|tone|motion|gain <value>` — Track parameters
- `track <1-4> algorithm <id>` — Set algorithm
- `track <1-4> sound <id>` — Set synth preset
- `track <1-4> mute on|off` — Toggle mute
- `track <1-4> fx <slot 1-3> <effect id>` — Set track effect

## Internal Helpers
- `parseTrackNumber` — Validates and converts track display index

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]], [[AlgorithmCatalog]], [[SynthCatalog]], [[EffectPresetCatalog]]
