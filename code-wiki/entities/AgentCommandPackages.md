---
title: AgentCommandPackages
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, packages, presets, p2p]
source_files:
  - Source/App/AgentCommandPackages.h
  - Source/App/AgentCommandPackages.cpp
---

# AgentCommandPackages

## Purpose
Handles contribution package and preset agent commands.

## Commands
- `packages list` — List installed packages
- `packages catalog` — Show contribution catalog
- `packages preview <path>` — Preview a package file
- `packages install <path>` — Install a package
- `packages remove <packageId>` — Remove a package
- `packages export <path> <kind> <packageId> <name>` — Export template
- `packages apply <kind> <packageId> <itemId> [track] [slot]` — Apply contribution
- `presets catalog` — Show merged preset catalog
- `presets apply <ref> [track] [slot]` — Apply preset by ref

## Internal Helpers
- `applyContribution` — Handles all contribution kinds (sound, effect, drumkit, arrangement, scene)

## Relationships
- **CalledBy:** [[AgentCommand]] (execute dispatcher)
- **Uses:** [[AgentCommandUtils]], [[PresetRegistry]], [[PresetRefs]], [[ContributionPackage]]
