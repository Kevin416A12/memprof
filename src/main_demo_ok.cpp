#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

#define HOOKS_ENABLE_FILELINE   // <<--- ACTIVA el macro para este TU
#include "memproflib/Hooks.h"              // <<--- Debe venir DESPUÉS del define

#include "memproflib/Tracker.h"
#include "memproflib/SocketClienteTest.h"

int main() {
    Tracker tracker;
    int* singleInt = new int[10000000];               // ahora pasa (file,line) desde ESTE archiv
    delete singleInt;                        // idem

    std::cout << "Valor de singleInt: " << *singleInt << std::endl; // ojo: use-after-free
    return 0;
}
