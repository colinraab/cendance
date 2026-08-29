---
title: AgentCommand
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [agent-protocol, command, mcp]
source_files:
  - Source/App/AgentCommand.h
  - Source/App/AgentCommand.cpp
---

# AgentCommand

## Purpose
Agent protocol command execution engine. Parses text commands and dispatches them as [[Command]] structs. Used by both the TUI (via [[TuiApp::executeAgentCommand]]) and MCP (via [[McpServer::executeFn_]]).

## Architecture
AgentCommand.cpp is now a **thin dispatcher** — the `execute()` function parses the verb and delegates to domain-specific handler files:

| Domain File | Commands |
|---|---|
| [[AgentCommandState]] | help, state, catalog, meters, listen |
| [[AgentCommandTransport]] | play, pause, stop, tempo |
| [[AgentCommandTrack]] | track (density, complexity, tone, motion, gain, algorithm, sound, mute, fx) |
| [[AgentCommandMaster]] | master (fx) |
| [[AgentCommandMusical]] | key, progression, arrangement |
| [[AgentCommandPackages]] | packages (list, preview, install, remove, catalog, export, apply), presets (catalog, apply) |
| [[AgentCommandUtils]] | Shared helpers (string utils, parsers, JSON builders, command dispatch, listen analysis) |

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `Response::ok` | `bool` | Success/failure |
| `Response::message` | `std::string` | Human-readable result |
| `Response::json` | `std::string` | JSON response (for MCP) |
| `ExecutionContext::appState` | `AppState&` | Shared state access |
| `ExecutionContext::currentMeters` | `MeterData&` | Current metering snapshot |
| `ExecutionContext::meterHistory` | `vector<MeterData>&` | Historical metering |
| `ExecutionContext::dispatchCommand` | `DispatchCommandFn` | Command dispatch callback |
| `ExecutionContext::contributionLibrary` | `ContributionPackage::Library*` | P2P library pointer |

## Relationships
- **UsedBy:** [[TuiApp]] (agent input bar)
- **UsedBy:** [[McpServer]] (via executeFn_ callback)
- **Dispatches to:** [[CommandQueue]] (via dispatchCommand callback)
- **Delegates to:** [[AgentCommandState]], [[AgentCommandTransport]], [[AgentCommandTrack]], [[AgentCommandMaster]], [[AgentCommandMusical]], [[AgentCommandPackages]]

## Behavior
- `execute(input, context)` — Parse command string, route to domain handler, return Response.
- Commands are text-based (e.g., "state full", "play", "set track 0 density 0.7").
- Response includes both human-readable message and JSON for MCP.

## Thread Safety
- Called from main thread (TUI) or MCP thread. Not real-time.
- Accesses [[AppState]] directly (main thread only).
- Dispatches commands via callback to [[TuiApp::dispatchAndLog]].

## Gotchas
- All domain handlers live in the `AgentCommand` namespace.
- `kMasterTrackIndex` constant is defined in [[AgentCommandUtils]].
- Anonymous namespaces in .cpp files contain file-local helpers (e.g., `stateJson`, `catalogAlgorithmsJson`).
