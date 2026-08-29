#pragma once

#include "ContentHeader.h"

#include <juce_core/juce_core.h>

#include <cstdint>
#include <future>
#include <string>
#include <vector>

// SecurityManager handles Ed25519 keypair generation, persistence,
// signing, and verification using libsodium.
//
// All crypto operations are designed to be called off the audio thread.
// The sign() and verify() methods return std::future so callers can
// dispatch them to worker threads.
class SecurityManager {
public:
    SecurityManager();
    ~SecurityManager();

    // Initialize: load or generate keypair. Must be called before sign/verify.
    // Returns true on success.
    bool initialize();

    // Returns true if a keypair has been loaded/generated.
    bool hasKeyPair() const;

    // Returns the public key as hex string (for sharing with peers).
    std::string publicKeyHex() const;

    // Returns the public key hex used as sender_id for signature verification.
    std::string senderId() const;

    // Sign data asynchronously. Returns a future<signature_hex_string>.
    // On failure, the future contains an empty string.
    std::future<std::string> sign(const std::vector<uint8_t>& data);

    // Verify asynchronously. Returns a future<bool>.
    // sender_pubkey_hex: the sender's public key in hex
    // data: the original payload bytes
    // signature_hex: the Ed25519 signature in hex
    std::future<bool> verify(const std::string& sender_pubkey_hex,
                             const std::vector<uint8_t>& data,
                             const std::string& signature_hex);

    // Compute SHA-256 hash of data, return as hex string.
    static std::string sha256Hex(const std::vector<uint8_t>& data);

    // Compute SHA-256 hash of a string, return as hex string.
    static std::string sha256Hex(const std::string& text);

    // Convert binary bytes to hex string.
    static std::string toHex(const uint8_t* data, size_t len);

    // Convert hex string to binary bytes.
    static std::vector<uint8_t> fromHex(const std::string& hex);

    // Get the path where the keypair is stored.
    juce::File keyFilePath() const;

private:
    bool loadOrGenerateKeypair();

    std::vector<uint8_t> publicKey_;
    std::vector<uint8_t> secretKey_;
    bool initialized_ = false;
};
