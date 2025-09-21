#include "Tracker.h"
#include "SocketClient.h"
#include <chrono>
#include <iostream>
#include <cstdio>
#include <cstdlib>

namespace memprof {

Tracker& Tracker::instance() {
    static Tracker inst;
    return inst;
}

Tracker::Tracker() {
    // Intentar conectar a la GUI (no bloqueante si falla)
    sockClient_ = new SocketClient("127.0.0.1", 5555);
    std::atexit([](){ Tracker::instance().reportAtExit(); });
}

Tracker::~Tracker() {
    delete sockClient_;
}

ms_t Tracker::now_ms() const {
    using namespace std::chrono;
    return (ms_t)duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}

void Tracker::onAlloc(void* addr, size_t size, const char* file, int line, const char* typeName, bool isArray) {
    if (!addr) return;
    Allocation a;
    a.addr = addr; a.size = size; a.isArray = isArray;
    a.file = file ? file : "??";
    a.line = line;
    a.typeName = typeName ? typeName : "unknown";
    a.timestamp_ms = now_ms();

    {
        std::lock_guard<std::mutex> lk(m_);
        live_.emplace(addr, a);
        auto newCurr = currBytes_.fetch_add(size) + size;
        size_t prevMax = maxBytes_.load();
        while (newCurr > prevMax && !maxBytes_.compare_exchange_weak(prevMax, newCurr)) {}
        totalAllocs_.fetch_add(1);
    }

    if (sockClient_ && sockClient_->connected()) {
        char pbuf[32]; std::snprintf(pbuf, sizeof(pbuf), "%p", addr);
        std::string j = "{\"type\":\"ALLOC\",\"addr\":\"";
        j += pbuf;
        j += "\",\"size\":";
        j += std::to_string(size);
        j += ",\"file\":\"";
        j += a.file;
        j += "\",\"line\":";
        j += std::to_string(line);
        j += ",\"typeName\":\"";
        j += a.typeName;
        j += "\",\"t\":";
        j += std::to_string(a.timestamp_ms);
        j += "}\n";
        sockClient_->send(j);
    }
}

void Tracker::onFree(void* addr, bool /*isArray*/) {
    if (!addr) return;
    Allocation a{};
    bool found = false;
    {
        std::lock_guard<std::mutex> lk(m_);
        auto it = live_.find(addr);
        if (it != live_.end()) {
            a = it->second;
            live_.erase(it);
            currBytes_.fetch_sub(a.size);
            totalFrees_.fetch_add(1);
            found = true;
        }
    }

    if (found && sockClient_ && sockClient_->connected()) {
        char pbuf[32]; std::snprintf(pbuf, sizeof(pbuf), "%p", addr);
        std::string j = "{\"type\":\"FREE\",\"addr\":\"";
        j += pbuf;
        j += "\",\"t\":";
        j += std::to_string(now_ms());
        j += "}\n";
        sockClient_->send(j);
    }
}

void Tracker::sendSnapshot() {
    size_t curr = currBytes_.load();
    size_t mx = maxBytes_.load();
    size_t liveCount;
    {
        std::lock_guard<std::mutex> lk(m_);
        liveCount = live_.size();
    }
    if (sockClient_ && sockClient_->connected()) {
        std::string j = "{\"type\":\"SNAPSHOT\",\"t\":";
        j += std::to_string(now_ms());
        j += ",\"currBytes\":";
        j += std::to_string(curr);
        j += ",\"maxBytes\":";
        j += std::to_string(mx);
        j += ",\"liveCount\":";
        j += std::to_string(liveCount);
        j += "}\n";
        sockClient_->send(j);
    }
}

void Tracker::reportAtExit() {
    size_t liveCount;
    size_t leakBytes = 0;
    {
        std::lock_guard<std::mutex> lk(m_);
        liveCount = live_.size();
        for (auto &kv : live_) leakBytes += kv.second.size;
    }

    std::cerr << "[memprof] TotalAllocs="<< totalAllocs_.load()
              << " TotalFrees="<< totalFrees_.load()
              << " MaxBytes="<< maxBytes_.load()
              << " CurrBytes="<< currBytes_.load()
              << " LiveAllocs="<< liveCount
              << " LeakBytes="<< leakBytes << "\n";

    if (sockClient_ && sockClient_->connected()) {
        std::string j = "{\"type\":\"LEAK_SUMMARY\",\"t\":";
        j += std::to_string(now_ms());
        j += ",\"liveCount\":";
        j += std::to_string(liveCount);
        j += ",\"leakBytes\":";
        j += std::to_string(leakBytes);
        j += "}\n";
        sockClient_->send(j);
    }
}

} // namespace memprof
