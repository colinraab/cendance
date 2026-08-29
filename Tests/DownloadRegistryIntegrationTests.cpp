#include "IntegrationTestHelpers.h"
#include "../Source/Network/P2PDownloadRegistry.h"
#include "../Source/Security/ContentHeader.h"

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

#define ASSERT_GE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (!(_a >= _b)) { \
        std::cerr << "  FAIL: " << #a << " >= " << #b << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
        return; \
    } \
} while(0)

#define PASS() do { testsPassed++; std::cout << "  PASS\n"; } while(0)

static P2PDownloadEntry makeEntry(const std::string& id, ContentType type) {
    P2PDownloadEntry entry;
    entry.preset_id = id;
    entry.sender_id = "test_sender";
    entry.timestamp = 1234567890;
    entry.verified = true;
    entry.local_path = "/tmp/" + id;
    entry.content_type = type;
    entry.display_name = "Test " + id;
    entry.format = "wav";
    entry.sample_rate = 44100;
    entry.channels = 2;
    entry.duration = 1.5;
    entry.sha256 = "abc123";
    return entry;
}

// ---- Test 1: Add entry -> save -> new registry loads -> entry matches ----
void testAddSaveLoadRoundTrip() {
    std::cout << "testAddSaveLoadRoundTrip...\n";
    TempDir tmpDir;

    // Create a registry and add an entry
    P2PDownloadRegistry registry;
    auto entry = makeEntry("preset_001", ContentType::Preset);
    registry.addEntry(entry);

    // Save
    ASSERT_TRUE(registry.save());

    // New registry loads
    P2PDownloadRegistry registry2;
    ASSERT_TRUE(registry2.load());

    auto entries = registry2.allEntries();
    ASSERT_GE(entries.size(), (size_t)1);

    bool found = false;
    for (auto& e : entries) {
        if (e.preset_id == "preset_001") {
            found = true;
            ASSERT_EQ(e.sender_id, "test_sender");
            ASSERT_EQ(e.timestamp, (uint64_t)1234567890);
            ASSERT_TRUE(e.verified);
            ASSERT_EQ(static_cast<int>(e.content_type), static_cast<int>(ContentType::Preset));
            break;
        }
    }
    ASSERT_TRUE(found);

    PASS();
}

// ---- Test 2: Multiple entries preserved through save/load ----
void testMultipleEntriesPreserved() {
    std::cout << "testMultipleEntriesPreserved...\n";

    P2PDownloadRegistry registry;
    registry.addEntry(makeEntry("preset_001", ContentType::Preset));
    registry.addEntry(makeEntry("sample_001", ContentType::Sample));
    registry.addEntry(makeEntry("algo_001", ContentType::Algorithm));

    ASSERT_TRUE(registry.save());

    P2PDownloadRegistry registry2;
    ASSERT_TRUE(registry2.load());

    auto entries = registry2.allEntries();
    ASSERT_GE(entries.size(), (size_t)3);

    int presetCount = 0, sampleCount = 0, algoCount = 0;
    for (auto& e : entries) {
        if (e.preset_id == "preset_001") presetCount++;
        if (e.preset_id == "sample_001") sampleCount++;
        if (e.preset_id == "algo_001") algoCount++;
    }
    ASSERT_EQ(presetCount, 1);
    ASSERT_EQ(sampleCount, 1);
    ASSERT_EQ(algoCount, 1);

    PASS();
}

// ---- Test 3: Content type filtering ----
void testContentTypeFiltering() {
    std::cout << "testContentTypeFiltering...\n";

    P2PDownloadRegistry registry;
    registry.addEntry(makeEntry("preset_001", ContentType::Preset));
    registry.addEntry(makeEntry("sample_001", ContentType::Sample));

    auto entries = registry.allEntries();
    ASSERT_GE(entries.size(), (size_t)2);

    // Verify we can distinguish by content type
    for (auto& e : entries) {
        if (e.preset_id == "preset_001") {
            ASSERT_EQ(static_cast<int>(e.content_type), static_cast<int>(ContentType::Preset));
        }
        if (e.preset_id == "sample_001") {
            ASSERT_EQ(static_cast<int>(e.content_type), static_cast<int>(ContentType::Sample));
        }
    }

    PASS();
}

// ---- Test 4: Registry file is valid JSON ----
void testRegistryFileIsValidJson() {
    std::cout << "testRegistryFileIsValidJson...\n";

    P2PDownloadRegistry registry;
    registry.addEntry(makeEntry("test_001", ContentType::Preset));
    ASSERT_TRUE(registry.save());

    auto file = registry.registryFilePath();
    ASSERT_TRUE(file.existsAsFile());

    juce::String content = file.loadFileAsString();
    ASSERT_FALSE(content.isEmpty());

    auto parsed = juce::JSON::parse(content);
    ASSERT_TRUE(parsed.isObject());

    auto* downloads = varGet(parsed, "downloads").getArray();
    ASSERT_TRUE(downloads != nullptr);
    ASSERT_GE(downloads->size(), (size_t)1);

    PASS();
}

// ---- Test 5: Empty registry save/load ----
void testEmptyRegistrySaveLoad() {
    std::cout << "testEmptyRegistrySaveLoad...\n";

    P2PDownloadRegistry registry;
    ASSERT_TRUE(registry.save());

    P2PDownloadRegistry registry2;
    ASSERT_TRUE(registry2.load());

    auto entries = registry2.allEntries();
    ASSERT_EQ(entries.size(), (size_t)0);

    PASS();
}

int main() {
    std::cout << "=== Download Registry Integration Tests ===\n\n";

    testAddSaveLoadRoundTrip();
    testMultipleEntriesPreserved();
    testContentTypeFiltering();
    testRegistryFileIsValidJson();
    testEmptyRegistrySaveLoad();

    std::cout << "\n=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed > 0 ? 1 : 0;
}
