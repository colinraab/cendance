#include "CliOptions.h"

#include <iostream>
#include <string>
#include <cstdlib>

CliParseResult parseCommandLine(int argc, char* argv[]) {
    CliOptions options;
    options.agentPort = 0;
    options.mcpMode = false;
    options.discoverPeers = false;
    options.saveOnExit = false;

    if (const char* envAgentPort = std::getenv("CENDANCE_AGENT_PORT")) {
        try {
            options.agentPort = std::stoi(envAgentPort);
        } catch (...) {
            std::cerr << "Invalid CENDANCE_AGENT_PORT: " << envAgentPort << std::endl;
            return {options, 1};
        }
    }

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--device" && i + 1 < argc) {
            options.deviceName = argv[++i];
        } else if (arg == "--load-project" && i + 1 < argc) {
            options.loadProjectPath = argv[++i];
        } else if (arg == "--save-project" && i + 1 < argc) {
            options.saveProjectPath = argv[++i];
        } else if (arg == "--save-on-exit") {
            options.saveOnExit = true;
        } else if (arg == "--agent-port" && i + 1 < argc) {
            try {
                options.agentPort = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Invalid --agent-port value." << std::endl;
                return {options, 1};
            }
        } else if (arg == "--help") {
            std::cout << "cendance options:\n"
                      << "  --device <name>       Select audio device\n"
                      << "  --load-project <path> Load a .cendance project on startup\n"
                      << "  --save-project <path> Set active project path for save operations\n"
                      << "  --save-on-exit        Save project on clean exit\n"
                      << "  --agent-port <port>   Enable localhost agent line protocol\n"
                      << "  --discover-peers      Enable LAN peer discovery via mDNS\n"
                      << "  --advertise-name <n>  Set mDNS service name (default: hostname)\n"
                      << "  --record <path>       Record output to file on startup\n"
                      << "  --record-format <fmt> Recording format: wav:f32, wav:s16, flac:24, flac:16\n"
                      << "  --mcp                 Run headless MCP stdio server\n";
            return {options, 0};
        } else if (arg == "--mcp") {
            options.mcpMode = true;
        } else if (arg == "--discover-peers") {
            options.discoverPeers = true;
        } else if (arg == "--advertise-name" && i + 1 < argc) {
            options.advertiseName = argv[++i];
        } else if (arg == "--record" && i + 1 < argc) {
            options.recordPath = argv[++i];
        } else if (arg == "--record-format" && i + 1 < argc) {
            options.recordFormat = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return {options, 1};
        }
    }

    if (options.agentPort < 0 || options.agentPort > 65535) {
        std::cerr << "Agent port must be between 0 and 65535." << std::endl;
        return {options, 1};
    }

    if (options.recordFormat != "wav:f32"
        && options.recordFormat != "wav:s16"
        && options.recordFormat != "flac:24"
        && options.recordFormat != "flac:16") {
        std::cerr << "Invalid recording format: " << options.recordFormat << std::endl;
        return {options, 1};
    }

    return {options, std::nullopt};
}
