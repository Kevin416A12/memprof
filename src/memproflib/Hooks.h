#pragma once
#include <cstddef>

void* operator new(size_t size);
void* operator new[](size_t size);
void  operator delete(void* p) noexcept;
void  operator delete[](void* p) noexcept;

// Sized delete (MSVC puede llamarlas en C++17+)
void  operator delete(void* p, size_t) noexcept;
void  operator delete[](void* p, size_t) noexcept;
