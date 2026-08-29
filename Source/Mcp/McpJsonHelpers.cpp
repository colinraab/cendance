#include "McpJsonHelpers.h"

//==============================================================================
/* Argument extraction helpers — safe conversion from juce::var to primitives. */

double McpJsonHelpers::toDouble (juce::var const& v, double fallback)
{
    if (v.isInt64()) return (double)(juce::int64)v;
    if (v.isInt())   return (double)(int)v;
    if (v.isDouble()) return (double)v;
    return fallback;
}

int McpJsonHelpers::toInt (juce::var const& v, int fallback)
{
    if (v.isInt64()) return (int)(juce::int64)v;
    if (v.isInt())   return (int)v;
    return fallback;
}

juce::String McpJsonHelpers::toString (juce::var const& v, const char* fallback)
{
    return v.isString() ? v.toString() : juce::String (fallback);
}

bool McpJsonHelpers::toBool (juce::var const& v, bool fallback)
{
    return v.isBool() ? (bool)v : fallback;
}

juce::String McpJsonHelpers::quoteArg (const juce::String& text)
{
    juce::String out;
    for (int i = 0; i < text.length(); ++i)
    {
        if (text[i] == '\\')      out << "\\\\";
        else if (text[i] == '\"')  out << "\\\"";
        else                        out << text[i];
    }
    return "\"" + out + "\"";
}

//==============================================================================
/* JSON-RPC 2.0 result / error builders. */

juce::var McpJsonHelpers::makeJsonError (int code, const juce::String& msg, const juce::var& id)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("jsonrpc", "2.0");
    if (! id.isVoid()) obj->setProperty ("id", id);
    auto* err = new juce::DynamicObject();
    err->setProperty ("code", code);
    err->setProperty ("message", msg);
    obj->setProperty ("error", juce::var (err));
    return juce::var (obj);
}

juce::var McpJsonHelpers::makeResult (const juce::var& result, const juce::var& id)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("jsonrpc", "2.0");
    if (! id.isVoid()) obj->setProperty ("id", id);
    obj->setProperty ("result", result);
    return juce::var (obj);
}

juce::var McpJsonHelpers::makeTextContent (const juce::String& text)
{
    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> arr;
    auto* item = new juce::DynamicObject();
    item->setProperty ("type", "text");
    item->setProperty ("text", text);
    arr.add (juce::var(item));
    obj->setProperty ("content", arr);
    return juce::var(obj);
}

juce::var McpJsonHelpers::makeToolError (const juce::String& msg, const juce::var& id)
{
    // MCP spec: tools/call errors use result with isError flag
    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> arr;
    auto* errItem = new juce::DynamicObject();
    errItem->setProperty ("type", "text");
    errItem->setProperty ("text", msg);
    arr.add (juce::var(errItem));

    auto* content = new juce::DynamicObject();
    content->setProperty ("content", arr);
    content->setProperty ("isError", true);
    return content;
}
