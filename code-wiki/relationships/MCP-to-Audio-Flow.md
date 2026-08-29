---
title: MCP-to-Audio-Flow
created: 2026-05-14
updated: 2026-05-14
type: relationship
tags: [mcp, agent-protocol, command]
---

# MCP to Audio Flow

## Path: MCP Tool Call → Audio Effect

```
External MCP Client (stdio)
    │
    ▼
McpServer::run()                  [MCP Thread]
    │
    ├─ readStdinLine() → JSON-RPC request
    ├─ dispatchRequest()
    │
    ▼
McpServer::handleToolsCall()
    │
    ├─ Extract tool name + arguments
    │
    ▼
Two paths:
    │
    ├─ Standard tools → executeFn_(commandString)
    │   │
    │   ▼
    │   AgentCommand::execute(input, context)  [Main Thread]
    │   │
    │   ├─ Parse command string
    │   ├─ Access AppState directly (main thread)
    │   ├─ dispatchCommand() callback → TuiApp::dispatchAndLog()
    │   │   │
    │   │   ▼
    │   │   CommandQueue::push() → AudioEngine (same as UI path)
    │   │
    │   └─ Return JSON response
    │
    └─ P2P tools → p2pFn_(toolName, argsJson)
        │
        ▼
        P2PClient operations
        │
        ├─ publishPreset/Sample/Algorithm
        ├─ requestPreset/Sample/Algorithm
        ├─ searchPresets/Samples/Algorithms
        └─ install/remove packages → AlgorithmPresetRegistry
```

## Key Differences from UI Path
- MCP uses text commands via [[AgentCommand::execute()]], same as TUI agent input
- MCP can also access [[AppState]] directly (main thread) for read-only operations
- P2P tools bypass AgentCommand and go directly to [[P2PClient]]
- Response format is JSON (MCP protocol), not human-readable text

## Related Pages
- [[McpServer]] — MCP server implementation
- [[AgentCommand]] — command execution engine
- [[P2PClient]] — P2P operations
- [[TuiApp]] — dispatchAndLog callback target
- [[UI-to-Audio-Flow]] — shared command path
