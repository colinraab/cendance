#include "McpServer.h"
#include "McpBinaryData.h"

//==============================================================================
// resources/list — returns available resources the client can read
//==============================================================================

juce::var McpServer::handleResourcesList (juce::var const&, const juce::var& id)
{
    auto* list = new juce::DynamicObject();
    juce::Array<juce::var> resources;

    auto* manual = new juce::DynamicObject();
    manual->setProperty("uri", "cendance://manual.md");
    manual->setProperty("name", "cendance MCP Manual");
    manual->setProperty("description", "Full documentation for using the cendance MCP server: transport controls, track parameters, preset refs, contribution packages, genre system, and agent workflows.");
    manual->setProperty("mimeType", "text/markdown");
    resources.add(juce::var(manual));

    list->setProperty("resources", resources);
    return makeResult(juce::var(list), id);
}

//==============================================================================
// resources/read — returns the text of a named resource
//==============================================================================

juce::var McpServer::handleResourcesRead (juce::var const& params, const juce::var& id)
{
    auto uri = getString(params.getProperty("uri", juce::var()), "");
    if (uri.isEmpty())
        return makeJsonError(-32602, "uri is required", id);

    if (uri == "cendance://manual.md")
    {
        int dataSize = 0;
        const void* data = McpBinaryData::getNamedResource("manual_md", dataSize);
        if (data != nullptr)
        {
            juce::String content (static_cast<const char*>(data), dataSize);
            auto* contentObj = new juce::DynamicObject();
            contentObj->setProperty("uri", "cendance://manual.md");
            contentObj->setProperty("mimeType", "text/markdown");
            contentObj->setProperty("text", content);
            juce::Array<juce::var> contents;
            contents.add(juce::var(contentObj));
            auto* res = new juce::DynamicObject();
            res->setProperty("contents", contents);
            return makeResult(juce::var(res), id);
        }
        return makeJsonError(-32603, "Embedded manual resource not found", id);
    }

    return makeJsonError(-32602, "Unknown resource: " + uri, id);
}
