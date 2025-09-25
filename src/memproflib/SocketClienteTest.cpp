#include "SocketClienteTest.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

void enviar_mensaje_socket(const char* mensaje) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) == 0) {
        send(sock, mensaje, (int)strlen(mensaje), 0);
        std::cout << "Mensaje enviado\n";
    } else {
        std::cout << "No se pudo conectar\n";
    }

    closesocket(sock);
    WSACleanup();
}