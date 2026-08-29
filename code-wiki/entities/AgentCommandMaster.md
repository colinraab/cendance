---
title: AgentCommandMaster
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, master]
source_files:
  - Source/App/AgentCommandMaster.h
  - Source/App/AgentCommandMaster.cpp
---

# AgentCommandMaster

## Purpose
Handles master bus agent commands.

## Commands
- `master fx <slot 1-3> <effect id>` — Set master effect

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]], [[EffectPresetCatalog]]
