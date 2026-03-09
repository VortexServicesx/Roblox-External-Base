#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

class mem {
public:
    mem() = default;
    ~mem() { if (handle) CloseHandle(handle); }
    
    bool attach(const std::string& proc_name);
    
    HANDLE handle = nullptr;
    
    template <typename T>
    T read(uintptr_t addr) {
        T buffer{};
        ReadProcessMemory(handle, (void*)addr, &buffer, sizeof(T), nullptr);
        return buffer;
    }
    
    template <typename T>
    void write(uintptr_t addr, T value) {
        WriteProcessMemory(handle, (void*)addr, &value, sizeof(T), nullptr);
    }
    
    std::string read_str(uintptr_t addr);
    uintptr_t get_base() { return base; }
    
private:
    uintptr_t base = 0;
    DWORD pid = 0;
};

inline mem* g_mem = nullptr;
