#include <iostream>
#include <thread>
#include <chrono>
#include "memproflib/Tracker.h"

int main() {
    std::cout << "Demo LEAK start\n";
    for (int i=0; i<50; ++i) {
        char* p = new char[1024 * ((i % 8) + 1)];
        (void)p; // intencionalmente no liberado
    }

    // Leak grande
    void* big = std::malloc(2 * 1024 * 1024);
    (void)big;

    memprof::Tracker::instance().sendSnapshot();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Demo LEAK end\n";
    return 0;
}
