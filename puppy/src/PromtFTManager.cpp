#include "PromtFTManager.hpp"

#include "PromtCtlDirection.hpp"
#include "PromtFileTranslator.hpp"
#include <windows.h>

PromtFTManager::PromtFTManager() {
    void *p_unknown = nullptr;

    auto hr = CoCreateInstance(CLSID_PromtFTManager, nullptr, 0x17u, IID_Inknown2, &p_unknown);
    Raise(hr, "CoCreateInstance");

    hr = (**(HRESULT(__stdcall ***)(LPVOID, const GUID *, LPVOID *)) p_unknown)(p_unknown, &IID_IPromtFTManager, &mInstance);
    Raise(hr, "Create PromtFTManager instance");

    Release(p_unknown);
}

PromtFileTranslator PromtFTManager::Translator(PromtFTManager::FileType ft, PromtCtlDirection &dir) const {
    void *p_translator = nullptr;
    VARIANTARG varg{.vt=2, .iVal=(short)ft};
    HRESULT hr = (*(HRESULT(__thiscall **)(void *, void *, VARIANTARG, VARIANTARG, void *))(*(DWORD *) mInstance + 48))(
        mInstance, mInstance, varg, varg, &p_translator);
    Raise(hr, "Create file translator");
    return PromtFileTranslator(p_translator, dir.m_instance);
};

PromtFTManager::~PromtFTManager() {
    Release(mInstance);
}
