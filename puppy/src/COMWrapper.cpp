#include "COMWrapper.hpp"

#include <cstdio>
#include <stdexcept>
#include <windows.h>

void COMWrapper::Raise(const HRESULT hr, const char *const ctx) const {
    if (hr != 0) {
        char msg[128];
        snprintf(msg, 128, "[%s] Non-zero HRESULT value: 0x%x at: %s\n", classname(), hr, ctx);
        printf(msg);
        throw std::runtime_error(msg);
    }
}

void COMWrapper::Release(const void *const cls) const {
    if (cls) (void) (*(int(__thiscall **)(const void *, const void *))(*(DWORD *) cls + 8))(cls, cls);
}
