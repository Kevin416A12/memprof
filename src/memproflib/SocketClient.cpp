#include "SocketClient.h"
#include <cstring>
#include <iostream>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

namespace memprof {

SocketClient::SocketClient(const std::string& host, int port) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) { connected_ = false; sock_ = nullptr; return; }
    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { connected_ = false; sock_ = nullptr; return; }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(static_cast<u_short>(port));
    serv.sin_addr.s_addr = inet_addr(host.c_str());
    if (serv.sin_addr.s_addr == INADDR_NONE) {
        IN_ADDR inaddr{};
        if (InetPtonA(AF_INET, host.c_str(), &inaddr) != 1) {
            closesocket(s); WSACleanup(); connected_ = false; sock_ = nullptr; return;
        }
        serv.sin_addr = inaddr;
    }
    if (::connect(s, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR) {
        closesocket(s); connected_ = false; sock_ = nullptr; WSACleanup(); return;
    }
    sock_ = reinterpret_cast<void*>(s);
    connected_ = true;
#else
    int s = ::socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { connected_ = false; sock_ = -1; return; }
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serv.sin_addr) <= 0) { ::close(s); connected_ = false; sock_ = -1; return; }
    if (::connect(s, (sockaddr*)&serv, sizeof(serv)) < 0) { ::close(s); connected_ = false; sock_ = -1; return; }
    sock_ = s; connected_ = true;
#endif
}

SocketClient::~SocketClient() {
#ifdef _WIN32
    if (connected_ && sock_) { ::closesocket(reinterpret_cast<SOCKET>(sock_)); }
    WSACleanup();
#else
    if (connected_ && sock_ >= 0) { ::close(sock_); }
#endif
}

bool SocketClient::connected() const { return connected_; }

void SocketClient::send(const std::string& line) {
    if (!connected_) return;
#ifdef _WIN32
    SOCKET s = reinterpret_cast<SOCKET>(sock_);
    const char* data = line.c_str(); size_t left = line.size();
    while (left > 0) {
        int sent = ::send(s, data, (int)left, 0);
        if (sent <= 0) { connected_ = false; return; }
        data += sent; left -= sent;
    }
#else
    int s = sock_; const char* data = line.c_str(); size_t left = line.size();
    while (left > 0) {
        ssize_t w = ::write(s, data, left);
        if (w <= 0) { connected_ = false; return; }
        data += w; left -= (size_t)w;
    }
#endif
}

} // namespace memprof
