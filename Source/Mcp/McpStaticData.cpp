#include "McpServer.h"
#include <juce_core/juce_core.h>

//==============================================================================
// Static data — ported from authoringGuide.ts
//==============================================================================

juce::var McpServer::getAgentAuthoringGuide()
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("ok", true);

    juce::Array<juce::var> workflow;
    workflow.add ("1. Call get_cendance_preset_catalog and copy refs for built-in or custom presets.");
    workflow.add ("2. Call get_cendance_package_schema for the package kind you want to author.");
    workflow.add ("3. Write a local .cendance-package.json file using the schema and copied refs.");
    workflow.add ("4. Call preview_cendance_package to validate. Fix any errors.");
    workflow.add ("5. Call install_cendance_package after a successful preview.");
    workflow.add ("6. Call get_cendance_preset_catalog again and find your new package refs.");
    workflow.add ("7. Apply with apply_cendance_preset_ref, set_track_sound_ref, set_track_fx_ref, or set_master_fx_ref.");
    workflow.add ("8. Use listen_to_cendance and get_cendance_meters to evaluate the result.");
    obj->setProperty ("workflow", workflow);

    auto* rf = new juce::DynamicObject();
    rf->setProperty ("stableString", "<domain>:<source>:<id>");
    rf->setProperty ("builtin", "<domain>:builtin:<builtinSlug>");
    rf->setProperty ("package", "<domain>:package:<packageId>:<itemId>[@<packageVersion>]");
    juce::Array<juce::var> domains;
    for (auto d : {"effect","sound","drumKit","arrangement","scene","sample","midiGenerator","instrumentEngine"})
        domains.add (juce::var(d));
    rf->setProperty ("domains", domains);
    rf->setProperty ("guidance", "Never invent built-in refs. Call get_cendance_preset_catalog, then copy refs from catalog entries.");
    obj->setProperty ("refFormat", juce::var(rf));

    juce::Array<juce::var> kinds;
    for (auto k : {"effectPresetPack","soundPresetPack","drumKitPresetPack","scenePresetPack","arrangementPresetPack","samplePack"})
        kinds.add (juce::var(k));
    obj->setProperty ("packageKinds", kinds);

    juce::Array<juce::var> rules;
    rules.add ("Do not invent built-in refs; discover them from get_cendance_preset_catalog.");
    rules.add ("Do not claim numeric preset ids; package item ids are namespaced under packageId.");
    rules.add ("Use preview_cendance_package before install_cendance_package.");
    rules.add ("For v1, packages are data-only. Do not include executable code.");
    rules.add ("Sample packs may include WAV/FLAC payload files listed in payloads and item filePath fields.");
    obj->setProperty ("creationRules", rules);

    juce::Array<juce::var> rec;
    for (auto t : {"get_cendance_preset_catalog","get_cendance_package_schema","preview_cendance_package",
                   "install_cendance_package","apply_cendance_preset_ref","listen_to_cendance"})
        rec.add (juce::var(t));
    obj->setProperty ("recommendedTools", rec);

    return juce::var(obj);
}

juce::var McpServer::getPackageSchema (const juce::String& kind)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("ok", true);
    obj->setProperty ("schema", "cendancePackage.v1");

    auto* tlf = new juce::DynamicObject();
    tlf->setProperty ("schema","cendancePackage.v1");
    tlf->setProperty ("id","Stable package id. Use reverse-DNS or agent namespace, e.g. agent.demo.gloss.");
    tlf->setProperty ("version","Semantic version string chosen by the authoring agent.");
    juce::Array<juce::var> allKinds;
    for (auto k : {"effectPresetPack","soundPresetPack","drumKitPresetPack","scenePresetPack","arrangementPresetPack","samplePack"})
        allKinds.add (juce::var(k));
    tlf->setProperty ("kind", allKinds);
    tlf->setProperty ("name","Human-readable package name.");
    tlf->setProperty ("description","Short package description.");
    tlf->setProperty ("authorAgent","Agent name or network identity.");
    tlf->setProperty ("createdAt","ISO-8601 timestamp.");
    tlf->setProperty ("license","License identifier, e.g. CC0-1.0.");
    tlf->setProperty ("contentHash","Optional. If omitted, preview reports the computed hash.");
    tlf->setProperty ("signature","Optional. Unsigned packages are accepted for local development.");
    auto* compat = new juce::DynamicObject();
    compat->setProperty ("minPackageSchema","cendancePackage.v1");
    compat->setProperty ("maxPackageSchema","cendancePackage.v1");
    tlf->setProperty ("compatibility", juce::var(compat));
    tlf->setProperty ("dependencies","Optional array of package ids.");
    tlf->setProperty ("tags","Optional array of strings.");
    tlf->setProperty ("payloads","Optional array of relative payload paths. samplePack items may reference WAV/FLAC files.");
    tlf->setProperty ("items","Non-empty array. Shape depends on kind.");
    obj->setProperty ("topLevelFields", juce::var(tlf));

    auto* rff = new juce::DynamicObject();
    rff->setProperty ("stableString","<domain>:<source>:<id>");
    rff->setProperty ("builtin","<domain>:builtin:<builtinSlug>");
    rff->setProperty ("package","<domain>:package:<packageId>:<itemId>[@<packageVersion>]");
    juce::Array<juce::var> dns;
    for (auto d : {"effect","sound","drumKit","arrangement","scene","sample","midiGenerator","instrumentEngine"})
        dns.add (juce::var(d));
    rff->setProperty ("domains", dns);
    rff->setProperty ("guidance","Never invent built-in refs. Call get_cendance_preset_catalog, then copy refs from catalog entries.");
    obj->setProperty ("refFormat", juce::var(rff));

    juce::Array<juce::var> effectTypes;
    for (auto e : {"None","HighPassSweep","ReverbWash","ReduxCrush","DelayEcho",
                   "SaturationWaveshaper","SoftHardClip","Wavefolder","AsymShaper",
                   "CompressorGlue","PeakLimiter","TransientShaper","CombFilter",
                   "MultiModeEQ","FormantFilter","Autopan","RingModulator","Chorus",
                   "Phaser","Flanger","JitterDegrade","ErosionDegrade","TranceGate",
                   "SidechainDucker","BeatRepeatInsert","FrequencyShifter","PitchShifter",
                   "Harmonizer","TimeFreezer","GrainDelay","PhysicalModelingResonator"})
        effectTypes.add (juce::var(e));
    obj->setProperty ("effectTypes", effectTypes);

    auto* itemSchemas = new juce::DynamicObject();
    auto addSchema = [itemSchemas] (const char* name, std::initializer_list<const char*> req,
                                     std::initializer_list<const char*> opt,
                                     std::initializer_list<const char*> notes)
    {
        auto* s = new juce::DynamicObject();
        juce::Array<juce::var> ra, oa, na;
        for (auto v : req) ra.add (juce::var(v));
        for (auto v : opt) oa.add (juce::var(v));
        for (auto v : notes) na.add (juce::var(v));
        s->setProperty ("required", ra);
        s->setProperty ("optional", oa);
        if (na.size() > 0) s->setProperty ("notes", na);
        itemSchemas->setProperty (name, juce::var(s));
    };

    addSchema ("effectPresetPack",
        {"id","name","effectType"},
        {"description","paramA","paramB","paramC","basedOnEffectRef","basedOnEffectDisplayId","tags"},
        {"Custom effect presets are data-only EffectType plus paramA/B/C.",
         "They become playable after install through effect:package:<packageId>:<itemId> refs."});

    addSchema ("soundPresetPack",
        {"id","name","track","soundDisplayId or soundRef"},
        {"description","fx","density","complexity","tone","motion","gain","tags"},
        {"track is one-based: 1 drums, 2 bass, 3 chords, 4 lead.",
         "soundRef should be copied from get_cendance_preset_catalog.",
         "fx entries may be numeric effect display ids or objects like { ref: 'effect:...' }."});

    addSchema ("drumKitPresetPack",
        {"id","name","slots"},
        {"description","fx","tags"},
        {"slots must have 4 entries.",
         "Each slot supports sampleId, volume, tuneSemitones, startOffset, decay, velocitySensitivity.",
         "fx entries may be numeric effect display ids or objects like { ref: 'effect:...' }."});

    addSchema ("arrangementPresetPack",
        {"id","name"},
        {"description","sectionCount","currentSection","mode","sectionLengths","sectionProgressions",
         "trackMasks","chainEnabled","chainLength","chainSequence","tags"},
        {});

    addSchema ("scenePresetPack",
        {"id","name"},
        {"description","bpm","progressionDisplayId","key","fx","masterGain","tracks","arrangement","tags"},
        {"tracks entries use one-based track plus optional algorithmDisplayId, soundDisplayId,",
         "muted, fx, and macro fields.",
         "fx entries support the same numeric id or ref object shape as sound presets."});

    addSchema ("samplePack",
        {"id","name","filePath","format","sampleRate","channels","duration","sha256"},
        {"description","tags"},
        {"filePath must be a relative payload path with .wav or .flac extension.",
         "format must be wav or flac, and payloads should list each referenced file."});

    obj->setProperty ("itemSchemas", juce::var(itemSchemas));

    if (kind.isEmpty() || kind == "effectPresetPack") {
        auto* ex = new juce::DynamicObject();
        ex->setProperty ("schema","cendancePackage.v1"); ex->setProperty ("id","agent.demo.fx");
        ex->setProperty ("version","0.1.0"); ex->setProperty ("kind","effectPresetPack");
        ex->setProperty ("name","Agent Demo FX"); ex->setProperty ("description","Agent-authored FX presets.");
        ex->setProperty ("authorAgent","demo-agent"); ex->setProperty ("createdAt","2026-05-11T00:00:00Z");
        ex->setProperty ("license","CC0-1.0");
        auto* cmp = new juce::DynamicObject();
        cmp->setProperty("minPackageSchema","cendancePackage.v1");
        cmp->setProperty("maxPackageSchema","cendancePackage.v1");
        ex->setProperty("compatibility", juce::var(cmp));
        auto* item = new juce::DynamicObject();
        item->setProperty("id","soft-wide-delay"); item->setProperty("name","Soft Wide Delay");
        item->setProperty("description","A custom delay setting.");
        item->setProperty("effectType","DelayEcho");
        item->setProperty("paramA",360.0); item->setProperty("paramB",0.34);
        item->setProperty("paramC",0.42);
        juce::Array<juce::var> tags; tags.add("delay"); tags.add("space");
        item->setProperty("tags", tags);
        juce::Array<juce::var> items; items.add(juce::var(item));
        ex->setProperty("items", items);
        obj->setProperty (kind.isEmpty() ? "effectPresetPackExample" : "example", juce::var(ex));
    }

    if (kind.isEmpty() || kind == "soundPresetPack") {
        auto* ex = new juce::DynamicObject();
        ex->setProperty("schema","cendancePackage.v1"); ex->setProperty("id","agent.demo.sounds");
        ex->setProperty("version","0.1.0"); ex->setProperty("kind","soundPresetPack");
        ex->setProperty("name","Agent Demo Sounds");
        ex->setProperty("description","Agent-authored sounds using existing engines and FX.");
        ex->setProperty("authorAgent","demo-agent"); ex->setProperty("createdAt","2026-05-11T00:00:00Z");
        ex->setProperty("license","CC0-1.0");
        auto* cmp = new juce::DynamicObject();
        cmp->setProperty("minPackageSchema","cendancePackage.v1");
        cmp->setProperty("maxPackageSchema","cendancePackage.v1");
        ex->setProperty("compatibility", juce::var(cmp));
        auto* item = new juce::DynamicObject();
        item->setProperty("id","gloss-bass"); item->setProperty("name","Gloss Bass");
        item->setProperty("description","A bright bass preset layered with ref-based FX.");
        item->setProperty("track",2); item->setProperty("soundRef","sound:builtin:...");
        juce::Array<juce::var> fx;
        auto* fxo = new juce::DynamicObject();
        fxo->setProperty("ref","effect:builtin:..."); fx.add(juce::var(fxo));
        item->setProperty("fx", fx);
        item->setProperty("density",0.55); item->setProperty("complexity",0.35);
        item->setProperty("tone",0.72); item->setProperty("motion",0.48);
        item->setProperty("gain",0.95);
        juce::Array<juce::var> stags;
        stags.add("bass"); stags.add("bright"); stags.add("agent-authored");
        item->setProperty("tags", stags);
        juce::Array<juce::var> items; items.add(juce::var(item));
        ex->setProperty("items", items);
        obj->setProperty (kind.isEmpty() ? "soundPresetPackExample" : "example", juce::var(ex));
    }

    return juce::var(obj);
}
