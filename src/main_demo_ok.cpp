#include <iostream>
#include <thread>
#include <chrono>
#include "memproflib/Tracker.h"

int main() {
    std::cout << "Demo OK start\n";

    // Asignación simple
    int* a = new int(42);
    delete a;

    // Varias asignaciones y liberaciones
    for (int i=0; i<100; ++i) {
        char* p = new char[128];
        delete[] p;
    }

    // Uno sin liberar (para ver en snapshot)
    void* leak1 = std::malloc(10*1024);
    (void)leak1;

    memprof::Tracker::instance().sendSnapshot();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Demo OK end\n";
    return 0;
}
