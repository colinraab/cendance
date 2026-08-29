#include "ToSGuard.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace ToSGuard {
namespace {

constexpr const char* kConfigFileName = "config.json";
constexpr const char* kConfigDirName = "cendance";

bool ensureConfigDirectory() {
    auto dir = configDirectory();
    if (!dir.exists())
        return dir.createDirectory().wasOk();
    return dir.isDirectory();
}

juce::var loadConfig() {
    auto file = configFile();
    if (!file.existsAsFile())
        return {};

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
        return {};

    auto parsed = juce::JSON::parse(content);
    if (parsed.isVoid() || !parsed.isObject())
        return {};

    return parsed;
}

bool hasAcceptedProperty(const juce::var& parsed) {
    auto accepted = parsed.getProperty("tos_accepted", {});
    return accepted.isBool() && static_cast<bool>(accepted);
}

} // namespace

juce::File configDirectory() {
    if (const char* overrideDir = std::getenv("CENDANCE_CONFIG_DIR");
        overrideDir != nullptr && overrideDir[0] != '\0') {
        return juce::File(juce::String(overrideDir));
    }

    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(juce::String(kConfigDirName));
}

juce::File configFile() {
    return configDirectory().getChildFile(juce::String(kConfigFileName));
}

bool isAccepted() {
    return hasAcceptedProperty(loadConfig());
}

std::string acceptedTimestamp() {
    auto parsed = loadConfig();
    if (!hasAcceptedProperty(parsed))
        return {};

    juce::String ts = parsed.getProperty("tos_accepted_at", juce::String());
    return ts.toStdString();
}

bool accept() {
    if (!ensureConfigDirectory())
        return false;

    auto file = configFile();

    // Load existing config if present so we don't wipe other fields
    auto root = std::make_unique<juce::DynamicObject>();

    auto parsed = loadConfig();
    if (auto* obj = parsed.getDynamicObject()) {
        for (auto& prop : obj->getProperties()) {
            root->setProperty(prop.name, prop.value);
        }
    }

    root->setProperty("tos_accepted", true);
    root->setProperty("tos_accepted_at",
                      juce::Time::getCurrentTime().toISO8601(true));

    juce::String json = juce::JSON::toString(juce::var(root.release()), true);

    std::ofstream out(file.getFullPathName().toStdString(),
                      std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        return false;

    out << json.toStdString();
    out.close();
    return out.good() && isAccepted();
}

std::string tosText() {
    return R"(cendance Terms of Service

By using cendance's peer-to-peer sharing features for presets, samples,
sample packs, and embedded custom sound presets, you agree
to the following terms:

1. WARRANTY DISCLAIMER
   cendance is provided "as is" without warranty of any kind. The
   authors are not liable for any hardware damage, hearing damage,
   or other losses arising from use of this software. Use at your
   own risk. Keep volume at safe levels.

2. COPYRIGHT & INDEMNIFICATION
   You are responsible for ensuring that any presets, audio files,
   loops, one-shots, embedded samples, and sample packs you upload
   or share do not infringe third-party copyrights, licenses, or
   publicity rights. Share only content you created, own, or are
   explicitly licensed to redistribute. You agree to indemnify and
   hold harmless the cendance authors from any claims arising from
   content you share.

3. DMCA POLICY
   If you believe shared content infringes your copyright, please
   follow the DMCA takedown process at:
   https://www.dmca.com/Complaint/Process

4. P2P NETWORK USE
   Peer-to-peer features transmit preset and audio data between users.
   You understand that shared presets, sample files, loops, one-shots,
   and sample packs may be copied to peers' devices and retained in
   their local downloads. No personal data beyond your public key and
   chosen display name is transmitted.

Type "I AGREE" (without quotes) to accept these terms and continue.
Press Escape to decline and exit.)";
}

} // namespace ToSGuard
