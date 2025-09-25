#pragma once
#include <cstddef>
#include <new>

// Overloads “normales” (sin file/line) – opcionales pero útiles si quieres capturar todo
void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void  operator delete(void* p) noexcept;
void  operator delete[](void* p) noexcept;
// Sized delete (MSVC/GCC/Clang pueden llamarlas)
void  operator delete(void* p, std::size_t) noexcept;
void  operator delete[](void* p, std::size_t) noexcept;

// Overloads con metadata de llamada (archivo/linea) – ESTOS son los que queremos usar
void* operator new(std::size_t size, const char* file, int line);
void* operator new[](std::size_t size, const char* file, int line);

// “Placement delete” que empareja la forma anterior cuando un ctor lanza excepción
void  operator delete(void* p, const char* file, int line) noexcept;
void  operator delete[](void* p, const char* file, int line) noexcept;

// --------- Macro opt-in para este TU ---------
// Si el TU define HOOKS_ENABLE_FILELINE antes de incluir este header,
// entonces todos los `new` de ese TU pasarán (file,line).
#ifdef HOOKS_ENABLE_FILELINE
// Nota: esto NO toca placement-new (new (buf) T) ni nothrow (new (std::nothrow) T)
// porque esas formas no se expanden a `new(__FILE__,__LINE__)`.
#define new new(__FILE__, __LINE__)
#endif
