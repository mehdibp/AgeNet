#include "PythonBridge.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace veins {


PythonBridge::~PythonBridge() {
    close();
}

bool PythonBridge::isConnected() const noexcept {
    return socketFd >= 0;
}

void PythonBridge::close() noexcept {
    if (socketFd >= 0) { ::close(socketFd); socketFd = -1; }
    readBuffer.clear();
}

void PythonBridge::connectToServer(const std::string& host, int port, double timeoutSeconds) {
    if (isConnected()) return;

    if (port < 1 || port > 65535) {
        throw std::runtime_error("Python bridge port must be in range 1..65535");
    }

    socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        throw std::runtime_error(std::string("Cannot create Python bridge socket: ") + std::strerror(errno));
    }

    // Avoid waiting forever if the Python process stops responding.
    timeval timeout{};
    timeout.tv_sec  = static_cast<time_t>(timeoutSeconds);
    timeout.tv_usec = static_cast<suseconds_t>((timeoutSeconds - timeout.tv_sec) * 1e6);

    if (::setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0 ||
        ::setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        const std::string error = std::strerror(errno);
        close();
        throw std::runtime_error("Cannot configure Python bridge timeout: " + error);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        close();
        throw std::runtime_error("Python bridge host must be a valid IPv4 address: " + host);
    }

    if (::connect(socketFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        const std::string error = std::strerror(errno);
        close();
        throw std::runtime_error("Cannot connect to Python at " + host + ":" + std::to_string(port) + ": " + error);
    }
}

nlohmann::json PythonBridge::exchange(const nlohmann::json& message) {
    if (!isConnected()) {
        throw std::runtime_error("Python bridge is not connected");
    }

    sendAll(message.dump() + "\n");
    const std::string response = receiveLine();

    try {
        return nlohmann::json::parse(response);
    }
    catch (const nlohmann::json::exception& error) {
        close();
        throw std::runtime_error(std::string("Python returned invalid JSON: ") + error.what());
    }
}

void PythonBridge::sendAll(const std::string& data) {
    std::size_t sent = 0;

    while (sent < data.size()) {
        const ssize_t count = ::send(socketFd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (count < 0) {
            const std::string error = std::strerror(errno);
            close();
            throw std::runtime_error("Failed sending message to Python: " + error);
        }
        if (count == 0) {
            close();
            throw std::runtime_error("Python closed the connection while receiving a message");
        }
        sent += static_cast<std::size_t>(count);
    }
}

std::string PythonBridge::receiveLine() {
    while (true) {
        const std::size_t newline = readBuffer.find('\n');
        if (newline != std::string::npos) {
            std::string line = readBuffer.substr(0, newline);
            readBuffer.erase(0, newline + 1);
            return line;
        }

        if (readBuffer.size() > MAX_LINE_SIZE) {
            close();
            throw std::runtime_error("Python response exceeded the maximum line size");
        }

        char chunk[65536];
        const ssize_t count = ::recv(socketFd, chunk, sizeof(chunk), 0);
        if (count < 0) {
            const std::string error = std::strerror(errno);
            close();
            throw std::runtime_error("Failed receiving response from Python: " + error);
        }
        if (count == 0) {
            close();
            throw std::runtime_error("Python closed the connection before completing its response");
        }
        readBuffer.append(chunk, static_cast<std::size_t>(count));
    }
}


} // namespace veins
