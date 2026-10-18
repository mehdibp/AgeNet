#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace veins {


// Owns the TCP connection between one AgeNetApp instance and the
// Python AgeNet process. This class deliberately knows nothing about
// OMNeT++ events, vehicle state, neighbor discovery, or RL logic.

class PythonBridge {
    private:
        int socketFd = -1;
        static constexpr std::size_t MAX_LINE_SIZE = 1024 * 1024;

        void sendAll(const std::string& data);
        std::string receiveLine();

    public:
        PythonBridge() = default;
        PythonBridge(const PythonBridge&) = delete;
        PythonBridge& operator=(const PythonBridge&) = delete;

        ~PythonBridge();

        void connectToServer(const std::string& host, int port);
        nlohmann::json exchange(const nlohmann::json& message);
        void close() noexcept;
        bool isConnected() const noexcept;
};

} // namespace veins
