#include <iostream>
#include <string>
#include <vector>
#include <mutex>

std::vector<void*> __stdio_buf;
std::mutex __stdio_lock;
extern "C" __EXPORT__ std::mutex* __stdio_plock = &__stdio_lock;
extern "C" __EXPORT__ void* __stdio_data = nullptr;
extern "C" __EXPORT__ int __stdio_count = 0;

extern "C" __EXPORT__ void __stdio_clear() {
    std::lock_guard<std::mutex> lock(__stdio_lock);
    for (int a = 0; a < __stdio_count; a++) {
        delete[]((char*)__stdio_buf[a*2]);
    }
    __stdio_buf.clear();
    __stdio_count = 0;
}

#define printf(s, ...) do {\
    std::lock_guard<std::mutex> lock(__stdio_lock);\
    __stdio_buf.push_back(this);\
    auto str = new char[500];\
    if (sprintf(str, s, __VA_ARGS__) < 0) {\
        std::cerr << "failed to print!" << std::endl;\
    }\
    __stdio_buf.push_back(str);\
    __stdio_data = __stdio_buf.data();\
    __stdio_count++;\
} while (0)
//    printf(("OBJ[" + std::to_string((uintptr_t)this) + "]" + std::string(s)).c_str(), __VA_ARGS__)