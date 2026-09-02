# Future Pilot Protocol Integration Notes

Status: deferred as of 2026-09-02. cendance has no Pilot dependency, adapter,
wire compatibility, or live Pilot connection.

This note records a practical path for adding Pilot support later without
mistaking cendance's existing local sharing code for a Pilot implementation.
Recheck Pilot's current documentation and supported client interfaces before
starting; the upstream API may change.

## Recommended Boundary

Implement Pilot as an optional transport adapter behind the existing sharing
operations. Keep cendance's signed envelopes, hashes, package validation,
license metadata, preview flow, and staging rules above that transport boundary.
Pilot transport trust must not bypass cendance content validation.

For a first implementation, connect to a locally managed Pilot client or daemon
through a pinned, documented API boundary. Keep Pilot networking and blocking
I/O off the real-time audio callback. Do not silently fall back to the local
file store after a requested remote operation fails.

## Smallest Useful Spike

1. Transfer one signed preset between two explicitly configured peers.
2. Give the cendance payload protocol its own version and service identifier.
3. Stage received content outside the installed package directories.
4. Verify the signature, hash, timestamp, schema, paths, and license metadata
   before preview or installation.
5. Report connection, transfer, validation, and rejection failures distinctly.
6. Test two-device discovery, successful transfer, tampering, incompatible
   versions, interruption, retry, and duplicate delivery.

Automatic discovery, public catalogs, moderation, accounts, and hosted
infrastructure should remain separate later phases. If the cendance project
ever operates a service, write service-specific privacy and usage terms for
that actual service rather than restoring the archived generic template.

## Existing Code That Can Be Reused

- `P2PToolHandler` provides the application-level sharing operations.
- `P2PClient` provides the current file and HTTP backends and is the likely
  adapter seam.
- `PresetSerializer` and `SecurityManager` provide signed content envelopes.
- Contribution package validation and preview provide the safe installation
  boundary.

`PeerDiscovery` advertises and browses the cendance-specific `_cendance._tcp`
mDNS service. It is not Pilot discovery and should not be presented as such.

## Release Gate

Do not advertise Pilot support until a pinned integration passes repeatable
two-machine tests and the user-facing documentation identifies exactly what
connects, what data leaves the machine, who operates any service, and how the
feature is disabled.

Upstream starting point: [Pilot Protocol](https://pilotprotocol.network/).
