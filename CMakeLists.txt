cmake_minimum_required(VERSION 3.24)

if(APPLE AND "${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING
        "Requested minimum macOS deployment version" FORCE)
endif()

project(cendance VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_path(SODIUM_INCLUDE_DIR sodium.h)
if(APPLE)
    # Prefer the static archive so developer packages do not depend on a
    # Homebrew-specific dylib path at runtime.
    set(_cendance_library_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a .dylib)
endif()
find_library(SODIUM_LIBRARY sodium)
if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_cendance_library_suffixes})
    unset(_cendance_library_suffixes)
endif()

if(NOT SODIUM_INCLUDE_DIR OR NOT SODIUM_LIBRARY)
    message(FATAL_ERROR "libsodium is required for P2P preset signing. Install it with your platform package manager, e.g. brew install libsodium.")
endif()

# --- macOS CLT workaround ---
# Some CommandLineTools installations have stale files in
# <CLT>/usr/include/c++/v1/ that shadow the real C++ stdlib headers
# shipped inside the SDK. Detect this and force the SDK's headers.
if(APPLE AND NOT DEFINED CENDANCE_SKIP_CLT_WORKAROUND)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE _sdk_path
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(_clt_cxx "/Library/Developer/CommandLineTools/usr/include/c++/v1")
    set(_sdk_cxx "${_sdk_path}/usr/include/c++/v1")

    if(IS_DIRECTORY "${_clt_cxx}"
       AND IS_DIRECTORY "${_sdk_cxx}"
       AND NOT EXISTS "${_clt_cxx}/algorithm")
        message(STATUS "Detected broken CLT C++ headers — applying SDK workaround")
        set(_cxx_fix "-nostdinc++ -isystem ${_sdk_cxx}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_cxx_fix}" CACHE STRING "" FORCE)
        # Also propagate via env so juceaide's nested cmake picks it up
        set(ENV{CXXFLAGS} "$ENV{CXXFLAGS} ${_cxx_fix}")
    endif()
endif()

# JUCE as submodule
add_subdirectory(JUCE EXCLUDE_FROM_ALL)

file(GLOB_RECURSE CENDANCE_DRUMKIT_OGG_FILES CONFIGURE_DEPENDS
    "Resources/DrumKits/*.ogg"
)
list(SORT CENDANCE_DRUMKIT_OGG_FILES)

file(GLOB_RECURSE CENDANCE_MELODIC_SAMPLE_OGG_FILES CONFIGURE_DEPENDS
    "Resources/MelodicSamples/*.ogg"
)
list(SORT CENDANCE_MELODIC_SAMPLE_OGG_FILES)

file(GLOB CENDANCE_IR_WAV_FILES CONFIGURE_DEPENDS
    "Resources/IRs/*.wav"
)
list(SORT CENDANCE_IR_WAV_FILES)

if(NOT CENDANCE_DRUMKIT_OGG_FILES)
    message(FATAL_ERROR "No drum kit OGG files found under Resources/DrumKits")
endif()

if(NOT CENDANCE_MELODIC_SAMPLE_OGG_FILES)
    message(FATAL_ERROR "No melodic sample OGG files found under Resources/MelodicSamples")
endif()

if(NOT CENDANCE_IR_WAV_FILES)
    message(FATAL_ERROR "No impulse-response WAV files found under Resources/IRs")
endif()

juce_add_binary_data(cendance_drumkit_data
    HEADER_NAME "DrumKitBinaryData.h"
    NAMESPACE "DrumKitBinaryData"
    SOURCES
    ${CENDANCE_DRUMKIT_OGG_FILES}
)

juce_add_binary_data(cendance_melodic_sample_data
    HEADER_NAME "MelodicSampleBinaryData.h"
    NAMESPACE "MelodicSampleBinaryData"
    SOURCES
    ${CENDANCE_MELODIC_SAMPLE_OGG_FILES}
)

juce_add_binary_data(cendance_ir_data
    HEADER_NAME "IrBinaryData.h"
    NAMESPACE "IrBinaryData"
    SOURCES
    ${CENDANCE_IR_WAV_FILES}
)

juce_add_binary_data(cendance_mcp_resources
    HEADER_NAME "McpBinaryData.h"
    NAMESPACE "McpBinaryData"
    SOURCES
    Resources/manual.md
)

# FTXUI via FetchContent
include(FetchContent)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "Do not install the private FTXUI dependency" FORCE)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG 5cfed50702f52d51c1b189b5f97f8beaf5eaa2a6 # v6.1.9
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(ftxui)

# Main executable
juce_add_console_app(cendance
    PRODUCT_NAME "cendance"
)

target_sources(cendance PRIVATE
    Source/Main.cpp
    Source/App/AgentCommand.cpp
    Source/App/AgentCommandUtils.cpp
    Source/App/AgentCommandState.cpp
    Source/App/AgentCommandTransport.cpp
    Source/App/AgentCommandTrack.cpp
    Source/App/AgentCommandMaster.cpp
    Source/App/AgentCommandMusical.cpp
    Source/App/AgentCommandPackages.cpp
    Source/App/AgentCommandGroove.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/DrumSampleLibrary.cpp
    Source/App/MelodicSampleLibrary.cpp
    Source/App/KeyMapping.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/StartupProjectInitializer.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/GenreCatalog.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/Audio/AudioEngine.cpp
    Source/Audio/EffectProcessor.cpp
    Source/Audio/CommandProcessor.cpp
    Source/Audio/Generators/CustomAlgorithmInstance.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Transport.cpp
    Source/Audio/Harmony/Scale.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Audio/Generators/FourOnFloor.cpp
    Source/Audio/Generators/Breakbeat.cpp
    Source/Audio/Generators/EuclideanRhythm.cpp
    Source/Audio/Generators/WalkingBass.cpp
    Source/Audio/Generators/SyncBass.cpp
    Source/Audio/Generators/BlockChords.cpp
    Source/Audio/Generators/SyncStabs.cpp
    Source/Audio/Generators/Arpeggiator.cpp
    Source/Audio/Generators/MarkovMelody.cpp
    Source/Audio/Generators/DrumStyleAlgorithms.cpp
    Source/Audio/Generators/BassStyleAlgorithms.cpp
    Source/Audio/Generators/ChordStyleAlgorithms.cpp
    Source/Audio/Generators/LeadStyleAlgorithms.cpp
    Source/Audio/Synths/DrumEngine.cpp
    Source/Audio/Synths/MelodicSampler.cpp
    Source/Audio/Synths/BassEngine.cpp
    Source/Audio/Synths/ChordEngine.cpp
    Source/Audio/Synths/LeadEngine.cpp
    Source/Audio/Effects/Spot/TapeStop.cpp
    Source/Audio/Effects/Spot/BeatRepeat.cpp
    Source/Audio/Effects/3_Filters/HighPassSweep.cpp
    Source/Audio/Effects/1_Space/ReverbWash.cpp
    Source/Audio/Effects/6_Degrade/ReduxCrush.cpp
    Source/Audio/Effects/1_Space/DelayEcho.cpp
    Source/Audio/Effects/2_Distortion/SaturationWaveshaper.cpp
    Source/Audio/Effects/2_Distortion/SoftHardClip.cpp
    Source/Audio/Effects/2_Distortion/Wavefolder.cpp
    Source/Audio/Effects/2_Distortion/AsymShaper.cpp
    Source/Audio/Effects/0_Dynamics/CompressorGlue.cpp
    Source/Audio/Effects/0_Dynamics/PeakLimiter.cpp
    Source/Audio/Effects/0_Dynamics/TransientShaper.cpp
    Source/Audio/Effects/3_Filters/CombFilter.cpp
    Source/Audio/Effects/3_Filters/MultiModeEQ.cpp
    Source/Audio/Effects/3_Filters/FormantFilter.cpp
    Source/Audio/Effects/7_Rhythm/Autopan.cpp
    Source/Audio/Effects/4_Modulation/RingModulator.cpp
    Source/Audio/Effects/4_Modulation/Chorus.cpp
    Source/Audio/Effects/4_Modulation/Phaser.cpp
    Source/Audio/Effects/4_Modulation/Flanger.cpp
    Source/Audio/Effects/6_Degrade/JitterDegrade.cpp
    Source/Audio/Effects/6_Degrade/ErosionDegrade.cpp
    Source/Audio/Effects/7_Rhythm/TranceGate.cpp
    Source/Audio/Effects/7_Rhythm/SidechainDucker.cpp
    Source/Audio/Effects/7_Rhythm/BeatRepeatInsert.cpp
    Source/Audio/Effects/5_Pitch/FrequencyShifter.cpp
    Source/Audio/Effects/5_Pitch/PitchShifter.cpp
    Source/Audio/Effects/5_Pitch/Harmonizer.cpp
    Source/Audio/Effects/8_Granular/TimeFreezer.cpp
    Source/Audio/Effects/8_Granular/GrainDelay.cpp
    Source/Audio/Effects/9_SpectralResonators/PhysicalModelingResonator.cpp
    Source/Audio/Effects/0_Dynamics/MultibandOtt.cpp
    Source/Audio/Effects/1_Space/ConvolutionReverb.cpp
    Source/Audio/Effects/1_Space/TapeDelay.cpp
    Source/Audio/Effects/1_Space/PingPongDelay.cpp
    Source/Audio/Effects/8_Granular/CloudGenerator.cpp
    Source/Audio/Effects/9_SpectralResonators/SpectralBlur.cpp
    Source/Audio/Effects/9_SpectralResonators/SpectralDelay.cpp
    Source/UI/AgentProtocolServer.h
    Source/UI/AgentProtocolServer.cpp
    Source/UI/TuiApp.cpp
    Source/UI/TuiAppProject.h
    Source/UI/TuiAppProject.cpp
    Source/UI/TuiAppRender.cpp
    Source/UI/TuiAppInput.cpp
    Source/UI/TuiAppInput.h
    Source/UI/TuiAppInputHelpers.cpp
    Source/UI/TuiAppInputModals.cpp
    Source/UI/TuiAppInputAlgorithm.cpp
    Source/UI/TuiAppInputSamples.cpp
    Source/UI/TuiAppInputProject.cpp
    Source/UI/TuiAppInputArrangement.cpp
    Source/UI/TuiAppInputGroove.cpp
    Source/App/CliOptions.cpp
    Source/App/StartupRuntime.cpp
    Source/App/JuceRuntime.cpp
    Source/App/StdoutRedirect.cpp
    Source/Mcp/P2PToolHandler.cpp
    Source/Mcp/McpMode.cpp
    Source/UI/TuiAppDrumCommands.cpp
    Source/UI/TuiAppNumberSelection.cpp
    Source/UI/TuiAppArrangement.cpp
    Source/Mcp/McpServer.cpp
    Source/Mcp/McpJsonHelpers.h
    Source/Mcp/McpJsonHelpers.cpp
    Source/Mcp/McpTools.cpp
    Source/Mcp/McpStaticData.cpp
    Source/Mcp/McpResources.cpp
    Source/Mcp/McpPrompts.cpp
    Source/Mcp/McpStdio.cpp
    Source/UI/Components/AgentInputBar.cpp
    Source/UI/Components/TrackPanel.cpp
    Source/UI/Components/TransportBar.cpp
    Source/UI/Components/ParameterBar.cpp
    Source/UI/Components/ArrangementModal.cpp
    Source/UI/Components/GrooveModal.cpp
    Source/UI/Components/NumberSelectionModal.cpp
    Source/UI/Components/KeyEntryModal.cpp
    Source/UI/Components/ProjectPathModal.cpp
    Source/UI/Components/SpectrumView.cpp
    Source/Security/ContentHeader.cpp
    Source/Security/SecurityManager.cpp
    Source/Security/PresetSerializer.cpp
    Source/Network/P2PClient.cpp
    Source/Network/P2PDownloadRegistry.cpp
    Source/Network/PeerDiscovery.cpp
    Source/Audio/AudioCaptureBus.cpp
    Source/Audio/FileRecorder.cpp
    Source/Audio/StreamSink.cpp
    Source/Audio/GrooveProcessor.cpp
)

target_compile_definitions(cendance PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_OGGVORBIS=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    CENDANCE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(cendance PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_data_structures
    juce::juce_dsp
    cendance_drumkit_data
    cendance_melodic_sample_data
    cendance_ir_data
    cendance_mcp_resources
    ftxui::component
    ${SODIUM_LIBRARY}
)

target_include_directories(cendance PRIVATE
    ${SODIUM_INCLUDE_DIR}
)

add_custom_command(TARGET cendance POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:cendance>
        "$<TARGET_FILE_DIR:cendance>/../cendance"
)

install(TARGETS cendance RUNTIME DESTINATION bin)
install(FILES
    LICENSE
    THIRD_PARTY_NOTICES.md
    DESTINATION share/doc/cendance
)
install(FILES Resources/LICENSE.md
    DESTINATION share/doc/cendance
    RENAME AUDIO_LICENSE.md
)
install(FILES Resources/IRs/README.md
    DESTINATION share/doc/cendance
    RENAME IMPULSE_RESPONSE_PROVENANCE.md
)

if(BUILD_TESTING)
# Phase 2 Verification tests
add_executable(cendance_tests Tests/QueueTests.cpp)
set_target_properties(cendance_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
target_link_libraries(cendance_tests PRIVATE Threads::Threads)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(cendance_tests PRIVATE -fsanitize=thread -g)
    target_link_options(cendance_tests PRIVATE -fsanitize=thread -g)
endif()
add_test(NAME cendance_queue_tests COMMAND cendance_tests)

# Phase 5 Verification tests
add_executable(cendance_harmony_tests 
    Tests/HarmonyTests.cpp 
    Source/Audio/Harmony/Scale.cpp 
    Source/Audio/Harmony/ChordProgression.cpp
)
set_target_properties(cendance_harmony_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_harmony_tests COMMAND cendance_harmony_tests)

add_executable(cendance_keymapping_tests
    Tests/KeyMappingTests.cpp
    Source/App/KeyMapping.cpp
)
target_link_libraries(cendance_keymapping_tests PRIVATE ftxui::component)
set_target_properties(cendance_keymapping_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_keymapping_tests COMMAND cendance_keymapping_tests)

add_executable(cendance_agent_command_tests
    Tests/AgentCommandTests.cpp
    Source/App/AgentCommand.cpp
    Source/App/AgentCommandUtils.cpp
    Source/App/AgentCommandState.cpp
    Source/App/AgentCommandTransport.cpp
    Source/App/AgentCommandTrack.cpp
    Source/App/AgentCommandMaster.cpp
    Source/App/AgentCommandMusical.cpp
    Source/App/AgentCommandPackages.cpp
    Source/App/AgentCommandGroove.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Mcp/McpServer.cpp
    Source/Mcp/McpTools.cpp
    Source/Mcp/McpJsonHelpers.cpp
    Source/Mcp/McpPrompts.cpp
    Source/Mcp/McpResources.cpp
    Source/Mcp/McpStaticData.cpp
)
target_link_libraries(cendance_agent_command_tests PRIVATE juce::juce_core cendance_mcp_resources)
set_target_properties(cendance_agent_command_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_agent_command_tests COMMAND cendance_agent_command_tests)

add_executable(cendance_agent_input_bar_tests
    Tests/AgentInputBarTests.cpp
    Source/UI/Components/AgentInputBar.cpp
)
target_link_libraries(cendance_agent_input_bar_tests PRIVATE
    ftxui::dom
)
set_target_properties(cendance_agent_input_bar_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_agent_input_bar_tests COMMAND cendance_agent_input_bar_tests)

add_executable(cendance_core_tests
    Tests/CoreLogicTests.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/StartupProjectInitializer.cpp
    Source/Audio/Transport.cpp
    Source/Audio/Harmony/Scale.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Audio/Effects/6_Degrade/ReduxCrush.cpp
    Source/Audio/Effects/Spot/BeatRepeat.cpp
    Source/Audio/Generators/SyncBass.cpp
    Source/Audio/Generators/SyncStabs.cpp
    Source/Audio/Generators/BlockChords.cpp
    Source/Audio/Generators/CustomAlgorithmInstance.cpp
    Source/Audio/Generators/DrumStyleAlgorithms.cpp
    Source/Audio/Generators/BassStyleAlgorithms.cpp
    Source/Audio/Generators/ChordStyleAlgorithms.cpp
    Source/Audio/Generators/LeadStyleAlgorithms.cpp
    Source/App/CustomAlgorithmPreset.cpp
)
target_link_libraries(cendance_core_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
)
set_target_properties(cendance_core_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_core_tests COMMAND cendance_core_tests)

add_executable(cendance_project_io_tests
    Tests/ProjectIOTests.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
)
target_link_libraries(cendance_project_io_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_project_io_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_project_io_tests COMMAND cendance_project_io_tests)

add_executable(cendance_audio_integration_tests
    Tests/AudioEngineIntegrationTests.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/DrumSampleLibrary.cpp
    Source/App/MelodicSampleLibrary.cpp
    Source/App/StartupProjectInitializer.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/GenreCatalog.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/Audio/AudioEngine.cpp
    Source/Audio/EffectProcessor.cpp
    Source/Audio/CommandProcessor.cpp
    Source/Audio/Generators/CustomAlgorithmInstance.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Transport.cpp
    Source/Audio/Harmony/Scale.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Audio/Generators/FourOnFloor.cpp
    Source/Audio/Generators/Breakbeat.cpp
    Source/Audio/Generators/EuclideanRhythm.cpp
    Source/Audio/Generators/WalkingBass.cpp
    Source/Audio/Generators/SyncBass.cpp
    Source/Audio/Generators/BlockChords.cpp
    Source/Audio/Generators/SyncStabs.cpp
    Source/Audio/Generators/Arpeggiator.cpp
    Source/Audio/Generators/MarkovMelody.cpp
    Source/Audio/Generators/DrumStyleAlgorithms.cpp
    Source/Audio/Generators/BassStyleAlgorithms.cpp
    Source/Audio/Generators/ChordStyleAlgorithms.cpp
    Source/Audio/Generators/LeadStyleAlgorithms.cpp
    Source/Audio/Synths/DrumEngine.cpp
    Source/Audio/Synths/MelodicSampler.cpp
    Source/Audio/Synths/BassEngine.cpp
    Source/Audio/Synths/ChordEngine.cpp
    Source/Audio/Synths/LeadEngine.cpp
    Source/Audio/Effects/Spot/TapeStop.cpp
    Source/Audio/Effects/Spot/BeatRepeat.cpp
    Source/Audio/Effects/3_Filters/HighPassSweep.cpp
    Source/Audio/Effects/1_Space/ReverbWash.cpp
    Source/Audio/Effects/6_Degrade/ReduxCrush.cpp
    Source/Audio/Effects/1_Space/DelayEcho.cpp
    Source/Audio/Effects/2_Distortion/SaturationWaveshaper.cpp
    Source/Audio/Effects/2_Distortion/SoftHardClip.cpp
    Source/Audio/Effects/2_Distortion/Wavefolder.cpp
    Source/Audio/Effects/2_Distortion/AsymShaper.cpp
    Source/Audio/Effects/0_Dynamics/CompressorGlue.cpp
    Source/Audio/Effects/0_Dynamics/PeakLimiter.cpp
    Source/Audio/Effects/0_Dynamics/TransientShaper.cpp
    Source/Audio/Effects/3_Filters/CombFilter.cpp
    Source/Audio/Effects/3_Filters/MultiModeEQ.cpp
    Source/Audio/Effects/3_Filters/FormantFilter.cpp
    Source/Audio/Effects/7_Rhythm/Autopan.cpp
    Source/Audio/Effects/4_Modulation/RingModulator.cpp
    Source/Audio/Effects/4_Modulation/Chorus.cpp
    Source/Audio/Effects/4_Modulation/Phaser.cpp
    Source/Audio/Effects/4_Modulation/Flanger.cpp
    Source/Audio/Effects/6_Degrade/JitterDegrade.cpp
    Source/Audio/Effects/6_Degrade/ErosionDegrade.cpp
    Source/Audio/Effects/7_Rhythm/TranceGate.cpp
    Source/Audio/Effects/7_Rhythm/SidechainDucker.cpp
    Source/Audio/Effects/7_Rhythm/BeatRepeatInsert.cpp
    Source/Audio/Effects/5_Pitch/FrequencyShifter.cpp
    Source/Audio/Effects/5_Pitch/PitchShifter.cpp
    Source/Audio/Effects/5_Pitch/Harmonizer.cpp
    Source/Audio/Effects/8_Granular/TimeFreezer.cpp
    Source/Audio/Effects/8_Granular/GrainDelay.cpp
    Source/Audio/Effects/9_SpectralResonators/PhysicalModelingResonator.cpp
    Source/Audio/Effects/0_Dynamics/MultibandOtt.cpp
    Source/Audio/Effects/1_Space/ConvolutionReverb.cpp
    Source/Audio/Effects/1_Space/TapeDelay.cpp
    Source/Audio/Effects/1_Space/PingPongDelay.cpp
    Source/Audio/Effects/8_Granular/CloudGenerator.cpp
    Source/Audio/Effects/9_SpectralResonators/SpectralBlur.cpp
    Source/Audio/Effects/9_SpectralResonators/SpectralDelay.cpp
    Source/Audio/AudioCaptureBus.cpp
    Source/Audio/FileRecorder.cpp
    Source/Audio/StreamSink.cpp
    Source/Audio/GrooveProcessor.cpp
)
set_target_properties(cendance_audio_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
target_link_libraries(cendance_audio_integration_tests PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_data_structures
    juce::juce_dsp
    cendance_drumkit_data
    cendance_melodic_sample_data
    cendance_ir_data
)

target_compile_definitions(cendance_audio_integration_tests PRIVATE
    JUCE_USE_OGGVORBIS=1
)
add_test(NAME cendance_audio_integration_tests COMMAND cendance_audio_integration_tests)
set_tests_properties(cendance_audio_integration_tests PROPERTIES TIMEOUT 120)

add_executable(cendance_convolution_reverb_tests
    Tests/ConvolutionReverbTests.cpp
    Source/Audio/Effects/1_Space/ConvolutionReverb.cpp
)
set_target_properties(cendance_convolution_reverb_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
target_link_libraries(cendance_convolution_reverb_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_dsp
    cendance_ir_data
)
add_test(NAME cendance_convolution_reverb_tests COMMAND cendance_convolution_reverb_tests)

# ========================================================================
# P0: New test executables
# ========================================================================

add_executable(cendance_groove_processor_tests
    Tests/GrooveProcessorTests.cpp
    Source/Audio/GrooveProcessor.cpp
)
target_link_libraries(cendance_groove_processor_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
)
set_target_properties(cendance_groove_processor_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_groove_processor_tests COMMAND cendance_groove_processor_tests)

add_executable(cendance_content_header_tests
    Tests/ContentHeaderTests.cpp
    Source/Security/ContentHeader.cpp
)
target_link_libraries(cendance_content_header_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_content_header_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_content_header_tests COMMAND cendance_content_header_tests)

# ========================================================================
# P1: New test executables
# ========================================================================

add_executable(cendance_arrangement_preset_tests
    Tests/ArrangementPresetManagerTests.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Config/ToSGuard.cpp
)
target_link_libraries(cendance_arrangement_preset_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_arrangement_preset_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_arrangement_preset_tests COMMAND cendance_arrangement_preset_tests)

add_executable(cendance_audio_capture_bus_tests
    Tests/AudioCaptureBusTests.cpp
    Source/Audio/AudioCaptureBus.cpp
)
target_link_libraries(cendance_audio_capture_bus_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_audio_capture_bus_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_audio_capture_bus_tests COMMAND cendance_audio_capture_bus_tests)

# ========================================================================
# P2: New test executables
# ========================================================================

add_executable(cendance_custom_algorithm_preset_tests
    Tests/CustomAlgorithmPresetTests.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Config/ToSGuard.cpp
)
target_link_libraries(cendance_custom_algorithm_preset_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_custom_algorithm_preset_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_custom_algorithm_preset_tests COMMAND cendance_custom_algorithm_preset_tests)

add_executable(cendance_stream_sink_tests
    Tests/StreamSinkTests.cpp
    Source/Audio/StreamSink.cpp
)
target_link_libraries(cendance_stream_sink_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_stream_sink_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_stream_sink_tests COMMAND cendance_stream_sink_tests)

add_executable(cendance_file_recorder_tests
    Tests/FileRecorderTests.cpp
    Source/Audio/FileRecorder.cpp
    Source/Audio/AudioCaptureBus.cpp
)
target_link_libraries(cendance_file_recorder_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
)
set_target_properties(cendance_file_recorder_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_file_recorder_tests COMMAND cendance_file_recorder_tests)

add_executable(cendance_cli_options_tests
    Tests/CliOptionsTests.cpp
    Source/App/CliOptions.cpp
)
set_target_properties(cendance_cli_options_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME cendance_cli_options_tests COMMAND cendance_cli_options_tests)

# ========================================================================
# Integration Test Executables
# ========================================================================

# P0: P2P Round-Trip Integration Tests
add_executable(cendance_p2p_integration_tests
    Tests/P2PIntegrationTests.cpp
    Source/Security/PresetSerializer.cpp
    Source/Security/SecurityManager.cpp
    Source/Security/ContentHeader.cpp
    Source/Network/P2PClient.cpp
    Source/Network/P2PDownloadRegistry.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Harmony/ChordProgression.cpp
)
target_link_libraries(cendance_p2p_integration_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    ${SODIUM_LIBRARY}
)
target_include_directories(cendance_p2p_integration_tests PRIVATE ${SODIUM_INCLUDE_DIR})
set_target_properties(cendance_p2p_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_p2p_integration_tests COMMAND cendance_p2p_integration_tests)
set_tests_properties(cendance_p2p_integration_tests PROPERTIES TIMEOUT 60)

# P0: Security Integration Tests
add_executable(cendance_security_integration_tests
    Tests/SecurityIntegrationTests.cpp
    Source/Security/SecurityManager.cpp
    Source/Security/PresetSerializer.cpp
    Source/Security/ContentHeader.cpp
    Source/Network/P2PClient.cpp
    Source/Network/P2PDownloadRegistry.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Harmony/ChordProgression.cpp
)
target_link_libraries(cendance_security_integration_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    ${SODIUM_LIBRARY}
)
target_include_directories(cendance_security_integration_tests PRIVATE ${SODIUM_INCLUDE_DIR})
set_target_properties(cendance_security_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_security_integration_tests COMMAND cendance_security_integration_tests)
set_tests_properties(cendance_security_integration_tests PROPERTIES TIMEOUT 60)

# P1: MCP Tool Handler Integration Tests
add_executable(cendance_mcp_tool_handler_integration_tests
    Tests/McpToolHandlerIntegrationTests.cpp
    Source/Mcp/P2PToolHandler.cpp
    Source/Security/PresetSerializer.cpp
    Source/Security/SecurityManager.cpp
    Source/Security/ContentHeader.cpp
    Source/Network/P2PClient.cpp
    Source/Network/P2PDownloadRegistry.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Harmony/ChordProgression.cpp
)
target_link_libraries(cendance_mcp_tool_handler_integration_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    ${SODIUM_LIBRARY}
)
target_include_directories(cendance_mcp_tool_handler_integration_tests PRIVATE ${SODIUM_INCLUDE_DIR})
set_target_properties(cendance_mcp_tool_handler_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_mcp_tool_handler_integration_tests COMMAND cendance_mcp_tool_handler_integration_tests)
set_tests_properties(cendance_mcp_tool_handler_integration_tests PROPERTIES TIMEOUT 60)

# P1: Download Registry Integration Tests
add_executable(cendance_download_registry_integration_tests
    Tests/DownloadRegistryIntegrationTests.cpp
    Source/Network/P2PDownloadRegistry.cpp
)
target_link_libraries(cendance_download_registry_integration_tests PRIVATE
    juce::juce_core
)
set_target_properties(cendance_download_registry_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_download_registry_integration_tests COMMAND cendance_download_registry_integration_tests)
set_tests_properties(cendance_download_registry_integration_tests PROPERTIES TIMEOUT 30)

# P2: Agent Command -> P2P Bridge Integration Tests
add_executable(cendance_agent_command_p2p_integration_tests
    Tests/AgentCommandP2PIntegrationTests.cpp
    Source/App/AgentCommand.cpp
    Source/App/AgentCommandUtils.cpp
    Source/App/AgentCommandState.cpp
    Source/App/AgentCommandTransport.cpp
    Source/App/AgentCommandTrack.cpp
    Source/App/AgentCommandMaster.cpp
    Source/App/AgentCommandMusical.cpp
    Source/App/AgentCommandPackages.cpp
    Source/App/AgentCommandGroove.cpp
    Source/App/ArrangementPresetManager.cpp
    Source/App/ContributionPackage.cpp
    Source/App/ContributionPackageParse.cpp
    Source/App/PresetRef.cpp
    Source/App/PresetRegistry.cpp
    Source/App/ProjectKey.cpp
    Source/App/ProjectIO.cpp
    Source/App/ProjectIOLoad.cpp
    Source/App/AlgorithmPresetRegistry.cpp
    Source/App/CustomAlgorithmPreset.cpp
    Source/App/GenreCatalog.cpp
    Source/Config/ToSGuard.cpp
    Source/Audio/Harmony/ChordProgression.cpp
    Source/Security/PresetSerializer.cpp
    Source/Security/SecurityManager.cpp
    Source/Security/ContentHeader.cpp
    Source/Network/P2PClient.cpp
    Source/Network/P2PDownloadRegistry.cpp
)
target_link_libraries(cendance_agent_command_p2p_integration_tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    ${SODIUM_LIBRARY}
)
target_include_directories(cendance_agent_command_p2p_integration_tests PRIVATE ${SODIUM_INCLUDE_DIR})
set_target_properties(cendance_agent_command_p2p_integration_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
add_test(NAME cendance_agent_command_p2p_integration_tests COMMAND cendance_agent_command_p2p_integration_tests)
set_tests_properties(cendance_agent_command_p2p_integration_tests PROPERTIES TIMEOUT 60)

# The test suite currently uses standard assert() checks. Keep them enabled in
# optimized configurations so a Release test run exercises the same checks as
# Debug rather than compiling them out via NDEBUG.
get_property(_cendance_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(_target IN LISTS _cendance_targets)
    if(_target MATCHES "^cendance(_.*)?_tests$")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${_target} PRIVATE -UNDEBUG)
        elseif(MSVC)
            target_compile_options(${_target} PRIVATE /UNDEBUG)
        endif()
    endif()
endforeach()
endif()

set(CPACK_PACKAGE_NAME "cendance")
set(CPACK_PACKAGE_VENDOR "cendance contributors")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Terminal-first generative music workstation")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME
    "cendance-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TGZ")
include(CPack)
