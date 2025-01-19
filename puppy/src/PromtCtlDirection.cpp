#include "PromtCtlDirection.hpp"

#include <string>
#include <windows.h>

const char *PromtCtlDirection::classname() const {
    return m_classname;
}

std::wstring PromtCtlDirection::Translate(const std::wstring_view src) const {
    BSTR src_b = SysAllocString(std::wstring(src).c_str());
    BSTR dest = nullptr;

    auto hr = (*(HRESULT(__stdcall **)(const void *, BSTR, VARIANTARG, BSTR *))(*(DWORD *) m_instance + 112))(m_instance, src_b, {}, &dest);
    Raise(hr, "Translate");

    // I want none of that callee managed memory crap
    std::wstring result(dest);
    SysFreeString(src_b);
    SysFreeString(dest);

    return result;
}

PromtCtlDirection::~PromtCtlDirection() {
    Release(m_instance);
}
