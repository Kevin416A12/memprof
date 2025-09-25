#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

#define HOOKS_ENABLE_FILELINE   // <<--- ACTIVA el macro para este TU
#include "memproflib/Hooks.h"              // <<--- Debe venir DESPUÉS del define

#include "memproflib/Tracker.h"
#include "memproflib/SocketClienteTest.h"

int main() {
    int * singleInt = new int;               // ahora pasa (file,line) desde ESTE archivo
    int* array = new int[10000000];          // idem para new[]


    std::cout << "Valor de singleInt: " << *singleInt << std::endl; // ojo: use-after-free
    return 0;
}
