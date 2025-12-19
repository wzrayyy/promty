#include "COMWrapper.hpp"

#include <cstdio>
#include <windows.h>
#include <comdef.h>

void COMWrapper::Raise(const HRESULT hr, const char *const ctx) const {
    if (hr != 0) {
        char msg[1024];
        _com_error err(hr);
        snprintf(msg, 1024, "[%s] Error: %s (0x%lx) at: %s\n", classname(), err.ErrorMessage(), hr, ctx);
        printf("%s\n", msg);
        exit(1);
    }
}

void COMWrapper::Release(const void *const cls) const {
    if (cls) (void) (*(int(__thiscall **)(const void *, const void *))(*(DWORD *) cls + 8))(cls, cls);
}
