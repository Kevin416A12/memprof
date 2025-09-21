#pragma once
#include <string>

namespace memprof {
    class SocketClient {
    public:
        SocketClient(const std::string& host, int port);
        ~SocketClient();
        bool connected() const;
        void send(const std::string& line);
    private:
        bool connected_{false};
#ifdef _WIN32
        void* sock_{nullptr}; // SOCKET
#else
        int sock_{-1};
#endif
    };
}
