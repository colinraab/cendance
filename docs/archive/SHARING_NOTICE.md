# Archived cendance Experimental Sharing Notice

> Archived on 2026-09-02. This notice is not displayed, acknowledged, or
> enforced by the current application. It is retained only as historical design
> material in case a hosted or federated sharing service is added later.

Last updated: 2026-09-02

This notice applies only when you use cendance features that sign, publish,
download, install, or exchange presets, samples, algorithms, arrangements, or
projects. It is not a separate license for cendance and does not limit the
rights granted by the project's AGPL-3.0-only license.

This is a practical project notice, not legal advice or a substitute for terms
published by an independent network or service operator.

## Current Network Status

cendance does not operate a hosted sharing service and does not currently
integrate with Pilot Protocol. By default, sharing operations use a local
file-backed store on the same computer. A user can explicitly configure an HTTP
service with `CENDANCE_P2P_ENDPOINT`; that service is operated and trusted by
whoever configured it, not by the cendance project.

The localhost agent protocol and LAN discovery support are experimental. LAN
discovery does not currently make the loopback-only agent protocol reachable
from another computer.

## Content You Share

Share only content that you created, own, or have permission to redistribute.
You are responsible for preserving required attribution, license, and provenance
information. Do not share unlawful, infringing, malicious, deceptive, or abusive
content.

Audio embedded in a package is copied to the receiving computer and may remain
there after the package is removed. Do not assume that deleting a local copy
recalls copies already shared.

## Content You Receive

Network identity, signatures, and hashes can help identify a sender and detect
modified data. They do not establish that content is safe, lawful, accurately
licensed, useful, or compatible. Review package metadata and preview packages
before installing them. Treat files and endpoints from unknown sources as
untrusted.

## Privacy and Transport

Sharing may expose the public identity and display name stored in cendance
metadata, as well as the content and metadata you choose to publish. A configured
HTTP endpoint receives network requests and can observe normal connection
metadata. cendance does not promise transport encryption for a user-configured
HTTP endpoint; use HTTPS and a service you trust.

Pilot Protocol, if integrated in the future, would remain a separate project
with its own software, network behavior, privacy policy, and terms.

## Acknowledgement

cendance records acknowledgement in its local configuration using the legacy
`tos_accepted` and `tos_accepted_at` field names. Those names are retained for
compatibility; acknowledgement applies to this sharing notice, not to ordinary
use, modification, or redistribution of the AGPL-licensed software.

The project is provided without warranty under the terms of the
[AGPL-3.0-only license](../../LICENSE). Use safe listening levels and protect your
hearing and equipment.
