#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <setjmp.h>

std::string __noterm_msg;
jmp_buf __noterm_env;
void (*__noterm_ftFunc)();
void (*__noterm_cFunc)();

extern "C" __declspec(dllexport)
void _gfortran_runtime_error_at (const char * c, const char * w, ...) {
    char buffer[1024];
    va_list args;
    va_start (args, w);
    vsprintf (buffer, w, args);
    va_end (args);
    __noterm_msg = std::string(c) + "\n" + std::string(buffer);
    longjmp(__noterm_env, 1);
}

char* ExecF() {
    if (!setjmp(__noterm_env)) {
        __noterm_ftFunc();
        return nullptr;
    }
    else {
        return &__noterm_msg[0];
    }
}

char* ExecC() {
	try {
		__noterm_cFunc();
		return nullptr;
	}
	catch (char* err) {
		__noterm_msg = std::string(err);
	}
	catch (const std::exception &e) { __noterm_msg = e.what(); }
    catch (const std::string    &e) { __noterm_msg = e; }
    catch (const char           *e) { __noterm_msg = e; }
	catch (...) { __noterm_msg = "Unknown exception ocurred!"; }
	
	return &__noterm_msg[0];
}