#include <iostream>
#include <cstring>

typedef void (*emptyFunc) (void);
extern "C" __declspec(dllexport) void gcc_safe_call (emptyFunc func, char* buf) {
    try { 
        func();
        std::cout << "(gcc handler) function success" << std::endl;
    }
    catch (char* c) {
        std::cout << "(gcc handler) function failed with char*" << std::endl;
        strcpy(buf, c);
    }
    catch (...) {
        std::cout << "(gcc handler) function failed with other" << std::endl;
        strcpy(buf, "Some other error happened!");
    }
}