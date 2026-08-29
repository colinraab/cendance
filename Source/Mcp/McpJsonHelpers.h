#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
/* McpJsonHelpers — static utilities shared across all MCP server translation
   units: argument extraction, JSON-RPC result builders, and MCP tool errors. */

class McpJsonHelpers
{
public:
    static double  toDouble    (juce::var const& v, double fallback);
    static int     toInt       (juce::var const& v, int fallback);
    static juce::String toString (juce::var const& v, const char* fallback);
    static bool    toBool      (juce::var const& v, bool fallback);
    static juce::String quoteArg (const juce::String& text);

    // JSON-RPC 2.0 result / error builders
    static juce::var makeResult      (const juce::var& result, const juce::var& id);
    static juce::var makeTextContent (const juce::String& text);
    static juce::var makeToolError   (const juce::String& msg, const juce::var& id);
    static juce::var makeJsonError   (int code, const juce::String& msg, const juce::var& id);
};
