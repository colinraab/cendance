# Pilot Protocol — Agent-to-Agent Wire Format Specification

> Archived on 2026-09-02. Despite its title, this document specifies an early
> cendance-local protocol and is not the external Pilot Protocol wire format.
> cendance has no current Pilot integration. See
> [../SHARING_ARCHITECTURE.md](../SHARING_ARCHITECTURE.md).

## Overview

The pilot protocol is a line-delimited JSON protocol for agent-to-agent communication between cendance instances. It enables remote agents to discover peers, share presets, samples, and algorithms over LAN or WAN.

## Transport

### TCP Line Protocol
- **Default binding**: `127.0.0.1:<port>` (loopback only, configurable via `--agent-port`)
- **Format**: One JSON request per line (`\n` delimited), one JSON response per line
- **Encoding**: UTF-8
- **Max line length**: 4096 bytes
- **Connection timeout**: 30 seconds idle

### HTTP API (Rendezvous Server)
When configured with a remote endpoint (`CENDANCE_P2P_ENDPOINT`), cendance uses HTTP:

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/publish` | Publish a signed envelope |
| GET | `/api/v1/presets` | List available presets |
| GET | `/api/v1/presets/{id}` | Fetch a specific preset |
| GET | `/api/v1/samples` | List available samples |
| GET | `/api/v1/samples/{id}` | Fetch a specific sample |
| GET | `/api/v1/algorithms` | List available algorithms |
| GET | `/api/v1/algorithms/{id}` | Fetch a specific algorithm |

### mDNS Discovery (LAN)
- **Service type**: `_cendance._tcp`
- **Broadcast port**: 40500
- **Advertisement interval**: 1.5 seconds
- **Fields**: instance ID, description, IP address, TCP port

## Message Format

### Request
```json
{
  "id": "<optional correlation id>",
  "method": "<method_name>",
  "params": { ... }
}
```

### Response
```json
{
  "id": "<correlation id from request>",
  "ok": true,
  "result": { ... }
}
```

### Error Response
```json
{
  "id": "<correlation id from request>",
  "ok": false,
  "error": {
    "code": <int>,
    "message": "<error description>"
  }
}
```

## Agent Protocol Commands

The TCP line protocol uses a simpler text-based format. Each line is a command:

### P2P Commands

#### `p2p` — P2P Status
Returns the P2P bridge status and available subcommands.

**Request**: `p2p`
**Response**:
```json
{
  "ok": true,
  "message": "P2P bridge available. Commands: p2p.search, p2p.publish, p2p.download, p2p.verify, p2p.peers"
}
```

#### `p2p.search` — Search Network
Search the P2P network for available content.

**Request**: `p2p.search <type> [query]`
- `type`: `presets`, `samples`, or `algorithms`
- `query`: optional search string

**Response**:
```json
{
  "ok": true,
  "presets": [
    {
      "id": "<preset_id>",
      "sender_id": "<public_key_hex>",
      "display_name": "<name>",
      "timestamp": 1234567890
    }
  ]
}
```

#### `p2p.publish` — Publish Content
Publish a signed envelope to the network.

**Request**: `p2p.publish <envelope_json>`
**Response**:
```json
{
  "ok": true,
  "id": "<assigned_id>"
}
```

#### `p2p.download` — Download Content
Download and verify content from the network.

**Request**: `p2p.download <id>`
**Response**:
```json
{
  "ok": true,
  "trust_level": 0,
  "local_path": "/path/to/downloaded/file",
  "name": "<display_name>",
  "format": "<format>",
  "sha256": "<hash>"
}
```

#### `p2p.verify` — Verify Content
Verify an incoming envelope without downloading.

**Request**: `p2p.verify <envelope_json>`
**Response**:
```json
{
  "ok": true,
  "trust_level": 0,
  "payload_json": "<verified payload>"
}
```

#### `p2p.peers` — List Discovered Peers
List LAN-discovered peers.

**Request**: `p2p.peers`
**Response**:
```json
{
  "ok": true,
  "peers": [
    {
      "instance_id": "<uuid>",
      "description": "<name>",
      "address": "192.168.1.100",
      "port": 8080,
      "last_seen": "<timestamp>"
    }
  ]
}
```

## Trust Levels

| Value | Name | Description |
|-------|------|-------------|
| 0 | Verified | Signature verified against known public key |
| 1 | Untrusted | Content hash matches but signature unverified |
| 2 | Tampered | Content hash mismatch — data corrupted or modified |

## Error Codes

| Code | Name | Description |
|------|------|-------------|
| 4001 | ToS Not Accepted | User hasn't accepted Terms of Service |
| 4002 | Missing Parameter | Required parameter not provided |
| 404 | Not Found | Requested content ID not found on network |
| 500 | Internal Error | Server-side processing error |
| 5001 | Signing Failed | Could not sign content for publishing |

## Envelope Format

All shared content uses a signed envelope:

```json
{
  "content_type": 0,
  "header": "{\"sender_id\":\"<pubkey_hex>\",\"timestamp\":1234567890,\"content_hash\":\"<sha256_hex>\",\"signature\":\"<ed25519_sig_hex>\",\"content_type\":0}",
  "payload": "<content-specific JSON>"
}
```

### Content Types
- `0` — Preset (project snapshot)
- `1` — Sample (audio file with metadata)
- `2` — Algorithm (custom generative pattern)

## Sequence Diagram: Agent-to-Agent Preset Sharing

```
Agent A (localhost:8080)          Agent B (localhost:8081)
        |                                  |
        |  1. mDNS discovery              |
        |<-------------------------------->|
        |  _cendance._tcp broadcast          |
        |                                  |
        |  2. TCP connect                  |
        |--------------------------------->|
        |                                  |
        |  3. p2p.search presets           |
        |--------------------------------->|
        |  {presets: [...]}                |
        |<---------------------------------|
        |                                  |
        |  4. p2p.download <id>            |
        |--------------------------------->|
        |  {ok, trust_level, local_path}   |
        |<---------------------------------|
        |                                  |
        |  5. Verify & apply               |
        |  (local processing)              |
```

## Security Considerations

1. **All content is signed** with Ed25519 before publishing
2. **Content hash verification** ensures integrity (SHA-256)
3. **Replay protection** rejects envelopes older than 5 minutes
4. **ToS gate** requires user acceptance before any P2P operation
5. **Trust on first use** — peers are not pre-trusted; signatures are verified per-envelope
6. **No transport encryption** on TCP line protocol (use SSH tunnel for WAN)
7. **Secret keys stored in plaintext** at `~/.cendance/identity.json` (future: OS keychain)
