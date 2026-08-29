#pragma once

#include <cstdint>
#include <string>

enum class ContentType : uint8_t {
    Preset = 0,
    Sample = 1,
    Algorithm = 2,
    Project = 3,
};

// Fixed-size header attached to every signed payload.
struct ContentHeader {
    std::string sender_id;       // public key hex string
    uint64_t timestamp = 0;      // unix epoch millis
    std::string content_hash;    // SHA-256 hex string of the payload bytes
    std::string signature;       // Ed25519 signature hex (64 bytes = 128 hex chars)
    ContentType content_type = ContentType::Preset;

    // Serializes to JSON string for embedding in transport envelopes.
    std::string toJson() const;

    // Parses from JSON object. Returns true on success.
    static bool fromJson(const std::string& json, ContentHeader& out, std::string& error);
};
