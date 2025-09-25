#include <stdio.h>
#include "Tracker.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>   // std::hex, std::dec
#include <chrono>
#include <string>
#include <cstdint>   // uintptr_t
#include "SocketClienteTest.h"
#include <sstream>


void Tracker::onAlloc(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray) {
    using namespace std::chrono;

    // Timestamp en milisegundos
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    std::ostringstream ss;
    ss << "{"
       << "\"type\":\"ALLOC\","
       << "\"addr\":\"0x" << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << "\","
       << "\"size\":" << size << ","
       << "\"file\":\"" << file << "\","
       << "\"line\":" << line << ","
       << "\"typeName\":\"" << typeName;
    if (false) ss << "[]";
    ss << "\","
       << "\"t\":" << now
       << "}\r\n";
   std::cout << ss.str();

    std::string logLine = ss.str();

    // Envías el mensaje por socket
    enviar_mensaje_socket(logLine.c_str());
}
void Tracker::Delete(void* addr, size_t /*size*/, const char* /*file*/, int /*line*/,
                     const char* /*typeName*/, bool /*isArray*/) {
   using namespace std::chrono;

   // Timestamp en milisegundos desde epoch
   const auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

   // Construir SOLO los campos requeridos
   std::ostringstream ss;
   ss << "{\"type\":\"FREE\","
      << "\"addr\":\"0x" << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << "\","
      << "\"t\":" << now
      << "}";

   ss << "\r\n";

   const std::string logLine = ss.str();

   // Log local (debug)
   std::cout << logLine << '\n';

   // Enviar por socket
   enviar_mensaje_socket(logLine.c_str());
}


    void Tracker::onAllocarr(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray) {
    using namespace std::chrono;

    // Timestamp en milisegundos
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    std::ostringstream ss;
    ss << "{"
       << "\"type\":\"ALLOC\","
       << "\"addr\":\"0x" << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << "\","
       << "\"size\":" << size << ","
       << "\"file\":\"" << file << "\","
       << "\"line\":" << line << ","
       << "\"typeName\":\"" << typeName;
    if (false) ss << "[]";
    ss << "\","
       << "\"t\":" << now
       << "}\r\n";

    std::string logLine = ss.str();
   std::cout << ss.str();
    // Envías el mensaje por socket
    enviar_mensaje_socket(logLine.c_str());
    }

    void Tracker::Deletearr(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray) {
   using namespace std::chrono;

   // Timestamp en milisegundos
   auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

   std::ostringstream ss;
   ss << "{"
      << "\"type\":\"ALLOC\","
      << "\"addr\":\"0x" << std::hex << reinterpret_cast<uintptr_t>(addr) << std::dec << "\","
      << "\"size\":" << size << ","
      << "\"file\":\"" << file << "\","
      << "\"line\":" << line << ","
      << "\"typeName\":\"" << typeName;
   if (false) ss << "[]";
   ss << "\","
      << "\"t\":" << now
      << "}\r\n";
   std::cout << ss.str();
   std::string logLine = ss.str();

   // Envías el mensaje por socket
   enviar_mensaje_socket(logLine.c_str());
    }
