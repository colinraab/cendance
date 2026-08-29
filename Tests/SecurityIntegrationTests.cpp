#include "IntegrationTestHelpers.h"
#include "../Source/Security/SecurityManager.h"
#include "../Source/Security/PresetSerializer.h"
#include "../Source/Security/ContentHeader.h"
#include "../Source/Config/ToSGuard.h"

#include <cassert>
#include <iostream>

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "  FAIL: " << #expr << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(expr) do { \
    if ((expr)) { \
        std::cerr << "  FAIL: expected false: " << #expr << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a != _b) { \
        std::cerr << "  FAIL: " << #a << " == " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a == _b) { \
        std::cerr << "  FAIL: " << #a << " != " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define PASS() do { testsPassed++; std::cout << "  PASS\n"; } while(0)

// ---- Test 1: Two-keypair sign/verify ----
void testTwoKeypairSignVerify() {
    std::cout << "testTwoKeypairSignVerify...\n";
    TempDir tmpDir;
    SecurityManager sm1;
    ASSERT_TRUE(sm1.initialize());

    // Delete the key file so sm2 generates a different keypair
    auto keyFile = sm1.keyFilePath();
    if (keyFile.existsAsFile()) keyFile.deleteFile();

    SecurityManager sm2;
    ASSERT_TRUE(sm2.initialize());

    // Verify they have different keypairs
    ASSERT_NE(sm1.publicKeyHex(), sm2.publicKeyHex());

    std::vector<uint8_t> data = {'h', 'e', 'l', 'l', 'o'};

    auto sigFuture = sm1.sign(data);
    std::string sig = sigFuture.get();
    ASSERT_FALSE(sig.empty());

    // Same key verifies
    auto verifyFuture = sm1.verify(sm1.publicKeyHex(), data, sig);
    ASSERT_TRUE(verifyFuture.get());

    // Different key (sm2's public key) fails to verify sm1's signature
    auto verifyFuture2 = sm1.verify(sm2.publicKeyHex(), data, sig);
    ASSERT_FALSE(verifyFuture2.get());

    PASS();
}

// ---- Test 2: Cross-keypair verification fails ----
void testCrossKeypairVerificationFails() {
    std::cout << "testCrossKeypairVerificationFails...\n";
    SecurityManager sm1;
    ASSERT_TRUE(sm1.initialize());

    // Delete the key file so sm2 generates a different keypair
    auto keyFile = sm1.keyFilePath();
    if (keyFile.existsAsFile()) keyFile.deleteFile();

    SecurityManager sm2;
    ASSERT_TRUE(sm2.initialize());

    ASSERT_NE(sm1.publicKeyHex(), sm2.publicKeyHex());

    std::vector<uint8_t> data = {'t', 'e', 's', 't'};
    auto sig = sm1.sign(data).get();
    ASSERT_FALSE(sig.empty());

    // Using sm2's public key to verify sm1's signature should fail
    auto result = sm1.verify(sm2.publicKeyHex(), data, sig);
    ASSERT_FALSE(result.get());

    PASS();
}

// ---- Test 3: Key persistence ----
void testKeyPersistence() {
    std::cout << "testKeyPersistence...\n";
    TempDir tmpDir;

    std::string pk1;
    {
        SecurityManager sm;
        ASSERT_TRUE(sm.initialize());
        pk1 = sm.publicKeyHex();
        ASSERT_FALSE(pk1.empty());
        // Key file is saved on initialize
    }

    // New instance should load the same key
    {
        SecurityManager sm;
        ASSERT_TRUE(sm.initialize());
        ASSERT_EQ(sm.publicKeyHex(), pk1);
    }

    PASS();
}

// ---- Test 4: Key file format ----
void testKeyFileFormat() {
    std::cout << "testKeyFileFormat...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    auto keyFile = sm.keyFilePath();
    ASSERT_TRUE(keyFile.existsAsFile());

    juce::String content = keyFile.loadFileAsString();
    ASSERT_FALSE(content.isEmpty());

    auto parsed = juce::JSON::parse(content);
    ASSERT_TRUE(parsed.isObject());

    ASSERT_TRUE(varHasProperty(parsed, "public_key"));
    ASSERT_TRUE(varHasProperty(parsed, "secret_key"));

    PASS();
}

// ---- Test 5: Replay protection ----
void testReplayProtection() {
    std::cout << "testReplayProtection...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    // Create a valid envelope manually with an old timestamp
    std::string payload = R"({"test":"data"})";
    std::vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);

    auto sigFuture = sm.sign(payloadBytes);
    std::string sigStr = sigFuture.get();
    ASSERT_FALSE(sigStr.empty());

    // Build envelope with timestamp 6 minutes ago
    uint64_t oldTimestamp = static_cast<uint64_t>(
        juce::Time::getCurrentTime().toMilliseconds()) - 6 * 60 * 1000;

    auto header = std::make_unique<juce::DynamicObject>();
    header->setProperty("sender_id", juce::String(sm.publicKeyHex()));
    header->setProperty("timestamp", static_cast<int64_t>(oldTimestamp));
    header->setProperty("content_hash", juce::String(hashHex));
    header->setProperty("content_type", static_cast<int>(ContentType::Preset));
    header->setProperty("signature", juce::String(sigStr));
    std::string headerJson = juce::JSON::toString(juce::var(header.release()), false).toStdString();

    auto envelope = std::make_unique<juce::DynamicObject>();
    envelope->setProperty("content_type", static_cast<int>(ContentType::Preset));
    envelope->setProperty("header", juce::String(headerJson));
    envelope->setProperty("payload", juce::String(payload));
    std::string envelopeJson = juce::JSON::toString(juce::var(envelope.release()), false).toStdString();

    // Verify should reject due to old timestamp
    PresetSerializer serializer;
    auto result = serializer.verifyAndLoad(envelopeJson, sm).get();
    ASSERT_FALSE(result.ok);
    ASSERT_EQ(static_cast<int>(result.trustLevel), static_cast<int>(TrustLevel::Tampered));
    ASSERT_TRUE(result.error.find("timestamp") != std::string::npos ||
                result.error.find("replay") != std::string::npos);

    PASS();
}

// ---- Test 6: Future timestamp rejection ----
void testFutureTimestampRejection() {
    std::cout << "testFutureTimestampRejection...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    std::string payload = R"({"test":"future"})";
    std::vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);
    auto sig = sm.sign(payloadBytes).get();

    // Build envelope with timestamp 1 hour in the future
    uint64_t futureTimestamp = static_cast<uint64_t>(
        juce::Time::getCurrentTime().toMilliseconds()) + 3600 * 1000;

    auto header = std::make_unique<juce::DynamicObject>();
    header->setProperty("sender_id", juce::String(sm.publicKeyHex()));
    header->setProperty("timestamp", static_cast<int64_t>(futureTimestamp));
    header->setProperty("content_hash", juce::String(hashHex));
    header->setProperty("content_type", static_cast<int>(ContentType::Preset));
    header->setProperty("signature", juce::String(sig));
    std::string headerJson = juce::JSON::toString(juce::var(header.release()), false).toStdString();

    auto envelope = std::make_unique<juce::DynamicObject>();
    envelope->setProperty("content_type", static_cast<int>(ContentType::Preset));
    envelope->setProperty("header", juce::String(headerJson));
    envelope->setProperty("payload", juce::String(payload));
    std::string envelopeJson = juce::JSON::toString(juce::var(envelope.release()), false).toStdString();

    PresetSerializer serializer;
    auto result = serializer.verifyAndLoad(envelopeJson, sm).get();
    ASSERT_FALSE(result.ok);

    PASS();
}

// ---- Test 7: Fresh envelope acceptance ----
void testFreshEnvelopeAcceptance() {
    std::cout << "testFreshEnvelopeAcceptance...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    std::string payload = R"({"test":"fresh"})";
    std::vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    std::string hashHex = SecurityManager::sha256Hex(payloadBytes);
    auto sig = sm.sign(payloadBytes).get();

    uint64_t nowTimestamp = static_cast<uint64_t>(
        juce::Time::getCurrentTime().toMilliseconds());

    auto header = std::make_unique<juce::DynamicObject>();
    header->setProperty("sender_id", juce::String(sm.publicKeyHex()));
    header->setProperty("timestamp", static_cast<int64_t>(nowTimestamp));
    header->setProperty("content_hash", juce::String(hashHex));
    header->setProperty("content_type", static_cast<int>(ContentType::Preset));
    header->setProperty("signature", juce::String(sig));
    std::string headerJson = juce::JSON::toString(juce::var(header.release()), false).toStdString();

    auto envelope = std::make_unique<juce::DynamicObject>();
    envelope->setProperty("content_type", static_cast<int>(ContentType::Preset));
    envelope->setProperty("header", juce::String(headerJson));
    envelope->setProperty("payload", juce::String(payload));
    std::string envelopeJson = juce::JSON::toString(juce::var(envelope.release()), false).toStdString();

    PresetSerializer serializer;
    auto result = serializer.verifyAndLoad(envelopeJson, sm).get();
    ASSERT_TRUE(result.ok);
    ASSERT_EQ(static_cast<int>(result.trustLevel), static_cast<int>(TrustLevel::Verified));

    PASS();
}

// ---- Test 8: SHA-256 determinism ----
void testSha256Determinism() {
    std::cout << "testSha256Determinism...\n";
    std::vector<uint8_t> data1 = {'a', 'b', 'c'};
    std::vector<uint8_t> data2 = {'a', 'b', 'c'};
    std::vector<uint8_t> data3 = {'x', 'y', 'z'};

    ASSERT_EQ(SecurityManager::sha256Hex(data1), SecurityManager::sha256Hex(data2));
    ASSERT_NE(SecurityManager::sha256Hex(data1), SecurityManager::sha256Hex(data3));

    PASS();
}

// ---- Test 9: Signature uniqueness (deterministic Ed25519) ----
void testSignatureDeterministic() {
    std::cout << "testSignatureDeterministic...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    std::vector<uint8_t> data = {'d', 'e', 't', 'e', 'r', 'm', 'i', 'n', 'i', 's', 't', 'i', 'c'};
    auto sig1 = sm.sign(data).get();
    auto sig2 = sm.sign(data).get();

    // Ed25519 is deterministic — same data + same key = same signature
    ASSERT_EQ(sig1, sig2);

    PASS();
}

// ---- Test 10: Empty data signing ----
void testEmptyDataSigning() {
    std::cout << "testEmptyDataSigning...\n";
    SecurityManager sm;
    ASSERT_TRUE(sm.initialize());

    std::vector<uint8_t> emptyData;
    auto sig = sm.sign(emptyData).get();
    ASSERT_FALSE(sig.empty());

    // Verify the empty-data signature
    auto verifyResult = sm.verify(sm.publicKeyHex(), emptyData, sig);
    ASSERT_TRUE(verifyResult.get());

    PASS();
}

int main() {
    std::cout << "=== Security Integration Tests ===\n\n";

    testTwoKeypairSignVerify();
    testCrossKeypairVerificationFails();
    testKeyPersistence();
    testKeyFileFormat();
    testReplayProtection();
    testFutureTimestampRejection();
    testFreshEnvelopeAcceptance();
    testSha256Determinism();
    testSignatureDeterministic();
    testEmptyDataSigning();

    std::cout << "\n=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed > 0 ? 1 : 0;
}
