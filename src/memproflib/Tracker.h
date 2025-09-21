#pragma once
#include "Types.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <atomic>

namespace memprof {

    struct Allocation {
        void* addr{};
        size_t size{};
        bool isArray{};
        const char* file{};
        int line{};
        const char* typeName{};
        ms_t timestamp_ms{};
    };

    class SocketClient; // forward

    class Tracker {
    public:
        static Tracker& instance();

        void onAlloc(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray);
        void onFree(void* addr, bool isArray);

        void sendSnapshot();     // métricas rápidas a GUI
        void reportAtExit();     // resumen final + leaks

    private:
        Tracker();
        ~Tracker();

        ms_t now_ms() const;

        std::mutex m_;
        std::unordered_map<void*, Allocation> live_;
        std::atomic<size_t> currBytes_{0};
        std::atomic<size_t> maxBytes_{0};
        std::atomic<size_t> totalAllocs_{0};
        std::atomic<size_t> totalFrees_{0};
        SocketClient* sockClient_{nullptr};
    };

} // namespace memprof
