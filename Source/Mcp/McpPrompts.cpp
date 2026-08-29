#include "McpServer.h"

//==============================================================================
// prompts/list — returns available prompt templates
//==============================================================================

juce::var McpServer::handlePromptsList (juce::var const&, const juce::var& id)
{
    auto* list = new juce::DynamicObject();
    juce::Array<juce::var> prompts;

    auto* p1 = new juce::DynamicObject();
    p1->setProperty("name", "start-project");
    p1->setProperty("description", "Start a new cendance project: set genre, tempo, key, and progression for a quick creative session.");
    juce::Array<juce::var> p1args;
    auto* p1a1 = new juce::DynamicObject();
    p1a1->setProperty("name", "genre");
    p1a1->setProperty("description", "Target genre. One of: House, UK Garage, DnB, Trap, Hip-Hop, Techno, Trance, Synth Pop. Leave blank for a random choice.");
    p1a1->setProperty("required", false);
    p1args.add(juce::var(p1a1));
    auto* p1a2 = new juce::DynamicObject();
    p1a2->setProperty("name", "mood");
    p1a2->setProperty("description", "Mood or energy level (e.g. chill, upbeat, dark, driving). Influences algorithm density and FX choices.");
    p1a2->setProperty("required", false);
    p1->setProperty("arguments", p1args);
    prompts.add(juce::var(p1));

    auto* p2 = new juce::DynamicObject();
    p2->setProperty("name", "analyze-improve");
    p2->setProperty("description", "Evaluate the current mix using listen_to_cendance and get_cendance_meters, then suggest track parameter and FX adjustments.");
    prompts.add(juce::var(p2));

    auto* p3 = new juce::DynamicObject();
    p3->setProperty("name", "design-package");
    p3->setProperty("description", "Step through the full cendance contribution package authoring workflow: discover refs, write JSON, preview, install, apply.");
    juce::Array<juce::var> p3args;
    auto* p3a1 = new juce::DynamicObject();
    p3a1->setProperty("name", "kind");
    p3a1->setProperty("description", "Package kind: effectPresetPack, soundPresetPack, drumKitPresetPack, scenePresetPack, or arrangementPresetPack.");
    p3a1->setProperty("required", true);
    p3args.add(juce::var(p3a1));
    p3->setProperty("arguments", p3args);
    prompts.add(juce::var(p3));

    auto* p4 = new juce::DynamicObject();
    p4->setProperty("name", "genre-randomize");
    p4->setProperty("description", "Set a genre and randomize compatible tempo, key, progression, algorithms, and sounds.");
    juce::Array<juce::var> p4args;
    auto* p4a1 = new juce::DynamicObject();
    p4a1->setProperty("name", "genre");
    p4a1->setProperty("description", "Genre to lock to. One of: House, UK Garage, DnB, Trap, Hip-Hop, Techno, Trance, Synth Pop.");
    p4a1->setProperty("required", true);
    p4args.add(juce::var(p4a1));
    p4->setProperty("arguments", p4args);
    prompts.add(juce::var(p4));

    list->setProperty("prompts", prompts);
    return makeResult(juce::var(list), id);
}

//==============================================================================
// prompts/get — returns a templated messages array for a named prompt
//==============================================================================

juce::var McpServer::handlePromptsGet (juce::var const& params, const juce::var& id)
{
    auto name = getString(params.getProperty("name", juce::var()), "");
    if (name.isEmpty())
        return makeJsonError(-32602, "name is required", id);

    juce::var args = params.getProperty("arguments", juce::var());
    if (args.isVoid() || !args.isObject())
        args = juce::var(new juce::DynamicObject());

    auto genreFromArgs = getString(args.getProperty("genre", juce::var()), "");
    auto moodFromArgs  = getString(args.getProperty("mood", juce::var()), "");
    auto kindFromArgs  = getString(args.getProperty("kind", juce::var()), "");

    juce::Array<juce::var> messages;

    if (name == "start-project")
    {
        juce::String body = "Start a new cendance project session. ";
        if (genreFromArgs.isNotEmpty())
            body += "Set the genre to '" + genreFromArgs + "'. ";
        else
            body += "Pick a random genre. ";
        if (moodFromArgs.isNotEmpty())
            body += "The mood is '" + moodFromArgs + "' — let that guide your algorithm and FX choices. ";
        else
            body += "Choose balanced density and complexity values. ";
        body += "Follow these steps:\n"
            "1. Call `get_cendance_state` to check the current state.\n"
            "2. Call `get_cendance_preset_catalog` and copy refs for sounds you want.\n"
            "3. Set tempo, key, and a chord progression.\n"
            "4. Configure each track (density, complexity, tone, motion, gain) with values between 0 and 2.\n"
            "5. Optionally apply FX using ref-based preset tools.\n"
            "6. Start playback with `play_cendance` and evaluate with `listen_to_cendance`.";

        auto* m1 = new juce::DynamicObject();
        m1->setProperty("role", "user");
        m1->setProperty("content", body);
        messages.add(juce::var(m1));
    }
    else if (name == "analyze-improve")
    {
        juce::String body = "Analyze and improve the current cendance mix. Follow these steps:\n"
            "1. Call `get_cendance_state` to get a full snapshot of the current project.\n"
            "2. Call `get_cendance_meters` to see levels per track.\n"
            "3. Call `listen_to_cendance` with seconds=8 to analyze the audio.\n"
            "4. Based on what you hear and see, suggest adjustments:\n"
            "   - If tracks clash: adjust density/complexity or apply an EQ/reverb.\n"
            "   - If the mix is flat: try adding transient shaper, chorus, or a subtle delay.\n"
            "   - If levels are unbalanced: adjust gain per track (0-2 range).\n"
            "   - If the arrangement feels static: try switching arrangement sections.\n"
            "5. Apply one or two changes, then call `listen_to_cendance` again to compare.\n"
            "Report what you changed and why.";

        auto* m1 = new juce::DynamicObject();
        m1->setProperty("role", "user");
        m1->setProperty("content", body);
        messages.add(juce::var(m1));
    }
    else if (name == "design-package")
    {
        juce::String body = "Design and publish a cendance contribution package.";
        if (kindFromArgs.isNotEmpty())
            body += " The package kind is '" + kindFromArgs + "'. ";
        else
            body += " ";
        body += "Follow these steps:\n"
            "1. Call `get_cendance_package_schema` with the correct kind to learn the item structure.\n"
            "2. Call `get_cendance_preset_catalog` to discover refs from built-in sources and installed packages.\n"
            "3. Write a `.cendance-package.json` file. Key rules:\n"
            "   - Use a stable, unique id (reverse-DNS, e.g. `myagent.pack.name`).\n"
            "   - Every package item needs a unique id and a human-readable name.\n"
            "   - For effect presets: include `effectType` (see schema) + `paramA`/`paramB`/`paramC`.\n"
            "   - For sound presets: reference an existing engine via `soundRef` or use an existing sound display id.\n"
            "   - Never invent refs — copy them from the catalog.\n"
            "4. Call `preview_cendance_package` with the file path. Fix any validation errors.\n"
            "5. After preview passes, call `install_cendance_package`.\n"
            "6. Call `get_cendance_preset_catalog` again and verify your new refs appear.\n"
            "7. Apply one of your new presets and confirm it works.";

        auto* m1 = new juce::DynamicObject();
        m1->setProperty("role", "user");
        m1->setProperty("content", body);
        messages.add(juce::var(m1));
    }
    else if (name == "genre-randomize")
    {
        juce::String body = "Set the genre to '" + genreFromArgs + "' and randomize the project within genre constraints.";
        body += " The available genres are:\n"
            "- House (118-130 BPM)\n"
            "- UK Garage (130-140 BPM)\n"
            "- DnB (160-175 BPM)\n"
            "- Trap (130-150 BPM)\n"
            "- Hip-Hop (80-100 BPM)\n"
            "- Techno (120-140 BPM)\n"
            "- Trance (130-150 BPM)\n"
            "- Synth Pop (100-130 BPM)\n\n"
            "Follow these steps:\n"
            "1. Call `randomize_for_genre` with the requested genre name.\n"
            "2. Call `get_cendance_state` with full=true to confirm genre, tempo, key, progression, algorithms, and sounds.\n"
            "3. Optionally fine-tune density/complexity/tone/motion/gain values (0-2).\n"
            "4. Apply FX if appropriate (EQ, reverb, delay, etc.) using ref-based tools.\n"
            "5. Call `play_cendance` when ready.\n"
            "6. Use `listen_to_cendance` to evaluate and iterate.";

        auto* m1 = new juce::DynamicObject();
        m1->setProperty("role", "user");
        m1->setProperty("content", body);
        messages.add(juce::var(m1));
    }
    else
    {
        return makeJsonError(-32602, "Unknown prompt: " + name, id);
    }

    auto* res = new juce::DynamicObject();
    res->setProperty("messages", messages);
    return makeResult(juce::var(res), id);
}
