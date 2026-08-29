#include "PresetRef.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace PresetRefs {
namespace {

std::string lowerCopy(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string jsonEscape(const std::string& text) {
    std::ostringstream out;
    for (const char ch : text) {
        switch (ch) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                out << "\\u"
                    << "0000";
            } else {
                out << ch;
            }
            break;
        }
    }
    return out.str();
}

std::string quoted(const std::string& text) {
    return "\"" + jsonEscape(text) + "\"";
}

} // namespace

bool PresetRef::operator==(const PresetRef& other) const {
    const bool versionsMatch = packageVersion.empty()
        || other.packageVersion.empty()
        || packageVersion == other.packageVersion;
    return domain == other.domain
        && source == other.source
        && builtinId == other.builtinId
        && packageId == other.packageId
        && itemId == other.itemId
        && versionsMatch;
}

std::string domainToString(Domain domain) {
    switch (domain) {
    case Domain::Effect: return "effect";
    case Domain::Sound: return "sound";
    case Domain::DrumKit: return "drumKit";
    case Domain::Arrangement: return "arrangement";
    case Domain::Scene: return "scene";
    case Domain::Sample: return "sample";
    case Domain::MidiGenerator: return "midiGenerator";
    case Domain::InstrumentEngine: return "instrumentEngine";
    }
    return "sound";
}

std::optional<Domain> domainFromString(const std::string& text) {
    const std::string value = lowerCopy(text);
    if (value == "effect") return Domain::Effect;
    if (value == "sound") return Domain::Sound;
    if (value == "drumkit" || value == "drum-kit") return Domain::DrumKit;
    if (value == "arrangement") return Domain::Arrangement;
    if (value == "scene") return Domain::Scene;
    if (value == "sample") return Domain::Sample;
    if (value == "midigenerator" || value == "midi-generator") return Domain::MidiGenerator;
    if (value == "instrumentengine" || value == "instrument-engine") return Domain::InstrumentEngine;
    return std::nullopt;
}

std::string sourceToString(Source source) {
    return source == Source::Builtin ? "builtin" : "package";
}

std::optional<Source> sourceFromString(const std::string& text) {
    const std::string value = lowerCopy(text);
    if (value == "builtin") return Source::Builtin;
    if (value == "package") return Source::Package;
    return std::nullopt;
}

PresetRef builtin(Domain domain, const std::string& builtinId) {
    PresetRef ref;
    ref.domain = domain;
    ref.source = Source::Builtin;
    ref.builtinId = builtinId;
    return ref;
}

PresetRef package(Domain domain,
                  const std::string& packageId,
                  const std::string& itemId,
                  const std::string& packageVersion) {
    PresetRef ref;
    ref.domain = domain;
    ref.source = Source::Package;
    ref.packageId = packageId;
    ref.itemId = itemId;
    ref.packageVersion = packageVersion;
    return ref;
}

bool isValid(const PresetRef& ref) {
    if (ref.source == Source::Builtin) {
        return !ref.builtinId.empty();
    }
    return !ref.packageId.empty() && !ref.itemId.empty();
}

std::string toStableString(const PresetRef& ref) {
    std::ostringstream out;
    out << domainToString(ref.domain) << ":";
    if (ref.source == Source::Builtin) {
        out << "builtin:" << ref.builtinId;
    } else {
        out << "package:" << ref.packageId << ":" << ref.itemId;
        if (!ref.packageVersion.empty()) {
            out << "@" << ref.packageVersion;
        }
    }
    return out.str();
}

std::optional<PresetRef> parseStableString(const std::string& text) {
    const auto first = text.find(':');
    const auto second = first == std::string::npos ? std::string::npos : text.find(':', first + 1);
    if (first == std::string::npos || second == std::string::npos) {
        return std::nullopt;
    }
    auto domain = domainFromString(text.substr(0, first));
    auto source = sourceFromString(text.substr(first + 1, second - first - 1));
    if (!domain.has_value() || !source.has_value()) {
        return std::nullopt;
    }
    if (*source == Source::Builtin) {
        auto ref = builtin(*domain, text.substr(second + 1));
        return isValid(ref) ? std::optional<PresetRef>(ref) : std::nullopt;
    }
    const auto third = text.find(':', second + 1);
    if (third == std::string::npos) {
        return std::nullopt;
    }
    std::string item = text.substr(third + 1);
    std::string version;
    const auto at = item.find('@');
    if (at != std::string::npos) {
        version = item.substr(at + 1);
        item = item.substr(0, at);
    }
    auto ref = package(*domain, text.substr(second + 1, third - second - 1), item, version);
    return isValid(ref) ? std::optional<PresetRef>(ref) : std::nullopt;
}

std::string toJson(const PresetRef& ref) {
    std::ostringstream out;
    out << "{\"domain\":" << quoted(domainToString(ref.domain))
        << ",\"source\":" << quoted(sourceToString(ref.source));
    if (ref.source == Source::Builtin) {
        out << ",\"builtinId\":" << quoted(ref.builtinId);
    } else {
        out << ",\"packageId\":" << quoted(ref.packageId)
            << ",\"itemId\":" << quoted(ref.itemId);
        if (!ref.packageVersion.empty()) {
            out << ",\"packageVersion\":" << quoted(ref.packageVersion);
        }
    }
    out << ",\"stable\":" << quoted(toStableString(ref)) << "}";
    return out.str();
}

std::optional<PresetRef> fromJsonObject(const void* dynamicObject) {
    auto* object = static_cast<const juce::DynamicObject*>(dynamicObject);
    if (object == nullptr) {
        return std::nullopt;
    }
    const std::string stable = object->getProperty("stable").toString().toStdString();
    if (!stable.empty()) {
        return parseStableString(stable);
    }
    auto domain = domainFromString(object->getProperty("domain").toString().toStdString());
    auto source = sourceFromString(object->getProperty("source").toString().toStdString());
    if (!domain.has_value() || !source.has_value()) {
        return std::nullopt;
    }
    if (*source == Source::Builtin) {
        auto ref = builtin(*domain, object->getProperty("builtinId").toString().toStdString());
        return isValid(ref) ? std::optional<PresetRef>(ref) : std::nullopt;
    }
    auto ref = package(*domain,
                       object->getProperty("packageId").toString().toStdString(),
                       object->getProperty("itemId").toString().toStdString(),
                       object->getProperty("packageVersion").toString().toStdString());
    return isValid(ref) ? std::optional<PresetRef>(ref) : std::nullopt;
}

std::string slugify(const std::string& text) {
    std::string out;
    bool lastDash = false;
    for (const char ch : text) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch)) {
            out.push_back(static_cast<char>(std::tolower(uch)));
            lastDash = false;
        } else if (!lastDash && !out.empty()) {
            out.push_back('-');
            lastDash = true;
        }
    }
    while (!out.empty() && out.back() == '-') {
        out.pop_back();
    }
    return out.empty() ? "unnamed" : out;
}

} // namespace PresetRefs
