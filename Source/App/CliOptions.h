#pragma once

#include <string>
#include <optional>

struct CliOptions {
    std::string deviceName;
    std::string loadProjectPath;
    std::string saveProjectPath;
    bool saveOnExit = false;
    int agentPort = 0;
    bool mcpMode = false;
    bool discoverPeers = false;
    std::string advertiseName;
    std::string recordPath;
    std::string recordFormat = "wav:f32";
    std::string audioStreamTarget;  // "stdout" or "tcp://host:port"
    std::string audioStreamFormat = "f32le";
};

struct CliParseResult {
    CliOptions options;
    std::optional<int> exitCode;
};

CliParseResult parseCommandLine(int argc, char* argv[]);
