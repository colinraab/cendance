#include "SecurityManager.h"
#include "../Config/AppDirectories.h"

#include <juce_core/juce_core.h>

#include <sodium.h>

#include <fstream>
#include <sstream>
#include <cstring>

SecurityManager::SecurityManager() {
    if (sodium_init() < 0) {
        // libsodium failed to initialize — mark as not initialized
        initialized_ = false;
    }
}

SecurityManager::~SecurityManager() {
    // Securely wipe secret key memory
    if (!secretKey_.empty()) {
        sodium_memzero(secretKey_.data(), secretKey_.size());
    }
}

bool SecurityManager::initialize() {
    if (initialized_)
        return true;

    if (sodium_init() < 0)
        return false;

    initialized_ = loadOrGenerateKeypair();
    return initialized_;
}

bool SecurityManager::hasKeyPair() const {
    return initialized_ && !publicKey_.empty() && !secretKey_.empty();
}

std::string SecurityManager::publicKeyHex() const {
    if (publicKey_.empty()) return {};
    return toHex(publicKey_.data(), publicKey_.size());
}

std::string SecurityManager::senderId() const {
    return publicKeyHex();
}

std::future<std::string> SecurityManager::sign(const std::vector<uint8_t>& data) {
    return std::async(std::launch::async, [this, data]() -> std::string {
        if (!hasKeyPair())
            return {};

        std::vector<unsigned char> sig(crypto_sign_BYTES);
        unsigned long long sigLen = 0;

        if (crypto_sign_detached(sig.data(), &sigLen,
                                 data.data(), data.size(),
                                 secretKey_.data()) != 0)
            return {};

        return toHex(sig.data(), static_cast<size_t>(sigLen));
    });
}

std::future<bool> SecurityManager::verify(const std::string& sender_pubkey_hex,
                                           const std::vector<uint8_t>& data,
                                           const std::string& signature_hex) {
    return std::async(std::launch::async, [sender_pubkey_hex, data, signature_hex]() -> bool {
        if (sodium_init() < 0)
            return false;

        auto pk = fromHex(sender_pubkey_hex);
        auto sig = fromHex(signature_hex);

        if (pk.size() != crypto_sign_PUBLICKEYBYTES)
            return false;
        if (sig.size() != crypto_sign_BYTES)
            return false;

        return crypto_sign_verify_detached(sig.data(),
                                           data.data(), data.size(),
                                           pk.data()) == 0;
    });
}

std::string SecurityManager::sha256Hex(const std::vector<uint8_t>& data) {
    if (sodium_init() < 0) return {};

    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash, data.data(), data.size());
    return toHex(hash, crypto_hash_sha256_BYTES);
}

std::string SecurityManager::sha256Hex(const std::string& text) {
    std::vector<uint8_t> data(text.begin(), text.end());
    return sha256Hex(data);
}

std::string SecurityManager::toHex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    return oss.str();
}

std::vector<uint8_t> SecurityManager::fromHex(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        uint8_t byte = 0;
        try {
            byte = static_cast<uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16));
        } catch (...) {
            byte = 0;
        }
        out.push_back(byte);
    }
    return out;
}

juce::File SecurityManager::keyFilePath() const {
    return AppDirectories::dataDirectory().getChildFile("identity.json");
}

bool SecurityManager::loadOrGenerateKeypair() {
    auto file = keyFilePath();

    if (file.existsAsFile()) {
        juce::String content = file.loadFileAsString();
        if (!content.isEmpty()) {
            auto parsed = juce::JSON::parse(content);
            if (parsed.isObject()) {
                auto* obj = parsed.getDynamicObject();
                juce::String pkHex = obj->getProperty("public_key");
                juce::String skHex = obj->getProperty("secret_key");
                if (!pkHex.isEmpty() && !skHex.isEmpty()) {
                    publicKey_ = fromHex(pkHex.toStdString());
                    secretKey_ = fromHex(skHex.toStdString());
                    if (publicKey_.size() == crypto_sign_PUBLICKEYBYTES &&
                        secretKey_.size() == crypto_sign_SECRETKEYBYTES) {
                        return true;
                    }
                }
            }
        }
    }

    // Generate new keypair
    publicKey_.resize(crypto_sign_PUBLICKEYBYTES);
    secretKey_.resize(crypto_sign_SECRETKEYBYTES);

    if (crypto_sign_keypair(publicKey_.data(), secretKey_.data()) != 0)
        return false;

    // Persist to disk
    auto dir = file.getParentDirectory();
    if (!dir.exists())
        dir.createDirectory();

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("public_key", juce::String(toHex(publicKey_.data(), publicKey_.size())));
    root->setProperty("secret_key", juce::String(toHex(secretKey_.data(), secretKey_.size())));

    juce::String json = juce::JSON::toString(juce::var(root.release()), true);

    std::ofstream out(file.getFullPathName().toStdString(),
                      std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        return false;

    out << json.toStdString();
    out.close();

    return true;
}
