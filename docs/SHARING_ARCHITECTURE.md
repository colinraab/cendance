# Experimental Sharing Architecture

Last reviewed: 2026-09-02

This document describes what cendance implements today. The files under
`docs/archive/` record earlier plans and implementation milestones; they are not
current specifications.

## Current Status

| Capability | Current behavior |
|---|---|
| Local package lifecycle | Preview, validate, install, list, apply, export, and remove data-only contribution packages through MCP. |
| Signed exchange | Ed25519-signed envelopes with SHA-256 content hashes for presets, samples, algorithms, arrangements, and projects. |
| Default backend | A file-backed store under the cendance user-data directory. No internet service is contacted. |
| Optional backend | An HTTP base URL supplied through `CENDANCE_P2P_ENDPOINT`. The repository does not include or operate that server. |
| Local agent protocol | One newline-delimited command per TCP connection on `127.0.0.1`, enabled with `--agent-port`. |
| LAN discovery | Experimental mDNS discovery for `_cendance._tcp`. The advertised agent server is still loopback-only, so discovery alone does not provide remote connectivity. |
| MCP | Local stdio transport enabled with `--mcp`; it is not a network listener. |
| Pilot Protocol | No current integration, dependency, bridge, or compatibility layer. |

The external [Pilot Protocol](https://pilotprotocol.network/) project still
exists, but cendance's historical documents used the same name for a different,
project-local JSON and TCP design. Do not treat those formats as Pilot Protocol
traffic.

## Data and Trust Boundaries

Contribution packages are data, not executable plug-ins. cendance validates
package schema, identifiers, references, numeric ranges, and paths before
installation. Built-in catalog IDs remain separate from namespaced contribution
references.

Shared content is wrapped in a signed envelope containing sender identity,
timestamp, content hash, signature, and content type. Hash and signature checks
detect modification and identify the signing key; they do not prove authorship,
license compatibility, safety, or quality.

Network and file operations run outside the real-time audio callback. Installed
content is applied through the same validated app and command pathways used by
local controls.

## Backends

### Local file store

Without `CENDANCE_P2P_ENDPOINT`, publish and search operations read and write a
local store. This is useful for development and single-machine exchange tests,
but it is not peer-to-peer networking.

### User-configured HTTP endpoint

When the environment variable is set, cendance expects these routes:

| Method | Route | Purpose |
|---|---|---|
| `POST` | `/api/v1/publish` | Publish a signed envelope. |
| `GET` | `/api/v1/presets` and `/api/v1/presets/{id}` | List or fetch presets. |
| `GET` | `/api/v1/samples` and `/api/v1/samples/{id}` | List or fetch samples. |
| `GET` | `/api/v1/algorithms` and `/api/v1/algorithms/{id}` | List or fetch algorithms. |
| `GET` | `/api/v1/projects` and `/api/v1/projects/{id}` | List or fetch projects. |

Empty or failed HTTP responses can fall back to the local file store. This
fallback is convenient for development but means a caller must inspect the
configured endpoint and returned status before claiming that content was shared
remotely.

## Current Release Posture

The application does not display or enforce a separate sharing notice or terms
of service. Local signing and file-backed exchange are available through the
sharing tools. Remote HTTP exchange occurs only when a user explicitly sets
`CENDANCE_P2P_ENDPOINT`.

## Future Pilot Integration

A real Pilot integration should be implemented as an explicit adapter rather
than by relabeling the current local protocol. The deferred design and release
gates are recorded in
[Future Pilot Protocol Integration Notes](archive/PILOT_PROTOCOL_INTEGRATION_NOTES.md).
