---
title: AgentCommandUtils
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, utility, helpers]
source_files:
  - Source/App/AgentCommandUtils.h
  - Source/App/AgentCommandUtils.cpp
---

# AgentCommandUtils

## Purpose
Shared utility functions for the AgentCommand system. Used by all domain handler files.

## Key Functions
- **String:** `trimCopy`, `lowerCopy`, `jsonEscape`, `quoted`
- **Parsing:** `tokenize`, `parseFloat`, `parseUInt`, `parseDurationSeconds`
- **Response:** `makeResponse`
- **Display:** `trackName`, `arrangementModeName`, `commandHelpJson`
- **Dispatch:** `dispatch`, `dispatchCommandList`
- **Listen:** `analyzeMeterHistory`, `listenJson`, `ListenScores`
- **Presets:** `refreshPresetRegistry`
- **Builders:** `appendMacroCommands`, `appendTrackFxCommands`, `appendMasterFxCommands`, `appendArrangementCommands`

## Constants
- `kMasterTrackIndex = 4` — Master bus track index

## Relationships
- **UsedBy:** All AgentCommand domain files
