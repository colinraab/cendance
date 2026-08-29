---
title: UI-to-Audio-Flow
created: 2026-05-14
updated: 2026-05-14
type: relationship
tags: [cross-thread, command, real-time]
---

# UI to Audio Flow

## Path: Keystroke → Audio Output

```
User Key Press
    │
    ▼
TuiApp::handleEventInput()        [Main Thread]
    │
    ├─ KeyMapping lookup
    │
    ▼
TuiApp::dispatchAndLog()          [Main Thread]
    │
    ├─ Create Command struct
    ├─ Push to CommandQueue::push()
    └─ Add to undo stack
    │
    ▼
CommandQueue (SPSC, 256)          [Lock-free buffer]
    │
    ▼
AudioEngine::processCommands()    [Audio Thread]
    │
    ├─ CommandQueue::pop()
    │
    ▼
Command dispatch switch           [Audio Thread]
    │
    ├─ SetAlgorithm → AppState::setAlgorithmId()
    ├─ SetDensity → AppState::setDensity()
    ├─ PlayStop → AppState::setPlaying()
    ├─ SetTrackEffectPreset → applyTrackEffectPreset()
    ├─ SpotEffectOn → AppState::activeSpotEffects bitmask
    ├─ RebuildCustomAlgorithms → rebuildCustomAlgorithmInstances()
    └─ ... (40+ command types)
    │
    ▼
AudioEngine::audioDeviceIOCallback() [Audio Thread]
    │
    ├─ Transport::advance()
    ├─ GenerativeAlgorithm::processMidi() × 4 tracks
    ├─ SoundEngine rendering × 4 tracks
    ├─ processTrackInsertEffects() × 4 tracks
    ├─ Mix to master buffer
    ├─ processMasterEffects()
    ├─ processSpotEffects()
    └─ Output to audio device
```

## Key Constraints
- CommandQueue is SPSC: single producer (UI), single consumer (audio)
- If queue full, command is dropped (returns false)
- No allocations on audio path
- All state changes go through atomics or command processing

## Related Pages
- [[CommandQueue]] — the queue implementation
- [[Command]] — command struct and encoding
- [[AudioEngine]] — command processing and audio callback
- [[TuiApp]] — UI input handling
- [[AppState]] — atomic state targets
