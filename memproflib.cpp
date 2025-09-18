//
// Created by kevin on 17/9/25.
//

#include "memproflib.h"

#include <iostream>

void* operator new(size_t size) {
    std::cout << "Alocar memoria\n";
    return malloc(size);
}

void* operator new[](size_t size) {
    std::cout << "Alocar memoria[]\n";
    return malloc(size);
}

void operator delete(void* p) noexcept {
    std::cout << "Liberar memoria\n";
    free(p);
 }

void operator delete[](void* p) noexcept {
    std::cout << "Liberar memoria[]\n";
    free(p);
}



int suma(int a, int b) {

    return a + b;
}
