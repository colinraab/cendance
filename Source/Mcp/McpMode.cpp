#include "McpMode.h"

#include "../App/AgentCommand.h"
#include "McpServer.h"

#include <juce_core/juce_core.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

using namespace juce;

namespace {

class MeterHistoryCollector {
public:
    explicit MeterHistoryCollector(MeterQueue& meterQueue) : meterQueue(meterQueue) {
        start();
    }

    ~MeterHistoryCollector() {
        running.store(false);
        if (thread.joinable()) thread.join();
    }

    void start() {
        MeterQueue* mq = &meterQueue;
        thread = std::thread([this, mq]() {
            while (running.load()) {
                MeterData current;
                if (mq->popLatest(current) && current.analyzerValid) {
                    std::lock_guard<std::mutex> lock(mutex);
                    history.push_back(current);
                    while (history.size() > 900) history.erase(history.begin());
                }
                juce::Thread::sleep(33);
            }
        });
    }

    std::pair<MeterData, std::vector<MeterData>> snapshot() {
        std::lock_guard<std::mutex> lock(mutex);
        MeterData current;
        if (!history.empty()) current = history.back();
        return {current, history};
    }

private:
    MeterQueue& meterQueue;
    std::mutex mutex;
    std::vector<MeterData> history;
    std::atomic<bool> running{true};
    std::thread thread;
};

} // namespace

int runMcpMode(McpModeContext context) {
    MeterHistoryCollector meterCollector(context.meterQueue);

    auto mcpDispatchFn = [&context, &meterCollector]
                          (const String& input) -> String {
        // Check if this is a P2P command
        std::string inputStd = input.toStdString();
        if (inputStd.rfind("p2p ", 0) == 0 && inputStd.size() > 4) {
            std::string rest = inputStd.substr(4);
            size_t spacePos = rest.find(' ');
            if (spacePos != std::string::npos) {
                std::string toolName = rest.substr(0, spacePos);
                std::string argsJson = rest.substr(spacePos + 1);
                return context.p2pToolHandler.handle(String(toolName), String(argsJson));
            }
        }

        auto [current, hist] = meterCollector.snapshot();
        auto ctx = AgentCommand::ExecutionContext {
            context.appState, &context.commandQueue, current, hist,
            [&context](const Command& cmd, const std::string&, const Command&) {
                return context.commandQueue.push(cmd);
            },
            &context.contributionLibrary
        };
        auto resp = AgentCommand::execute(input.toStdString(), ctx);
        return String(resp.json);
    };

    McpServer mcp(std::move(mcpDispatchFn), [&context](const String& toolName, const String& argsJson) {
        return context.p2pToolHandler.handle(toolName, argsJson);
    });
    mcp.run();

    return 0;
}
