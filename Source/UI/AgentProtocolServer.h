#pragma once

#include <functional>
#include <string>
#include <thread>
#include <atomic>

/* Standalone TCP server for the cendance agent line protocol.
 * Listens on localhost, reads one line per request, responds via write(). */

class AgentProtocolServer {
public:
    using ExecuteFn = std::function<std::string(const std::string &)>;

    explicit AgentProtocolServer(int serverPort, ExecuteFn execute);
    ~AgentProtocolServer();

    bool start(std::string &error);
    void stop();

private:
#if !defined(_WIN32)
    void closeServerFd();
    void runServer();
    void handleClient(int clientFd);
#endif

    int port = 0;
    ExecuteFn executeCommand;
    std::thread serverThread;
    std::atomic<bool> running{false};
#if !defined(_WIN32)
    std::atomic<int> serverFd{-1};
#endif
};
