#include "../Source/App/CliOptions.h"

#include <cassert>
#include <optional>
#include <string>
#include <vector>

namespace {

CliParseResult parse(std::vector<std::string> arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (auto& argument : arguments)
        argv.push_back(argument.data());

    return parseCommandLine(static_cast<int>(argv.size()), argv.data());
}

void testDefaults() {
    const auto result = parse({"cendance"});
    assert(!result.exitCode.has_value());
    assert(result.options.recordFormat == "wav:f32");
    assert(!result.options.mcpMode);
}

void testValidRecordingFormats() {
    for (const auto* format : {"wav:f32", "wav:s16", "flac:24", "flac:16"}) {
        const auto result = parse({"cendance", "--record-format", format});
        assert(!result.exitCode.has_value());
        assert(result.options.recordFormat == format);
    }
}

void testInvalidRecordingFormat() {
    const auto result = parse({"cendance", "--record-format", "mp3"});
    assert(result.exitCode == std::optional<int>(1));
}

void testRemovedStreamingOptionIsRejected() {
    const auto result = parse({"cendance", "--audio-stream", "stdout"});
    assert(result.exitCode == std::optional<int>(1));
}

void testMcpAndRecordingOptions() {
    const auto result = parse({"cendance", "--mcp", "--record", "take.wav",
                               "--record-format", "wav:s16"});
    assert(!result.exitCode.has_value());
    assert(result.options.mcpMode);
    assert(result.options.recordPath == "take.wav");
    assert(result.options.recordFormat == "wav:s16");
}

}  // namespace

int main() {
    testDefaults();
    testValidRecordingFormats();
    testInvalidRecordingFormat();
    testRemovedStreamingOptionIsRejected();
    testMcpAndRecordingOptions();
    return 0;
}
