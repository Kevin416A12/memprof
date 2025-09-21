#include "Hooks.h"
#include "Tracker.h"
#include <cstdlib>
#include <new>

using memprof::Tracker;

void* operator new(size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    Tracker::instance().onAlloc(p, size, nullptr, 0, nullptr, false);
    return p;
}

void* operator new[](size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    Tracker::instance().onAlloc(p, size, nullptr, 0, nullptr, true);
    return p;
}

void operator delete(void* p) noexcept {
    Tracker::instance().onFree(p, false);
    std::free(p);
}

void operator delete[](void* p) noexcept {
    Tracker::instance().onFree(p, true);
    std::free(p);
}

// sized delete (llamadas por MSVC en algunos modos)
void operator delete(void* p, size_t) noexcept {
    Tracker::instance().onFree(p, false);
    std::free(p);
}
void operator delete[](void* p, size_t) noexcept {
    Tracker::instance().onFree(p, true);
    std::free(p);
}
