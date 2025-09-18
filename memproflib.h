//
// Created by kevin on 17/9/25.
//
//memproflib.h
#ifndef MEMPROF_MEMPROFLIB_H
#define MEMPROF_MEMPROFLIB_H

#include <cstdlib>

void* operator new(size_t size);
void operator delete(void* p) noexcept;
void* operator new[](size_t size);
void operator delete[](void* p) noexcept;
int suma(int a, int b);

#endif //MEMPROF_MEMPROFLIB_H