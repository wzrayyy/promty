#include "PromtFileTranslator.hpp"
#include <string>

void PromtFileTranslator::Translate(const std::string_view source_path, const std::string_view dest_path) const {
    HRESULT hr;

    BSTR src = SysAllocString(std::wstring(source_path.begin(), source_path.end()).c_str());
    BSTR dest = SysAllocString(std::wstring(dest_path.begin(), dest_path.end()).c_str());
    hr = (*(HRESULT(__stdcall **)(const void *, const void *, BSTR, BSTR, VARIANTARG))(*(DWORD *) m_instance + 80))(
        m_instance, m_direction, src, dest, {});
    Raise(hr, "yk");
    SysFreeString(src);
    SysFreeString(dest);
}

PromtFileTranslator::~PromtFileTranslator() {
    Release(m_instance);
}
