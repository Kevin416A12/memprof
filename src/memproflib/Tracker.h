#pragma once
#include "Tracker.h"
#include "SocketClienteTest.h"
class Tracker {
public:
    void onAlloc(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray);
    void Delete(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray);
    void onAllocarr(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray);
    void Deletearr(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray);
};