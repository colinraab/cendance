---
title: Audio-to-UI-Flow
created: 2026-05-14
updated: 2026-05-14
type: relationship
tags: [cross-thread, metering, real-time]
---

# Audio to UI Flow

## Path: Audio Output → Display Update

```
AudioEngine::audioDeviceIOCallback() [Audio Thread]
    │
    ├─ Process audio block
    │
    ▼
MeterQueue::push(MeterData)       [Audio Thread]
    │
    ├─ trackLevels[4] (RMS)
    ├─ masterLevel (RMS)
    ├─ beatPosition / barNumber
    ├─ activeAlgorithm[4]
    ├─ isPlaying
    ├─ spectrumBins[32]
    ├─ activeNotes[4][2]
    └─ performanceProfile data
    │
    ▼
MeterQueue (SPSC, 64)             [Lock-free buffer]
    │
    ▼
TuiApp::run() loop                [Main Thread, ~30fps]
    │
    ├─ MeterQueue::popLatest()
    │   └─ Drains all, keeps most recent
    │   └─ OR-accumulates active notes
    │
    ▼
TuiApp::buildUI(MeterData)        [Main Thread]
    │
    ├─ TrackPanel (levels, algorithms)
    ├─ TransportBar (beat, bar, BPM)
    ├─ ParameterBar (density, complexity)
    ├─ SpectrumView (spectrum bins)
    └─ AgentInputBar (agent status)
    │
    ▼
FTXUI ScreenInteractive::Loop()   [Main Thread]
    │
    └─ Terminal rendering
```

## Key Constraints
- MeterQueue capacity is only 64 (smaller than CommandQueue's 256)
- `popLatest()` intentionally drops intermediate frames — UI only needs latest
- Active notes are OR-accumulated across drained frames to avoid missing short notes
- No allocations on audio path

## Related Pages
- [[MeterQueue]] — the queue implementation
- [[MeterData]] — metering data struct
- [[AudioEngine]] — metering data generation
- [[TuiApp]] — UI rendering
