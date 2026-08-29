#include "ContentHeader.h"

#include <juce_core/juce_core.h>

#include <iomanip>
#include <sstream>

std::string ContentHeader::toJson() const {
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("sender_id", juce::String(sender_id));
    root->setProperty("timestamp", static_cast<int64_t>(timestamp));
    root->setProperty("content_hash", juce::String(content_hash));
    root->setProperty("signature", juce::String(signature));
    root->setProperty("content_type", static_cast<int>(content_type));
    return juce::JSON::toString(juce::var(root.release()), false).toStdString();
}

bool ContentHeader::fromJson(const std::string& json, ContentHeader& out, std::string& error) {
    error.clear();
    auto parsed = juce::JSON::parse(juce::String(json));
    if (parsed.isVoid() || !parsed.isObject()) {
        error = "ContentHeader: invalid JSON";
        return false;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj) {
        error = "ContentHeader: not a JSON object";
        return false;
    }

    juce::String sid = obj->getProperty("sender_id");
    if (sid.isEmpty()) {
        error = "ContentHeader: missing sender_id";
        return false;
    }
    out.sender_id = sid.toStdString();

    out.timestamp = static_cast<uint64_t>(static_cast<int64_t>(obj->getProperty("timestamp")));

    juce::String hash = obj->getProperty("content_hash");
    if (hash.isEmpty()) {
        error = "ContentHeader: missing content_hash";
        return false;
    }
    out.content_hash = hash.toStdString();

    juce::String sig = obj->getProperty("signature");
    if (sig.isEmpty()) {
        error = "ContentHeader: missing signature";
        return false;
    }
    out.signature = sig.toStdString();

    int ct = static_cast<int>(obj->getProperty("content_type"));
    out.content_type = static_cast<ContentType>(ct);

    return true;
}
