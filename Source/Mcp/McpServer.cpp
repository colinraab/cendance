#include "McpServer.h"
#include "McpJsonHelpers.h"

#include <juce_core/juce_core.h>

//==============================================================================
McpServer::McpServer (ExecuteFn execute, P2PFn p2pExecute)
    : executeFn_ (std::move (execute)),
      p2pFn_ (std::move (p2pExecute))
{
    // JUCE_ASSERT
}

//==============================================================================
// JSON helpers — delegate to the shared utility library
//==============================================================================

juce::var McpServer::makeJsonError (int code, const juce::String& msg, const juce::var& id)
{ return McpJsonHelpers::makeJsonError (code, msg, id); }

juce::var McpServer::makeResult (const juce::var& result, const juce::var& id)
{ return McpJsonHelpers::makeResult (result, id); }

juce::var McpServer::makeTextContent (const juce::String& text)
{ return McpJsonHelpers::makeTextContent (text); }

juce::var McpServer::makeToolError (const juce::String& msg, const juce::var& id)
{ return McpJsonHelpers::makeToolError (msg, id); }

double McpServer::getDouble (juce::var const& v, double fallback)
{ return McpJsonHelpers::toDouble (v, fallback); }

int McpServer::getInt (juce::var const& v, int fallback)
{ return McpJsonHelpers::toInt (v, fallback); }

juce::String McpServer::getString (juce::var const& v, const char* fb)
{ return McpJsonHelpers::toString (v, fb); }

bool McpServer::getBool (juce::var const& v, bool fb)
{ return McpJsonHelpers::toBool (v, fb); }

juce::String McpServer::quoteArg (const juce::String& text)
{ return McpJsonHelpers::quoteArg (text); }

//==============================================================================
// MCP method dispatch — routes to handler implementations in other .cpp files
//==============================================================================

juce::var McpServer::dispatchRequest (juce::var const& req)
{
    if (req.isVoid())
        return makeJsonError(-32700, "Parse error", juce::var());

    auto method = req.getProperty("method", juce::var());
    if (method.isVoid())
        return makeJsonError(-32600, "Invalid Request", req.getProperty("id", juce::var()));

    auto params = req.getProperty("params", juce::var());
    auto id     = req.getProperty("id", juce::var());

    // JSON-RPC 2.0: notifications have no id field — server MUST NOT reply.
    if (id.isVoid())
        return juce::var();

    if (method.toString() == "initialize")
        return handleInitialize(params, id);
    if (method.toString() == "tools/list")
        return handleToolsList(params, id);
    if (method.toString() == "tools/call")
        return handleToolsCall(params, id);
    if (method.toString() == "resources/list")
        return handleResourcesList(params, id);
    if (method.toString() == "resources/read")
        return handleResourcesRead(params, id);
    if (method.toString() == "prompts/list")
        return handlePromptsList(params, id);
    if (method.toString() == "prompts/get")
        return handlePromptsGet(params, id);

    return makeJsonError(-32601, "Method not found", id);
}

//==============================================================================
// initialize
//==============================================================================

juce::var McpServer::handleInitialize (juce::var const& params, const juce::var& id)
{
    juce::ignoreUnused(params);
    auto* caps = new juce::DynamicObject();
    auto* tc = new juce::DynamicObject();
    caps->setProperty("tools", juce::var(tc));
    auto* rc = new juce::DynamicObject();
    caps->setProperty("resources", juce::var(rc));
    auto* pc = new juce::DynamicObject();
    caps->setProperty("prompts", juce::var(pc));

    auto* si = new juce::DynamicObject();
    si->setProperty("name", "cendance");
    si->setProperty("version", "0.1.0");

    auto* res = new juce::DynamicObject();
    res->setProperty("protocolVersion","2024-11-05");
    res->setProperty("capabilities", juce::var(caps));
    res->setProperty("serverInfo", juce::var(si));

    return makeResult(juce::var(res), id);
}
