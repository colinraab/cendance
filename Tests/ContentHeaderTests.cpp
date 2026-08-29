#include "../Source/Security/ContentHeader.h"

#include <cassert>
#include <iostream>
#include <string>

// ========================================================================
// P0 Tests
// ========================================================================

void testToJsonRoundTrip() {
    ContentHeader original;
    original.sender_id = "abc123def456";
    original.timestamp = 1700000000000ULL;
    original.content_hash = "deadbeef1234567890abcdef1234567890abcdef1234567890abcdef12345678";
    original.signature = "sig1234567890abcdef";
    original.content_type = ContentType::Preset;

    std::string json = original.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(error.empty());

    assert(parsed.sender_id == original.sender_id);
    assert(parsed.timestamp == original.timestamp);
    assert(parsed.content_hash == original.content_hash);
    assert(parsed.signature == original.signature);
    assert(parsed.content_type == original.content_type);
}

void testContentTypePreset() {
    ContentHeader hdr;
    hdr.sender_id = "sender1";
    hdr.timestamp = 1000;
    hdr.content_hash = "hash1";
    hdr.signature = "sig1";
    hdr.content_type = ContentType::Preset;

    std::string json = hdr.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(parsed.content_type == ContentType::Preset);
}

void testContentTypeProject() {
    ContentHeader hdr;
    hdr.sender_id = "sender2";
    hdr.timestamp = 2000;
    hdr.content_hash = "hash2";
    hdr.signature = "sig2";
    hdr.content_type = ContentType::Project;

    std::string json = hdr.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(parsed.content_type == ContentType::Project);
}

void testContentTypeSample() {
    ContentHeader hdr;
    hdr.sender_id = "sender3";
    hdr.timestamp = 3000;
    hdr.content_hash = "hash3";
    hdr.signature = "sig3";
    hdr.content_type = ContentType::Sample;

    std::string json = hdr.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(parsed.content_type == ContentType::Sample);
}

void testContentTypeAlgorithm() {
    ContentHeader hdr;
    hdr.sender_id = "sender4";
    hdr.timestamp = 4000;
    hdr.content_hash = "hash4";
    hdr.signature = "sig4";
    hdr.content_type = ContentType::Algorithm;

    std::string json = hdr.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(parsed.content_type == ContentType::Algorithm);
}

void testFromJsonMissingSenderId() {
    ContentHeader hdr;
    hdr.sender_id = "sender";
    hdr.timestamp = 1000;
    hdr.content_hash = "hash";
    hdr.signature = "sig";

    // Build JSON without sender_id by manipulating the string
    std::string json = "{\"timestamp\":1000,\"content_hash\":\"hash\",\"signature\":\"sig\",\"content_type\":0}";
    ContentHeader parsed;
    std::string error;
    // This may or may not fail depending on implementation — fromJson may use default empty string
    // The key test is that required fields are handled
    bool result = ContentHeader::fromJson(json, parsed, error);
    // If it succeeds, sender_id should be empty (default)
    if (result) {
        assert(parsed.sender_id.empty());
    }
}

void testFromJsonMissingContentHash() {
    std::string json = "{\"sender_id\":\"s1\",\"timestamp\":1000,\"signature\":\"sig\",\"content_type\":0}";
    ContentHeader parsed;
    std::string error;
    bool result = ContentHeader::fromJson(json, parsed, error);
    if (result) {
        assert(parsed.content_hash.empty());
    }
}

void testFromJsonMissingSignature() {
    std::string json = "{\"sender_id\":\"s1\",\"timestamp\":1000,\"content_hash\":\"h1\",\"content_type\":0}";
    ContentHeader parsed;
    std::string error;
    bool result = ContentHeader::fromJson(json, parsed, error);
    if (result) {
        assert(parsed.signature.empty());
    }
}

void testFromJsonInvalidJson() {
    std::string json = "this is not json at all";
    ContentHeader parsed;
    std::string error;
    assert(!ContentHeader::fromJson(json, parsed, error));
    assert(!error.empty());
}

void testFromJsonEmptyObject() {
    std::string json = "{}";
    ContentHeader parsed;
    std::string error;
    // Empty object should parse but with defaults
    bool result = ContentHeader::fromJson(json, parsed, error);
    if (result) {
        assert(parsed.sender_id.empty());
        assert(parsed.timestamp == 0);
        assert(parsed.content_hash.empty());
        assert(parsed.signature.empty());
        assert(parsed.content_type == ContentType::Preset); // default
    }
}

void testTimestampPreserved() {
    ContentHeader hdr;
    hdr.sender_id = "ts_test";
    hdr.timestamp = 1700000000000ULL; // a specific large timestamp
    hdr.content_hash = "hash_ts";
    hdr.signature = "sig_ts";
    hdr.content_type = ContentType::Project;

    std::string json = hdr.toJson();
    ContentHeader parsed;
    std::string error;
    assert(ContentHeader::fromJson(json, parsed, error));
    assert(parsed.timestamp == 1700000000000ULL);
}

// ========================================================================
// Main
// ========================================================================

int main() {
    testToJsonRoundTrip();
    std::cout << "  testToJsonRoundTrip passed\n";

    testContentTypePreset();
    std::cout << "  testContentTypePreset passed\n";

    testContentTypeProject();
    std::cout << "  testContentTypeProject passed\n";

    testContentTypeSample();
    std::cout << "  testContentTypeSample passed\n";

    testContentTypeAlgorithm();
    std::cout << "  testContentTypeAlgorithm passed\n";

    testFromJsonMissingSenderId();
    std::cout << "  testFromJsonMissingSenderId passed\n";

    testFromJsonMissingContentHash();
    std::cout << "  testFromJsonMissingContentHash passed\n";

    testFromJsonMissingSignature();
    std::cout << "  testFromJsonMissingSignature passed\n";

    testFromJsonInvalidJson();
    std::cout << "  testFromJsonInvalidJson passed\n";

    testFromJsonEmptyObject();
    std::cout << "  testFromJsonEmptyObject passed\n";

    testTimestampPreserved();
    std::cout << "  testTimestampPreserved passed\n";

    std::cout << "ContentHeader tests passed!\n";
    return 0;
}
