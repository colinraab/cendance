# Wiki Log

> Chronological record of all wiki actions. Append-only.
> Format: `## [YYYY-MM-DD] action | subject`

## [2026-05-14] create | Wiki initialized
- Domain: cendance C++ terminal generative music app
- Full architecture scan completed
- Created SCHEMA.md, index.md, and all entity/concept/relationship pages
- Source: docs/CODEBASE_OVERVIEW.md + full header scan of all 130+ source files

## [2026-05-14] Refactor | Split AgentCommand.cpp into 7 domain files
- AgentCommand.cpp: 1,464 → ~100 lines (thin dispatcher)
- Created: AgentCommandUtils, AgentCommandState, AgentCommandTransport, AgentCommandTrack, AgentCommandMaster, AgentCommandMusical, AgentCommandPackages
- Updated: AgentCommand.md entity page + 7 new entity pages
- Updated: index.md with new entities
- Updated: CMakeLists.txt with new .cpp files

## [2026-05-14] Refactor | Split ProjectIO.cpp into save/load + operations
- ProjectIO.cpp: 1,419 → ~274 lines (snapshot, validate, apply, paths, recent projects)
- Created: ProjectIOLoad.h/.cpp — all JSON save/load/parsing logic (~680 lines)
- Fixed pre-existing `cendance_project_io_tests` failure by adding missing includes/dependencies
- Updated: ProjectIO.md + new ProjectIOLoad.md entity pages
- Updated: index.md with new entity
- Updated: Main.cpp, TuiAppProject.cpp, PresetSerializer.cpp, ProjectIOTests.cpp callers
- Updated: CMakeLists.txt with ProjectIOLoad.cpp in all targets
- All 8 tests pass (including previously failing project_io_tests)

## [2026-05-14] Refactor | Split TuiAppInput.cpp into modal handlers + router
- TuiAppInput.cpp: 1,177 → ~287 lines (thin router + handleMainInput)
- Created: TuiAppInput.h (tui_input namespace constants + helpers)
- Created: TuiAppInputHelpers.cpp (trimCopy, parseDisplayId, etc.)
- Created: TuiAppInputModals.cpp (handleToSInput, handleAgentInput, handleNumberSelectionInput, handleKeySelectionInput)
- Created: TuiAppInputAlgorithm.cpp (handleAlgorithmEditorInput)
- Created: TuiAppInputSamples.cpp (handleDrumSampleModalInput, handleSoundFileBrowserInput)
- Created: TuiAppInputProject.cpp (handleProjectPathModalInput)
- Created: TuiAppInputArrangement.cpp (handleArrangementModalInput)
- Updated: TuiApp.h with 10 new private method declarations
- Updated: index.md with TuiAppInput entity
- Updated: CMakeLists.txt with 7 new source files
- All 8 tests pass

## [2026-05-14] Refactor | Split Main.cpp into focused modules
- Main.cpp: 699 → ~120 lines (thin orchestrator)
- Created: CliOptions.h/.cpp (CLI arg parsing)
- Created: StartupRuntime.h/.cpp (project loading, library init, snapshot apply)
- Created: JuceRuntime.h/.cpp (JUCE MessageManager thread + AudioEngine lifetime)
- Created: StdoutRedirect.h/.cpp (MCP stdout redirect RAII)
- Created: P2PToolHandler.h/.cpp (P2P tool dispatch, extracted from lambda)
- Created: McpMode.h/.cpp (MCP server runtime + meter history collector)
- Updated: CMakeLists.txt with 6 new .cpp files
- All 8 tests pass
