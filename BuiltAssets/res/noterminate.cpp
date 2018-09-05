#include <iostream>
#include <stdarg.h>
#include <string>
#include <setjmp.h>

char* msg;
jmp_buf env;
int (*ftFunc)();

extern "C" __declspec(dllexport)
void _gfortran_runtime_error_at (const char * c, const char * w, ...) {
    char buffer[1024];
    va_list args;
    va_start (args, w);
    vsprintf (buffer, w, args);
    va_end (args);
    static std::string err = std::string(c) + "\n" + std::string(buffer);
    msg = &err[0];
    longjmp(env, 1);
}

char* Exec() {
    if (!setjmp(env)) {
        ftFunc();
        return nullptr;
    }
    else {
        return msg;
    }
}