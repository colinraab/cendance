#include "McpServer.h"

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <iostream>
#include <sstream>
#include <csignal>
#include <atomic>
#include <map>
#include <vector>

#if _WIN32
  #include <io.h>
#else
  #include <unistd.h>
#endif

//==============================================================================
// Stdio I/O
//==============================================================================

void McpServer::writeStdout (const juce::var& response)
{
    juce::String json = juce::JSON::toString (response, false);
    // MCP stdio protocol: Content-Type header + body + newline
    juce::String header = "Content-Type: application/json\r\nContent-Length: ";
    auto body = header + juce::String(json.getNumBytesAsUTF8()) + "\r\n\r\n" + json + "\r\n";
    std::cout.write(body.toRawUTF8(), (long)body.getNumBytesAsUTF8());
    std::cout.flush();
}

juce::String McpServer::readStdinLine()
{
    std::string s;
    std::getline (std::cin, s);
    return juce::String (s);
}

std::string McpServer::readExactBytes (size_t n)
{
    std::string buf (n, '\0');
    size_t total = 0;
    while (total < n)
    {
        std::cin.read (const_cast<char*> (buf.data()) + total,
                       static_cast<std::streamsize>(n - total));
        auto read = static_cast<size_t>(std::cin.gcount());
        if (read == 0)
        {
            buf.resize (total);
            return buf;
        }
        total += read;
    }
    return buf;
}

void McpServer::run()
{
    running_.store(true);
    stopping_.store(false);

    // Handle SIGINT / SIGTERM for graceful shutdown
    struct sigaction sa{};
    sa.sa_handler = [](int){ /* caught */ };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (!stopping_.load())
    {
        // Check stdin for data with a short poll
        fd_set readfds;
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval tv{};
        tv.tv_sec  = 0;
        tv.tv_usec = 50000; // 50ms poll

        int n = select(STDIN_FILENO+1, &readfds, nullptr, nullptr, &tv);
        if (n < 0 || stopping_.load()) break;
        if (n == 0) {
            // No data — yield to JUCE message thread to keep timer callbacks running
            juce::Thread::sleep(1);
            continue;
        }

        // Read header lines, collecting all headers until blank line
        std::map<std::string, std::string> headers;
        juce::String body;
        auto firstLine = readStdinLine();
        if (firstLine.isEmpty() || std::cin.eof()) break;

        // If first line starts with '{' or '[', treat as raw JSON body
        auto trimmed = firstLine.trim();
        if (trimmed.startsWith("{") || trimmed.startsWith("[")) {
            body = firstLine;
            while (true) {
                auto extra = readStdinLine();
                if (extra.trim().isEmpty()) break;
                body += "\n" + extra;
                if (std::cin.eof()) break;
            }
        } else {
            // Collect all header lines until blank line
            juce::StringArray lines;
            lines.add(firstLine);
            while (true) {
                auto hdrLine = readStdinLine();
                if (hdrLine.isEmpty() || std::cin.eof()) break;
                if (hdrLine.trim().isEmpty()) break;
                lines.add(hdrLine);
            }
            for (auto& line : lines) {
                auto colon = line.indexOf(":");
                if (colon > 0) {
                    auto key = line.substring(0, colon).trim().toLowerCase().toStdString();
                    auto val = line.substring(colon + 1).trim().toStdString();
                    headers[key] = val;
                }
            }
            if (headers.find("content-length") == headers.end()) continue;
            int contentLength = juce::String(headers["content-length"]).getIntValue();
            if (contentLength <= 0) continue;

            size_t sz = static_cast<size_t>(contentLength);
            std::string buf = readExactBytes(sz);
            if (buf.size() < sz) continue;
            body = juce::String(buf);
        }

        if (body.isEmpty()) continue;

        // Guard against parse/execute exceptions
        try {
            auto req = juce::JSON::parse(body);
            auto resp = dispatchRequest(req);
            if (!resp.isVoid())
                writeStdout(resp);
        } catch (const std::exception& e) {
            // Emit JSON-RPC parse error for malformed input
            auto err = makeJsonError(-32700,
                                     juce::String("Internal error: ") + e.what(),
                                     juce::var());
            writeStdout(err);
        } catch (...) {
            auto err = makeJsonError(-32603, "Internal error", juce::var());
            writeStdout(err);
        }
    }

    running_.store(false);
}

void McpServer::stop()
{
    stopping_.store(true);
}
