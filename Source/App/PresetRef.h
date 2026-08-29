#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace PresetRefs {

enum class Domain : uint8_t {
    Effect = 0,
    Sound,
    DrumKit,
    Arrangement,
    Scene,
    Sample,
    MidiGenerator,
    InstrumentEngine,
};

enum class Source : uint8_t {
    Builtin = 0,
    Package,
};

struct PresetRef {
    Domain domain = Domain::Sound;
    Source source = Source::Builtin;
    std::string builtinId;
    std::string packageId;
    std::string itemId;
    std::string packageVersion;

    bool operator==(const PresetRef& other) const;
    bool operator!=(const PresetRef& other) const { return !(*this == other); }
};

std::string domainToString(Domain domain);
std::optional<Domain> domainFromString(const std::string& text);
std::string sourceToString(Source source);
std::optional<Source> sourceFromString(const std::string& text);

PresetRef builtin(Domain domain, const std::string& builtinId);
PresetRef package(Domain domain,
                  const std::string& packageId,
                  const std::string& itemId,
                  const std::string& packageVersion = "");

bool isValid(const PresetRef& ref);
std::string toStableString(const PresetRef& ref);
std::optional<PresetRef> parseStableString(const std::string& text);
std::string toJson(const PresetRef& ref);
std::optional<PresetRef> fromJsonObject(const void* dynamicObject);
std::string slugify(const std::string& text);

} // namespace PresetRefs

