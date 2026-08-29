#include "McpServer.h"

#include "../App/AgentCommand.h"
#include "../App/AppState.h"
#include "../App/ContributionPackage.h"

#include <juce_core/juce_core.h>

//==============================================================================
// Tool schemas — exact match of cendance-mcp-server/src/index.ts Zod schemas
//==============================================================================

juce::var McpServer::toolSchemas()
{
    return juce::JSON::parse (R"json([
    {"name":"send_cendance_command","description":"Send a raw cendance agent protocol command and return its JSON response.","inputSchema":{"type":"object","required":["command"],"properties":{"command":{"type":"string"}}}},
    {"name":"get_cendance_state","description":"Get the current cendance state snapshot.","inputSchema":{"type":"object","properties":{"full":{"type":"boolean","default":false}}}},
    {"name":"get_cendance_catalog","description":"Get a cendance catalog: algorithms, sounds, effects, progressions, or presets.","inputSchema":{"type":"object","required":["kind"],"properties":{"kind":{"type":"string","enum":["algorithms","sounds","effects","progressions","presets"]}}}},
    {"name":"get_cendance_meters","description":"Get the latest cendance meter and spectrum snapshot.","inputSchema":{"type":"object","properties":{}}},
    {"name":"listen_to_cendance","description":"Run cendance internal listening heuristic over recent meter history.","inputSchema":{"type":"object","properties":{"seconds":{"type":"number","default":8,"positive":true,"max":60}}}},
    {"name":"set_track_parameter","description":"Set a cendance track parameter: density, complexity, tone, motion, or gain.","inputSchema":{"type":"object","required":["track","parameter","value"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"parameter":{"type":"string","enum":["density","complexity","tone","motion","gain"]},"value":{"type":"number","minimum":0,"maximum":2}}}},
    {"name":"set_track_algorithm","description":"Set a track algorithm by one-based catalog id.","inputSchema":{"type":"object","required":["track","id"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"id":{"type":"integer","minimum":1}}}},
    {"name":"set_track_sound","description":"Set a track sound by one-based catalog id.","inputSchema":{"type":"object","required":["track","id"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"id":{"type":"integer","minimum":1}}}},
    {"name":"set_track_mute","description":"Mute or unmute a track.","inputSchema":{"type":"object","required":["track","muted"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"muted":{"type":"boolean"}}}},
    {"name":"set_track_fx","description":"Set a track FX slot by one-based effect catalog id.","inputSchema":{"type":"object","required":["track","slot","effectId"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"slot":{"type":"integer","minimum":1,"maximum":3},"effectId":{"type":"integer","minimum":1}}}},
    {"name":"set_master_fx","description":"Set a master FX slot by one-based effect catalog id.","inputSchema":{"type":"object","required":["slot","effectId"],"properties":{"slot":{"type":"integer","minimum":1,"maximum":3},"effectId":{"type":"integer","minimum":1}}}},
    {"name":"set_key","description":"Set the project key, for example A minor or Db major.","inputSchema":{"type":"object","required":["key"],"properties":{"key":{"type":"string","minLength":1}}}},
    {"name":"set_progression","description":"Set the global chord progression by one-based catalog id.","inputSchema":{"type":"object","required":["id"],"properties":{"id":{"type":"integer","minimum":1}}}},
    {"name":"set_genre","description":"Set the current project genre by one-based id or name.","inputSchema":{"type":"object","properties":{"id":{"type":"integer","minimum":0,"maximum":8},"name":{"type":"string","minLength":1}}}},
    {"name":"randomize_for_genre","description":"Set a genre and randomize compatible tempo, key, progression, algorithms, and sounds. Use id=0 or name=none for unfiltered randomization.","inputSchema":{"type":"object","properties":{"id":{"type":"integer","minimum":0,"maximum":8},"name":{"type":"string","minLength":1}}}},
    {"name":"set_arrangement_section","description":"Set the current arrangement section by one-based section id.","inputSchema":{"type":"object","required":["section"],"properties":{"section":{"type":"integer","minimum":1}}}},
    {"name":"play_cendance","description":"Start cendance playback.","inputSchema":{"type":"object","properties":{}}},
    {"name":"pause_cendance","description":"Pause cendance playback.","inputSchema":{"type":"object","properties":{}}},
    {"name":"stop_playback","description":"Stop cendance playback and reset arrangement.","inputSchema":{"type":"object","properties":{}}},
    {"name":"set_tempo","description":"Set or adjust the tempo. Use set with a value, or delta with +/-N.","inputSchema":{"type":"object","required":["value"],"properties":{"value":{"type":"number"},"mode":{"type":"string","enum":["set","delta"],"default":"set"}}}},
    {"name":"get_cendance_preset_catalog","description":"Get the merged ref-based cendance preset catalog, including built-in and installed custom presets.","inputSchema":{"type":"object","properties":{}}},
    {"name":"list_cendance_packages","description":"List installed cendance contribution packages and their items.","inputSchema":{"type":"object","properties":{}}},
    {"name":"get_cendance_contribution_catalog","description":"Get installed cendance contribution catalog items grouped by package kind.","inputSchema":{"type":"object","properties":{}}},
    {"name":"preview_cendance_package","description":"Validate and preview a cendance contribution package without installing it.","inputSchema":{"type":"object","required":["path"],"properties":{"path":{"type":"string","minLength":1}}}},
    {"name":"install_cendance_package","description":"Install a previously reviewed cendance contribution package from a local path.","inputSchema":{"type":"object","required":["path"],"properties":{"path":{"type":"string","minLength":1}}}},
    {"name":"remove_cendance_package","description":"Remove an installed cendance contribution package by stable package id.","inputSchema":{"type":"object","required":["packageId"],"properties":{"packageId":{"type":"string","minLength":1}}}},
    {"name":"export_cendance_package","description":"Export a cendance contribution package template.","inputSchema":{"type":"object","required":["path","kind","packageId","name"],"properties":{"path":{"type":"string"},"kind":{"type":"string","enum":["effectPresetPack","soundPresetPack","drumKitPresetPack","scenePresetPack","arrangementPresetPack","samplePack"]},"packageId":{"type":"string"},"name":{"type":"string"}}}},
    {"name":"apply_cendance_contribution","description":"Apply an installed cendance contribution item by stable package and item id.","inputSchema":{"type":"object","required":["kind","packageId","itemId"],"properties":{"kind":{"type":"string","enum":["sound","effect","drumkit","arrangement","scene"]},"packageId":{"type":"string"},"itemId":{"type":"string"},"track":{"type":"integer","minimum":1,"maximum":5},"slot":{"type":"integer","minimum":1,"maximum":3}}}},
    {"name":"apply_cendance_preset_ref","description":"Apply a durable cendance PresetRef. Sound refs need only ref; effect refs also need track and slot.","inputSchema":{"type":"object","required":["ref"],"properties":{"ref":{"type":"string"},"track":{"type":"integer","minimum":1,"maximum":5},"slot":{"type":"integer","minimum":1,"maximum":3}}}},
    {"name":"set_track_sound_ref","description":"Set a track sound by durable PresetRef from the merged preset catalog.","inputSchema":{"type":"object","required":["ref"],"properties":{"ref":{"type":"string"}}}},
    {"name":"set_track_fx_ref","description":"Set a track FX slot by durable PresetRef from the merged preset catalog.","inputSchema":{"type":"object","required":["ref","track","slot"],"properties":{"ref":{"type":"string"},"track":{"type":"integer","minimum":1,"maximum":4},"slot":{"type":"integer","minimum":1,"maximum":3}}}},
    {"name":"set_master_fx_ref","description":"Set a master FX slot by durable PresetRef from the merged preset catalog.","inputSchema":{"type":"object","required":["ref","slot"],"properties":{"ref":{"type":"string"},"slot":{"type":"integer","minimum":1,"maximum":3}}}},
    {"name":"get_cendance_agent_authoring_guide","description":"Get the end-to-end workflow an agent should follow to make music, discover refs, author packages, install presets, and apply them without source access.","inputSchema":{"type":"object","properties":{}}},
    {"name":"get_cendance_package_schema","description":"Get machine-readable cendance contribution package schema, ref format, effect types, and examples for agent-authored preset packages.","inputSchema":{"type":"object","properties":{"kind":{"type":"string","enum":["effectPresetPack","soundPresetPack","drumKitPresetPack","scenePresetPack","arrangementPresetPack","samplePack"]}}}}
    ,{"name":"get_tos_status","description":"Get the current Terms of Service acceptance status.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"save_and_sign_preset","description":"Save the current project state and sign it for P2P sharing. Returns the signed envelope.","inputSchema":{"type":"object","properties":{"state_json":{"type":"string","description":"Optional state JSON to sign. If omitted, uses current app state."}}}}
    ,{"name":"share_on_network","description":"Share a signed preset envelope on the P2P network.","inputSchema":{"type":"object","required":["preset_json"],"properties":{"preset_json":{"type":"string","description":"The signed envelope JSON to share."}}}}
    ,{"name":"search_network","description":"Search the P2P network for available presets.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"verify_incoming_preset","description":"Verify an incoming preset envelope from the P2P network.","inputSchema":{"type":"object","required":["preset_json"],"properties":{"preset_json":{"type":"string","description":"The signed envelope JSON to verify."}}}}
    ,{"name":"list_downloaded_presets","description":"List all downloaded presets from the P2P network.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"save_and_sign_sample","description":"Read a WAV, FLAC, or OGG file and sign it for P2P sample sharing.","inputSchema":{"type":"object","required":["path"],"properties":{"path":{"type":"string"},"name":{"type":"string"},"description":{"type":"string"},"tags":{"type":"array","items":{"type":"string"}}}}}
    ,{"name":"share_sample_on_network","description":"Share a signed sample envelope on the P2P network.","inputSchema":{"type":"object","required":["sample_json"],"properties":{"sample_json":{"type":"string"}}}}
    ,{"name":"search_samples","description":"Search the P2P network for available samples.","inputSchema":{"type":"object","properties":{"query":{"type":"string"},"format":{"type":"string","enum":["wav","flac","ogg"]}}}}
    ,{"name":"download_sample","description":"Download, verify, and store a sample from the P2P network.","inputSchema":{"type":"object","required":["sample_id"],"properties":{"sample_id":{"type":"string"}}}}
    ,{"name":"list_downloaded_samples","description":"List downloaded P2P samples.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"create_custom_sound_preset","description":"Create a signed custom sound preset envelope for one track.","inputSchema":{"type":"object","required":["track"],"properties":{"track":{"type":"integer","minimum":1,"maximum":4},"includeSamples":{"type":"boolean","default":true}}}}

    ,{"name":"create_custom_algorithm","description":"Create a custom algorithm preset from a pattern JSON.","inputSchema":{"type":"object","required":["track_index","name","pattern_json"],"properties":{"track_index":{"type":"integer","minimum":0,"maximum":3},"name":{"type":"string"},"pattern_json":{"type":"string","description":"JSON serialized CustomAlgorithmPreset."}}}}
    ,{"name":"list_custom_algorithms","description":"List custom algorithm presets, optionally filtered by track or genre.","inputSchema":{"type":"object","properties":{"track_index":{"type":"integer","minimum":0,"maximum":3},"genre_id":{"type":"integer","minimum":1,"maximum":8}}}}
    ,{"name":"get_algorithm_pattern","description":"Get the full pattern data for a custom algorithm.","inputSchema":{"type":"object","required":["algorithm_id"],"properties":{"algorithm_id":{"type":"integer","minimum":2048},"track_index":{"type":"integer","minimum":0,"maximum":3}}}}
    ,{"name":"update_custom_algorithm","description":"Update an existing custom algorithm preset.","inputSchema":{"type":"object","required":["algorithm_id","pattern_json"],"properties":{"algorithm_id":{"type":"integer","minimum":2048},"track_index":{"type":"integer","minimum":0,"maximum":3},"pattern_json":{"type":"string"}}}}
    ,{"name":"delete_custom_algorithm","description":"Delete a custom algorithm preset.","inputSchema":{"type":"object","required":["algorithm_id"],"properties":{"algorithm_id":{"type":"integer","minimum":2048},"track_index":{"type":"integer","minimum":0,"maximum":3}}}}
    ,{"name":"share_algorithm_on_network","description":"Share a custom algorithm on the P2P network (requires ToS acceptance).","inputSchema":{"type":"object","required":["algorithm_id"],"properties":{"algorithm_id":{"type":"integer","minimum":2048},"track_index":{"type":"integer","minimum":0,"maximum":3}}}}
    ,{"name":"start_recording","description":"Start recording the audio output to a file.","inputSchema":{"type":"object","required":["path"],"properties":{"path":{"type":"string","description":"Output file path."},"format":{"type":"string","enum":["wav:f32","wav:s16","flac:24","flac:16"],"default":"wav:f32","description":"Recording format."}}}}
    ,{"name":"stop_recording","description":"Stop the current audio recording.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"get_recording_status","description":"Get the current recording status including duration and overrun info.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"start_streaming","description":"Start streaming raw audio to stdout.","inputSchema":{"type":"object","properties":{"format":{"type":"string","enum":["f32le","s16le"],"default":"f32le","description":"Stream format."}}}}
    ,{"name":"stop_streaming","description":"Stop the current audio stream.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"get_streaming_status","description":"Get the current streaming status.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"set_swing","description":"Set the global swing amount. Shifts even 8th notes for a groovier feel.","inputSchema":{"type":"object","required":["amount"],"properties":{"amount":{"type":"number","minimum":0,"maximum":100,"description":"Swing amount: 0=straight, 50=triplet feel, 100=max."}}}}
    ,{"name":"get_swing","description":"Get the current global swing amount.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"set_humanize","description":"Set velocity humanization and timing jitter for a more organic feel.","inputSchema":{"type":"object","properties":{"velocity":{"type":"number","minimum":0,"maximum":100,"description":"Velocity randomization: 0=none, 100=±50 velocity range."},"timing":{"type":"number","minimum":0,"maximum":100,"description":"Timing jitter: 0=none, 100=±10ms max offset."}}}}
    ,{"name":"get_humanize","description":"Get current humanization settings.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"save_arrangement_preset","description":"Save the current arrangement as a named preset.","inputSchema":{"type":"object","required":["name"],"properties":{"name":{"type":"string","description":"Name for the arrangement preset."}}}}
    ,{"name":"load_arrangement_preset","description":"Load a saved arrangement preset by ID.","inputSchema":{"type":"object","required":["preset_id"],"properties":{"preset_id":{"type":"string","description":"The preset ID to load."}}}}
    ,{"name":"list_arrangement_presets","description":"List all saved arrangement presets.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"delete_arrangement_preset","description":"Delete a saved arrangement preset.","inputSchema":{"type":"object","required":["preset_id"],"properties":{"preset_id":{"type":"string","description":"The preset ID to delete."}}}}
    ,{"name":"save_and_sign_arrangement","description":"Save the current arrangement and sign it for P2P sharing. Returns the signed envelope.","inputSchema":{"type":"object","required":["name"],"properties":{"name":{"type":"string","description":"Name for the arrangement preset."}}}}
    ,{"name":"share_arrangement_on_network","description":"Share a signed arrangement envelope on the P2P network.","inputSchema":{"type":"object","required":["envelope_json"],"properties":{"envelope_json":{"type":"string","description":"The signed envelope JSON to share."}}}}
    ,{"name":"search_arrangements","description":"Search the P2P network for available arrangement presets.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"download_arrangement","description":"Download, verify, and apply an arrangement preset from the P2P network.","inputSchema":{"type":"object","required":["envelope_json"],"properties":{"envelope_json":{"type":"string","description":"The signed envelope JSON to download and apply."}}}}
    ,{"name":"save_and_sign_project","description":"Save the current project state and sign it for P2P sharing. Returns the signed envelope.","inputSchema":{"type":"object","properties":{"name":{"type":"string","description":"Optional name for the project. If omitted, uses the current project name."}}}}
    ,{"name":"share_project_on_network","description":"Share a signed project envelope on the P2P network.","inputSchema":{"type":"object","required":["envelope_json"],"properties":{"envelope_json":{"type":"string","description":"The signed envelope JSON to share."}}}}
    ,{"name":"search_projects","description":"Search the P2P network for available project files.","inputSchema":{"type":"object","properties":{}}}
    ,{"name":"download_project","description":"Download and verify a project file from the P2P network. Saves it locally as a .cendance file.","inputSchema":{"type":"object","required":["envelope_json"],"properties":{"envelope_json":{"type":"string","description":"The signed envelope JSON to download and verify."}}}}
    ,{"name":"list_downloaded_projects","description":"List all downloaded project files.","inputSchema":{"type":"object","properties":{}}}





    ])json");
}

//==============================================================================
// tools/list
//==============================================================================

juce::var McpServer::handleToolsList (juce::var const&, const juce::var& id)
{
    auto* res = new juce::DynamicObject();
    res->setProperty("tools", toolSchemas());
    return makeResult(juce::var(res), id);
}

//==============================================================================
// tools/call — the big dispatcher
//==============================================================================

juce::var McpServer::handleToolsCall (juce::var const& params, const juce::var& id)
{
    auto toolName = getString(params.getProperty("name", juce::var()), "");
    if (toolName.isEmpty())
        return makeJsonError(-32602, "Missing tool name", id);

    juce::var args = params.getProperty("arguments", juce::var());
    if (args.isVoid() || !args.isObject())
        args = juce::var(new juce::DynamicObject());

    juce::String cmd; // agent protocol command string

    // ─── State / catalog / meters ───
    if (toolName == "get_cendance_state") {
        bool full = getBool(args.getProperty("full",false), false);
        cmd = full ? "state full" : "state";
    }
    else if (toolName == "get_cendance_catalog") {
        auto kind = getString(args.getProperty("kind",juce::var()), "");
        if (kind.isEmpty()) return makeJsonError(-32602, "kind is required", id);
        cmd = "catalog " + kind;
    }
    else if (toolName == "get_cendance_meters") {
        cmd = "meters";
    }
    else if (toolName == "listen_to_cendance") {
        double sec = getDouble(args.getProperty("seconds",8.0), 8.0);
        sec = juce::jlimit(0.25, 60.0, sec);
        cmd = "listen " + juce::String(sec) + "s";
    }
    else if (toolName == "get_cendance_preset_catalog") {
        cmd = "presets catalog";
    }

    // ─── Track controls ───
    else if (toolName == "set_track_parameter") {
        int trk = getInt(args.getProperty("track",0), 0);
        auto par = getString(args.getProperty("parameter",juce::var()), "");
        double val = getDouble(args.getProperty("value",-1.0), -1.0);
        if (trk<1||trk>4) return makeJsonError(-32602,"track must be 1-4",id);
        if (par.isEmpty()) return makeJsonError(-32602,"parameter is required",id);
        if (val<0||val>2)  return makeJsonError(-32602,"value must be 0-2",id);
        cmd = "track " + juce::String(trk) + " " + par + " " + juce::String(val);
    }
    else if (toolName == "set_track_algorithm") {
        int trk = getInt(args.getProperty("track",0),0);
        int aid = getInt(args.getProperty("id",0),0);
        if (trk<1||trk>4) return makeJsonError(-32602,"track must be 1-4",id);
        if (aid<1)        return makeJsonError(-32602,"id must be >= 1",id);
        cmd = "track " + juce::String(trk) + " algorithm " + juce::String(aid);
    }
    else if (toolName == "set_track_sound") {
        int trk = getInt(args.getProperty("track",0),0);
        int sid = getInt(args.getProperty("id",0),0);
        if (trk<1||trk>4) return makeJsonError(-32602,"track must be 1-4",id);
        if (sid<1)        return makeJsonError(-32602,"id must be >= 1",id);
        cmd = "track " + juce::String(trk) + " sound " + juce::String(sid);
    }
    else if (toolName == "set_track_mute") {
        int trk = getInt(args.getProperty("track",0),0);
        bool mut = getBool(args.getProperty("muted",false), false);
        if (trk<1||trk>4) return makeJsonError(-32602,"track must be 1-4",id);
        cmd = "track " + juce::String(trk) + " mute " + (mut ? "on" : "off");
    }
    else if (toolName == "set_track_fx") {
        int trk = getInt(args.getProperty("track",0),0);
        int slt = getInt(args.getProperty("slot",0),0);
        int eid = getInt(args.getProperty("effectId",0),0);
        if (trk<1||trk>4) return makeJsonError(-32602,"track must be 1-4",id);
        if (slt<1||slt>3) return makeJsonError(-32602,"slot must be 1-3",id);
        if (eid<1)       return makeJsonError(-32602,"effectId must be >= 1",id);
        cmd = "track " + juce::String(trk) + " fx " + juce::String(slt) + " " + juce::String(eid);
    }

    // ─── Master FX ───
    else if (toolName == "set_master_fx") {
        int slt = getInt(args.getProperty("slot",0),0);
        int eid = getInt(args.getProperty("effectId",0),0);
        if (slt<1||slt>3) return makeJsonError(-32602,"slot must be 1-3",id);
        if (eid<1)        return makeJsonError(-32602,"effectId must be >= 1",id);
        cmd = "master fx " + juce::String(slt) + " " + juce::String(eid);
    }

    // ─── Musical ───
    else if (toolName == "set_key") {
        auto key = getString(args.getProperty("key",juce::var()), "");
        if (key.isEmpty()) return makeJsonError(-32602,"key is required",id);
        cmd = "key " + quoteArg(key);
    }
    else if (toolName == "set_progression") {
        int pid = getInt(args.getProperty("id",0),0);
        if (pid<1) return makeJsonError(-32602,"id must be >= 1",id);
        cmd = "progression " + juce::String(pid);
    }
    else if (toolName == "set_genre" || toolName == "randomize_for_genre") {
        auto name = getString(args.getProperty("name", juce::var()), "");
        auto idValue = args.getProperty("id", juce::var());
        const bool hasId = !idValue.isVoid();
        if (name.isEmpty() && !hasId) return makeJsonError(-32602, "id or name is required", id);

        juce::String genreArg;
        if (hasId) {
            const int gid = getInt(idValue, -1);
            if (gid < 0 || gid > 8) {
                return makeJsonError(-32602,
                    "id must be 0-8",
                    id);
            }
            genreArg = juce::String(gid);
        } else {
            genreArg = quoteArg(name);
        }

        cmd = toolName == "randomize_for_genre"
            ? "genre randomize " + genreArg
            : "genre " + genreArg;
    }
    else if (toolName == "set_arrangement_section") {
        int sec = getInt(args.getProperty("section",0),0);
        if (sec<1) return makeJsonError(-32602,"section must be >= 1",id);
        cmd = "arrangement section " + juce::String(sec);
    }

    // ─── Transport ───
    else if (toolName == "play_cendance")     cmd = "play";
    else if (toolName == "pause_cendance")    cmd = "pause";
    else if (toolName == "stop_playback")   cmd = "stop";
    else if (toolName == "set_tempo") {
        double val = getDouble(args.getProperty("value",-1.0),120.0);
        auto mode = getString(args.getProperty("mode",juce::var()), "set");
        if (mode == "set" || mode.isEmpty())
            cmd = "tempo set " + juce::String((int)val);
        else
            cmd = "tempo " + juce::String(val);
    }

    // ─── Preset refs ───
    else if (toolName == "apply_cendance_preset_ref") {
        auto ref = getString(args.getProperty("ref",juce::var()), "");
        if (ref.isEmpty()) return makeJsonError(-32602,"ref is required",id);
        juce::var tv = args.getProperty("track",juce::var());
        juce::var sv = args.getProperty("slot",juce::var());
        if (!tv.isVoid() && !sv.isVoid())
            cmd = "presets apply " + quoteArg(ref) + " " + juce::String(getInt(tv,1)) + " " + juce::String(getInt(sv,1));
        else
            cmd = "presets apply " + quoteArg(ref);
    }
    else if (toolName == "set_track_sound_ref") {
        auto ref = getString(args.getProperty("ref",juce::var()), "");
        if (ref.isEmpty()) return makeJsonError(-32602,"ref is required",id);
        cmd = "presets apply " + quoteArg(ref);
    }
    else if (toolName == "set_track_fx_ref") {
        auto ref = getString(args.getProperty("ref",juce::var()), "");
        int trk = getInt(args.getProperty("track",0),0);
        int slt = getInt(args.getProperty("slot",0),0);
        if (ref.isEmpty()) return makeJsonError(-32602,"ref required",id);
        if (trk<1||trk>4)  return makeJsonError(-32602,"track must be 1-4",id);
        if (slt<1||slt>3)  return makeJsonError(-32602,"slot must be 1-3",id);
        cmd = "presets apply " + quoteArg(ref) + " " + juce::String(trk) + " " + juce::String(slt);
    }
    else if (toolName == "set_master_fx_ref") {
        auto ref = getString(args.getProperty("ref",juce::var()), "");
        int slt = getInt(args.getProperty("slot",0),0);
        if (ref.isEmpty()) return makeJsonError(-32602,"ref required",id);
        if (slt<1||slt>3)  return makeJsonError(-32602,"slot must be 1-3",id);
        cmd = "presets apply " + quoteArg(ref) + " 5 " + juce::String(slt);
    }

    // ─── Packages ───
    else if (toolName == "list_cendance_packages")
        cmd = "packages list";
    else if (toolName == "get_cendance_contribution_catalog")
        cmd = "packages catalog";
    else if (toolName == "preview_cendance_package") {
        auto p = getString(args.getProperty("path",juce::var()), "");
        if (p.isEmpty()) return makeJsonError(-32602,"path required",id);
        cmd = "packages preview " + quoteArg(p);
    }
    else if (toolName == "install_cendance_package") {
        auto p = getString(args.getProperty("path",juce::var()), "");
        if (p.isEmpty()) return makeJsonError(-32602,"path required",id);
        cmd = "packages install " + quoteArg(p);
    }
    else if (toolName == "remove_cendance_package") {
        auto p = getString(args.getProperty("packageId",juce::var()), "");
        if (p.isEmpty()) return makeJsonError(-32602,"packageId required",id);
        cmd = "packages remove " + quoteArg(p);
    }
    else if (toolName == "export_cendance_package") {
        auto pa = getString(args.getProperty("path",juce::var()), "");
        auto ki = getString(args.getProperty("kind",juce::var()), "");
        auto pi = getString(args.getProperty("packageId",juce::var()), "");
        auto nm = getString(args.getProperty("name",juce::var()), "");
        if (pa.isEmpty()||ki.isEmpty()||pi.isEmpty()||nm.isEmpty())
            return makeJsonError(-32602,"path, kind, packageId, name required",id);
        cmd = "packages export " + quoteArg(pa) + " " + ki + " " + quoteArg(pi) + " " + quoteArg(nm);
    }
    else if (toolName == "apply_cendance_contribution") {
        auto ki = getString(args.getProperty("kind",juce::var()), "");
        auto pi = getString(args.getProperty("packageId",juce::var()), "");
        auto ii = getString(args.getProperty("itemId",juce::var()), "");
        if (ki.isEmpty()||pi.isEmpty()||ii.isEmpty())
            return makeJsonError(-32602,"kind, packageId, itemId required",id);
        juce::var tv = args.getProperty("track",juce::var());
        juce::var sv = args.getProperty("slot",juce::var());
        if (ki == "effect" && !tv.isVoid() && !sv.isVoid())
            cmd = "packages apply " + ki + " " + quoteArg(pi) + " " + quoteArg(ii)
                  + " " + juce::String(getInt(tv,1)) + " " + juce::String(getInt(sv,1));
        else
            cmd = "packages apply " + ki + " " + quoteArg(pi) + " " + quoteArg(ii);
    }

    // ─── Static data ───
    else if (toolName == "get_cendance_agent_authoring_guide") {
        return makeResult(makeTextContent(juce::JSON::toString(getAgentAuthoringGuide(),false)), id);
    }
    else if (toolName == "get_cendance_package_schema") {
        auto k = getString(args.getProperty("kind",juce::var()), "");
        return makeResult(makeTextContent(juce::JSON::toString(getPackageSchema(k),false)), id);
    }

    // ─── Custom Algorithm tools ───
    else if (toolName == "create_custom_algorithm"
             || toolName == "list_custom_algorithms"
             || toolName == "get_algorithm_pattern"
             || toolName == "update_custom_algorithm"
             || toolName == "delete_custom_algorithm"
             || toolName == "share_algorithm_on_network") {
        juce::String argsJson = juce::JSON::toString(args, false);
        if (p2pFn_)
            return makeResult(makeTextContent(p2pFn_(toolName, argsJson)), id);
        cmd = "p2p " + toolName + " " + argsJson;
    }

    // ─── P2P Preset Sharing tools ───
    else if (toolName == "get_tos_status"
             || toolName == "save_and_sign_preset"
             || toolName == "share_on_network"
             || toolName == "search_network"
             || toolName == "verify_incoming_preset"
             || toolName == "list_downloaded_presets"
             || toolName == "save_and_sign_sample"
             || toolName == "share_sample_on_network"
             || toolName == "search_samples"
             || toolName == "download_sample"
             || toolName == "list_downloaded_samples"
             || toolName == "create_custom_sound_preset") {
        juce::String argsJson = juce::JSON::toString(args, false);
        if (p2pFn_)
            return makeResult(makeTextContent(p2pFn_(toolName, argsJson)), id);

        // Fallback for older wiring: route P2P tools through executeFn_.
        cmd = "p2p " + toolName + " " + argsJson;
    }


    // ─── Recording tools ───
    else if (toolName == "start_recording"
             || toolName == "stop_recording"
             || toolName == "get_recording_status") {
        std::string action;
        if (toolName == "start_recording") action = "start";
        else if (toolName == "stop_recording") action = "stop";
        else action = "status";
        cmd = "record " + juce::String(action) + " " + juce::JSON::toString(args, false);
    }

    // ─── Streaming tools ───
    else if (toolName == "start_streaming"
             || toolName == "stop_streaming"
             || toolName == "get_streaming_status") {
        std::string action;
        if (toolName == "start_streaming") action = "start";
        else if (toolName == "stop_streaming") action = "stop";
        else action = "status";
        cmd = "stream " + juce::String(action) + " " + juce::JSON::toString(args, false);
    }

    // ─── Groove / Swing / Humanize ───
    else if (toolName == "set_swing") {
        double amount = getDouble(args.getProperty("amount", 0.0), 0.0);
        amount = juce::jlimit(0.0, 100.0, amount);
        cmd = "swing set " + juce::String(amount);
    }
    else if (toolName == "get_swing") {
        cmd = "swing get";
    }
    else if (toolName == "set_humanize") {
        double vel = getDouble(args.getProperty("velocity", -1.0), -1.0);
        double timing = getDouble(args.getProperty("timing", -1.0), -1.0);
        if (vel < 0 && timing < 0) return makeJsonError(-32602, "velocity or timing required", id);
        if (vel >= 0) vel = juce::jlimit(0.0, 100.0, vel);
        if (timing >= 0) timing = juce::jlimit(0.0, 100.0, timing);
        if (vel >= 0 && timing >= 0)
            cmd = "humanize set " + juce::String(vel) + " " + juce::String(timing);
        else if (vel >= 0)
            cmd = "humanize set " + juce::String(vel);
        else
            cmd = "humanize set 0 " + juce::String(timing);
    }
    else if (toolName == "get_humanize") {
        cmd = "humanize get";
    }

    // ─── Arrangement presets ───
    else if (toolName == "save_arrangement_preset") {
        auto name = getString(args.getProperty("name", juce::var()), "");
        if (name.isEmpty()) return makeJsonError(-32602, "name is required", id);
        cmd = "arrangement save " + name;
    }
    else if (toolName == "load_arrangement_preset") {
        auto presetId = getString(args.getProperty("preset_id", juce::var()), "");
        if (presetId.isEmpty()) return makeJsonError(-32602, "preset_id is required", id);
        cmd = "arrangement load " + presetId;
    }
    else if (toolName == "list_arrangement_presets") {
        cmd = "arrangement list";
    }
    else if (toolName == "delete_arrangement_preset") {
        auto presetId = getString(args.getProperty("preset_id", juce::var()), "");
        if (presetId.isEmpty()) return makeJsonError(-32602, "preset_id is required", id);
        cmd = "arrangement delete " + presetId;
    }
    else if (toolName == "save_and_sign_arrangement"
             || toolName == "share_arrangement_on_network"
             || toolName == "search_arrangements"
             || toolName == "download_arrangement"
             || toolName == "save_and_sign_project"
             || toolName == "share_project_on_network"
             || toolName == "search_projects"
             || toolName == "download_project"
             || toolName == "list_downloaded_projects") {
        // These go through the P2P bridge
        if (!p2pFn_) return makeJsonError(-32602, "P2P not available", id);
        juce::String argsJson = juce::JSON::toString(args, false);
        juce::String result = p2pFn_(toolName, argsJson);
        return makeResult(makeTextContent(result), id);
    }

    // ─── Raw command ───
    else if (toolName == "send_cendance_command") {
        auto c = getString(args.getProperty("command",juce::var()), "");
        if (c.isEmpty()) return makeJsonError(-32602,"command required",id);
        cmd = c;
    }
    else {
        return makeJsonError(-32602, "Unknown tool: " + toolName, id);
    }

    // Execute via the injected execute function (same path as TCP agent protocol)
    juce::String response = executeFn_(cmd);
    return makeResult(makeTextContent(response), id);
}
