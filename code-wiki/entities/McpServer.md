---
title: McpServer
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [mcp, stdio, network]
source_files:
  - Source/Mcp/McpServer.h
  - Source/Mcp/McpServer.cpp
  - Source/Mcp/McpTools.cpp
  - Source/Mcp/McpResources.cpp
  - Source/Mcp/McpPrompts.cpp
  - Source/Mcp/McpStaticData.cpp
  - Source/Mcp/McpStdio.cpp
  - Source/Mcp/McpJsonHelpers.h
  - Source/Mcp/McpJsonHelpers.cpp
---

# McpServer

## Purpose
Embeds the full cendance-mcp-server toolset inside the cendance binary. Implements MCP (Model Context Protocol) JSON-RPC 2.0 over stdio. Dispatches tool calls to the same [[AgentCommand::execute()]] pipeline the TUI uses.

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `executeFn_` | `ExecuteFn` (function<String(String)>) | Maps agent command string to JSON response |
| `p2pFn_` | `P2PFn` (function<String(String,String)>) | Handles P2P-specific MCP tools |
| `running_` | `std::atomic<bool>` | Server running state |
| `stopping_` | `std::atomic<bool>` | Server stopping flag |

## Relationships
- **Uses:** [[AgentCommand]] (via executeFn_)
- **UsedBy:** External MCP clients (stdio transport)
- **Creates responses via:** [[McpJsonHelpers]]

## MCP Tools (32 total)
| Tool | Description |
|------|-------------|
| `get_cendance_state` | Full state snapshot |
| `get_cendance_catalog` | Algorithm/synth/effect catalogs |
| `get_cendance_preset_catalog` | Preset catalog |
| `get_cendance_meters` | Current metering data |
| `listen_to_cendance` | Subscribe to meter updates |
| `play_cendance` / `pause_cendance` / `stop_playback` | Transport control |
| `set_tempo` | Set BPM |
| `set_track_parameter` | Set density/complexity/tone/motion |
| `set_track_algorithm` | Set algorithm ID |
| `set_track_sound` | Set synth preset |
| `set_track_mute` | Toggle mute |
| `set_track_fx` | Set track insert FX |
| `set_master_fx` | Set master insert FX |
| `set_key` | Set project key |
| `set_progression` | Set chord progression |
| `set_arrangement_section` | Set arrangement section |
| `list_cendance_packages` | List installed P2P packages |
| `get_cendance_contribution_cat` | Contribution catalog |
| `preview_cendance_package` | Preview package contents |
| `install_cendance_package` | Install P2P package |
| `remove_cendance_package` | Remove package |
| `export_cendance_package` | Export package |
| `apply_cendance_contribution` | Apply contribution |
| `apply_cendance_preset_ref` | Apply preset reference |
| `set_track_sound_ref` | Set sound by reference |
| `set_track_fx_ref` / `set_master_fx_ref` | Set FX by reference |
| `send_cendance_command` | Raw protocol command |
| `get_cendance_agent_authoring_guide` | Authoring guide |
| `get_cendance_package_schema` | Package schema |

## MCP Resources
- `manual` — Embedded user manual (from `Resources/manual.md`)

## MCP Prompts
- `start_project` — Project initialization prompt
- `analyze_improve` — Analysis and improvement prompt
- `design_package` — Package design prompt
- `genre_randomize` — Genre randomization prompt

## Behavior
- `run()` — Read JSON-RPC requests from stdin, dispatch, write responses to stdout.
- `stop()` — Signal server to stop.
- `dispatchRequest()` — Route to appropriate handler (tools/list, tools/call, resources/*, prompts/*, initialize).

## Thread Safety
- Runs on its own thread (or the message thread). Not real-time.
- Uses `ExecuteFn` callback to dispatch commands — this callback must be thread-safe.

## Gotchas
- stdio transport: reads via `readStdinLine()`, writes via `writeStdout()`.
- JSON-RPC 2.0 format with `Content-Length` header.
- P2P tools (publish/search/download) go through `p2pFn_` which has access to SecurityManager/P2PClient.
- All other tools go through `executeFn_` → [[AgentCommand::execute()]].
