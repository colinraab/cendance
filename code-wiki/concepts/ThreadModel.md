---
title: ThreadModel
created: 2026-05-14
updated: 2026-05-14
type: concept
tags: [cross-thread, real-time, architecture]
---

# Thread Model

## What It Is
cendance uses exactly 3 threads. Cross-thread communication is strictly via lock-free SPSC queues and atomic variables. No mutexes between threads.

## The Three Threads

### 1. Main Thread (UI)
- Runs FTXUI `ScreenInteractive::Loop()`
- Keyboard capture, terminal rendering
- Dispatches commands via [[CommandQueue]]
- Reads metering via [[MeterQueue]]
- **Can do:** allocations, file I/O, mutexes, std::cout
- **Must NOT do:** touch audio thread state directly

### 2. JUCE Message Thread (Background)
- Runs `juce::MessageManager`
- Timer callbacks, async updates, non-RT housekeeping
- MCP server can run here
- **Can do:** allocations, file I/O, JUCE message dispatch
- **Must NOT do:** touch audio thread state directly

### 3. Audio Thread (Real-Time)
- Triggered by `juce::AudioDeviceManager` callback
- Real-time DSP, sequencer clock, generative algorithms, effects
- **MUST NOT do:** allocations (new/delete), vectors resizing, standard mutexes, file I/O, std::cout
- **Can do:** atomic operations, lock-free queue operations, DSP math

## Communication Flows

```
UI Thread ──push()──▶ CommandQueue ──pop()──▶ Audio Thread
Audio Thread ──push()──▶ MeterQueue ──popLatest()──▶ UI Thread
```

- **UI → Audio:** [[CommandQueue]] (SPSC, 256 capacity)
- **Audio → UI:** [[MeterQueue]] (SPSC, 64 capacity)
- **Shared State:** [[AppState]] (all atomic members)

## Rules
1. Never block the audio thread
2. Never touch audio thread state from UI thread directly
3. All cross-thread data flows through queues or atomics
4. Cache registry counts in [[AudioEngine]] to avoid mutex on audio path

## Gotchas
- `AppState` setters use `memory_order_relaxed` — don't rely on ordering between different atomics
- If CommandQueue is full, commands are silently dropped
- MeterQueue `popLatest()` drains all and keeps most recent — intentional for UI responsiveness
