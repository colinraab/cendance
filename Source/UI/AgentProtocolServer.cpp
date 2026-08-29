#include "AgentProtocolServer.h"

#include <cstring>
#include <thread>
#include <atomic>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <io.h>
#endif

AgentProtocolServer::AgentProtocolServer(int serverPort, ExecuteFn execute)
    : port(serverPort), executeCommand(std::move(execute)) {}

AgentProtocolServer::~AgentProtocolServer() { stop(); }

bool AgentProtocolServer::start(std::string &error) {
    if (port <= 0) {
        error = "Agent port must be positive.";
        return false;
    }
#if defined(_WIN32)
    error = "Agent protocol server is not implemented on Windows yet.";
    return false;
#else
    serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        error = "Could not create agent socket.";
        return false;
    }

    int reuse = 1;
    ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(serverFd, reinterpret_cast<sockaddr *>(&address),
               sizeof(address)) != 0) {
        error = "Could not bind agent socket on 127.0.0.1:" +
                std::to_string(port) + ".";
        closeServerFd();
        return false;
    }

    if (::listen(serverFd, 8) != 0) {
        error = "Could not listen on agent socket.";
        closeServerFd();
        return false;
    }

    running.store(true, std::memory_order_relaxed);
    serverThread = std::thread([this]() { runServer(); });
    return true;
#endif
}

void AgentProtocolServer::stop() {
    running.store(false, std::memory_order_relaxed);
#if !defined(_WIN32)
    closeServerFd();
#endif
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

#if !defined(_WIN32)
void AgentProtocolServer::closeServerFd() {
    const int fd = serverFd.exchange(-1, std::memory_order_acq_rel);
    if (fd >= 0) {
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    }
}

void AgentProtocolServer::runServer() {
    while (running.load(std::memory_order_relaxed)) {
        const int fd = serverFd.load(std::memory_order_acquire);
        if (fd < 0) {
            break;
        }

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);
        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        const int ready = ::select(fd + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0) {
            continue;
        }

        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        const int clientFd = ::accept(
            fd, reinterpret_cast<sockaddr *>(&clientAddress), &clientLength);
        if (clientFd < 0) {
            continue;
        }
        handleClient(clientFd);
        ::close(clientFd);
    }
}

void AgentProtocolServer::handleClient(int clientFd) {
    std::string line;
    char buffer[256];
    while (running.load(std::memory_order_relaxed)) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientFd, &readSet);
        timeval timeout{};
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        const int ready =
            ::select(clientFd + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0) {
            break;
        }

        const ssize_t count = ::recv(clientFd, buffer, sizeof(buffer), 0);
        if (count <= 0) {
            break;
        }

        for (ssize_t i = 0; i < count; ++i) {
            const char ch = buffer[i];
            if (ch == '\n') {
                const std::string response = executeCommand(line) + "\n";
                ::send(clientFd, response.data(), response.size(), 0);
                return;
            } else if (ch != '\r') {
                line.push_back(ch);
                if (line.size() > 4096) {
                    const std::string response =
                        "{\"ok\":false,\"message\":\"Command line too long.\"}\n";
                    ::send(clientFd, response.data(), response.size(), 0);
                    return;
                }
            }
        }
    }
}
#endif
