# Code Wiki Schema

## Domain
cendance — C++ terminal generative music application (JUCE + FTXUI). CMake build, C++20, arm64 macOS.

## Conventions
- File names: lowercase, hyphens, match class names where possible (e.g., `audio-engine.md`)
- Every wiki page starts with YAML frontmatter
- Use `[[wikilinks]]` to link between pages (minimum 2 outbound links per page)
- When updating a page, always bump the `updated` date
- Every new page must be added to `index.md` under the correct section
- Every action must be appended to `log.md`
- Source file references use format: `Source/Path/File.h`

## Frontmatter
```yaml
---
title: ClassName
created: YYYY-MM-DD
updated: YYYY-MM-DD
type: entity | concept | relationship | query
tags: [from taxonomy below]
source_files:
  - Source/Path/Class.h
  - Source/Path/Class.cpp
---
```

## Tag Taxonomy
- **Core:** audio, ui, network, config, security, mcp, app-state
- **Patterns:** lock-free, spsc-queue, command-pattern, real-time, cross-thread
- **Domain:** generator, synth, effect, transport, preset, p2p, algorithm, spot-fx, insert-fx
- **Layer:** real-time, main-thread, message-thread
- **IO:** midi, audio-device, stdio

## Page Thresholds
- **Create an entity page** for every class/struct with >50 lines or that owns significant state
- **Create a concept page** for every architectural pattern, subsystem, or cross-cutting concern
- **Create a relationship page** for every major data flow or call chain
- **DON'T create a page** for trivial structs, getters/setters, or implementation details
- **Split a page** when it exceeds ~150 lines

## Update Policy (BEFORE EVERY COMMIT)
When code changes affect the wiki:
1. Identify which classes/concepts changed
2. Update affected entity pages (members, relationships, behavior)
3. Update affected concept pages (patterns, rules)
4. Update relationship pages if data flow changed
5. Update index.md if pages were added/removed
6. Append to log.md with the commit context
7. Bump `updated` date on all changed pages
