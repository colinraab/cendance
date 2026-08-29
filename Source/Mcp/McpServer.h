#pragma once

#include <juce_core/juce_core.h>
#include <functional>
#include <string>
#include <atomic>

/* ========================================================
 * MCP Server — C++ native implementation
 *
 * Embeds the full cendance-mcp-server toolset inside the
 * cendance binary.  The MCP stdio transport reads JSON-RPC
 * 2.0 requests from stdin, dispatches them to the same
 * AgentCommand::execute() pipeline the TUI uses, and
 * writes MCP-formatted responses to stdout.
 *
 * File layout for this translation unit:
 *   McpServer.cpp      — constructor, dispatch, initialize, tools/list
 *   McpTools.cpp       — 32 tool schemas + tools/call dispatcher
 *   McpResources.cpp   — resources/list, resources/read
 *   McpPrompts.cpp     — prompts/list, prompts/get
 *   McpStaticData.cpp  — authoring guide + package schema
 *   McpStdio.cpp       — writeStdout, readStdinLine, run, stop
 *   McpJsonHelpers.*   — arg extraction + JSON-RPC result builders
 *
 * Tools ported from cendance-mcp-server (minus lifecycle):
 *   get_cendance_state             set_track_parameter
 *   get_cendance_catalog           set_track_algorithm
 *   get_cendance_preset_catalog    set_track_sound
 *   get_cendance_meters            set_track_mute
 *   listen_to_cendance             set_track_fx
 *   play_cendance                  set_master_fx
 *   pause_cendance                 set_key
 *   stop_playback                set_progression
 *   set_tempo                    set_genre
 *   randomize_for_genre          set_arrangement_section
 *   list_cendance_packages         apply_cendance_contribution
 *   get_cendance_contribution_cat  apply_cendance_preset_ref
 *   preview_cendance_package       set_track_sound_ref
 *   install_cendance_package       set_track_fx_ref
 *   remove_cendance_package        set_master_fx_ref
 *   export_cendance_package
 *
 * Meta tools:
 *   send_cendance_command          (raw protocol)
 *   get_cendance_agent_authoring_guide
 *   get_cendance_package_schema
 * ======================================================== */

class McpServer
{
public:
    /** ExecuteFn maps an agent protocol command string (e.g. "state full")
     *  to the JSON response string already produced by AgentCommand. */
    using ExecuteFn = std::function<juce::String (const juce::String&)>;

    /** P2PFn handles P2P-specific MCP tools that need access to
     *  SecurityManager, P2PClient, etc. The tool name and arguments
     *  are passed as a JSON string; the response is a JSON string. */
    using P2PFn = std::function<juce::String (const juce::String& toolName,
                                                const juce::String& argsJson)>;

    explicit McpServer (ExecuteFn execute, P2PFn p2pExecute = nullptr);
    ~McpServer() = default;

    McpServer (const McpServer&) = delete;
    McpServer& operator= (const McpServer&) = delete;

    // ── Public API ──
    void run();
    void stop();

    // ── Tool schemas (implemented in McpTools.cpp) ──
    static juce::var toolSchemas();
    juce::var handleToolsList (juce::var const& params, const juce::var& id);
    juce::var handleToolsCall (juce::var const& params, const juce::var& id);

    // ── Static data (implemented in McpStaticData.cpp) ──
    static juce::var getAgentAuthoringGuide();
    static juce::var getPackageSchema (const juce::String& kind = "");

    // ── Resources (implemented in McpResources.cpp) ──
    juce::var handleResourcesList (juce::var const& params, const juce::var& id);
    juce::var handleResourcesRead (juce::var const& params, const juce::var& id);

    // ── Prompts (implemented in McpPrompts.cpp) ──
    juce::var handlePromptsList (juce::var const& params, const juce::var& id);
    juce::var handlePromptsGet (juce::var const& params, const juce::var& id);

    // ── JSON helpers (implemented in McpJsonHelpers.cpp) ──
    static juce::var makeResult (const juce::var& result, const juce::var& id);
    static juce::var makeTextContent (const juce::String& text);
    static juce::var makeToolError (const juce::String& msg, const juce::var& id);
    static juce::var makeJsonError (int code, const juce::String& msg, const juce::var& id);
    static double  getDouble  (juce::var const& v, double fallback);
    static int     getInt     (juce::var const& v, int fallback);
    static juce::String getString  (juce::var const& v, const char* fallback);
    static bool    getBool    (juce::var const& v, bool fallback);
    static juce::String quoteArg (const juce::String& text);

    // ── stdio I/O (implemented in McpStdio.cpp) ──
    void writeStdout (const juce::var& response);
    juce::String readStdinLine();
    std::string readExactBytes (size_t n);

private:
    // ── MCP resources ──
    enum ResourceId { Resource_Manual };
    int getResourceCount() const { return 1; }

    // ── MCP prompts ──
    enum PromptId { Prompt_StartProject, Prompt_AnalyzeImprove, Prompt_DesignPackage, Prompt_GenreRandomize };
    int getPromptCount() const { return 4; }

    // ── Internal dispatch ──
    juce::var dispatchRequest (juce::var const& req);
    juce::var handleInitialize (juce::var const& params, const juce::var& id);

    ExecuteFn       executeFn_;
    P2PFn           p2pFn_;
    std::atomic<bool> running_  { false };
    std::atomic<bool> stopping_ { false };
};
