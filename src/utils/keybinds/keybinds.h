#pragma once
#include <Windows.h>

inline bool key_pressed(int vk) {
    return (GetAsyncKeyState(vk) & 1) != 0;
}

inline bool key_down(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}
