#include "Hooks.h"
#include "Tracker.h"
#include "SocketClienteTest.h"
#include <cstdlib>
#include <iostream>
#include <new>

// Instancia global (si así lo quieres)
Tracker tracker;

// ================== Overloads con (file,line) ==================
void* operator new(std::size_t size, const char* file, int line) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    // Usa los file/line del sitio de llamada:
    tracker.onAlloc(p, size, file, line, "unknown", /*isArray*/false);
    return p;
}

void* operator new[](std::size_t size, const char* file, int line) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    tracker.onAllocarr(p, size, file, line, "unknown", /*isArray*/true);
    return p;
}

// “Placement delete” correspondiente a los forms anteriores (llamado si el ctor lanza)
void operator delete(void* p, const char* /*file*/, int /*line*/) noexcept {
    if (!p) return;
    std::cout << "usando el bueno" << std::endl;
    tracker.Delete(p, 0, nullptr, 0, "unknown", /*isArray*/false);
    std::free(p);
}
void operator delete[](void* p, const char* /*file*/, int /*line*/) noexcept {
    if (!p) return;
    tracker.Deletearr(p, 0, nullptr, 0, "unknown", /*isArray*/true);
    std::free(p);
}

// ================== Overloads “normales” (sin file/line) ==================
void* operator new(std::size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    // Sin file/line porque no se usó el macro
    tracker.onAlloc(p, size, /*file*/nullptr, /*line*/0, "unknown", /*isArray*/false);
    return p;
}
void* operator new[](std::size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    tracker.onAllocarr(p, size, /*file*/nullptr, /*line*/0, "unknown", /*isArray*/true);
    return p;
}

void operator delete(void* p) noexcept {
    if (!p) return;
    tracker.Delete(p, 0, nullptr, 0, "unknown", /*isArray*/false);
    std::cout << "usando el malo" << std::endl;
    std::free(p);
}
void operator delete[](void* p) noexcept {
    if (!p) return;
    tracker.Deletearr(p, 0, nullptr, 0, "unknown", /*isArray*/true);
    std::free(p);
}

// sized delete
void operator delete(void* p, std::size_t) noexcept {
    operator delete(p);
}
void operator delete[](void* p, std::size_t) noexcept {
    operator delete[](p);
}
