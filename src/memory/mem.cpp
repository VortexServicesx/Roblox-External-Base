#include "mem.h"

bool mem::attach(const std::string& proc_name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    
    if (Process32First(snap, &entry)) {
        do {
            if (proc_name == entry.szExeFile) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &entry));
    }
    CloseHandle(snap);
    
    if (!pid) return false;
    
    handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!handle) return false;
    
    HANDLE mod_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (mod_snap == INVALID_HANDLE_VALUE) return false;
    
    MODULEENTRY32 mod_entry{};
    mod_entry.dwSize = sizeof(mod_entry);
    
    if (Module32First(mod_snap, &mod_entry)) {
        if (proc_name == mod_entry.szModule) {
            base = (uintptr_t)mod_entry.modBaseAddr;
        }
    }
    CloseHandle(mod_snap);
    
    return base != 0;
}

std::string mem::read_str(uintptr_t addr) {
    if (!addr) return "";
    
    uint32_t len = read<uint32_t>(addr + 0x10);
    if (len == 0 || len > 1024) return "";
    
    if (len < 16) {
        char buffer[16]{};
        ReadProcessMemory(handle, (void*)addr, buffer, len, nullptr);
        return std::string(buffer, len);
    } else {
        uintptr_t str_ptr = read<uintptr_t>(addr);
        if (!str_ptr) return "";
        
        char* buffer = new char[len + 1]{};
        ReadProcessMemory(handle, (void*)str_ptr, buffer, len, nullptr);
        std::string result(buffer, len);
        delete[] buffer;
        return result;
    }
}
