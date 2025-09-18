#include <iostream>
#include "memproflib.h"
//
// Created by kevin on 17/9/25.
//
int main() {

    std::cout << "Vamos a usar el metodo suma de la biblioteca\n";

    int* ptr;
    ptr = new int;
    *ptr = 7;
    std::cout << "Valor de ptr: " << *ptr << "\n";
    int* punteroArray = new int[5];
    delete[] punteroArray;
    delete ptr;
}