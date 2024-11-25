#include "PromtCtlDocument.hpp"

#include "PromtCtlDirection.hpp"
#include <windows.h>

PromtCtlDocument::PromtCtlDocument() {
    void *p_unknown = nullptr;
    void *p_svr_directions = nullptr;
    void *p_svr_directions_ip = nullptr;
    HRESULT hr = 0;

    hr = CoGetClassObject(CLSID_PromtSvrDirectionsIP, CLSCTX_INPROC_SERVER, nullptr, IID_IPromtSvrDirectionsIP, &p_svr_directions_ip);
    Raise(hr, "CoGetClassObject");
    hr = (*(HRESULT(__stdcall **)(void *, unsigned int, const char *))(*(DWORD *) p_svr_directions_ip + 20))(p_svr_directions_ip,
                                                                                                             0x80000002, k_registry_key);
    Raise(hr, "SetRegistryKey");

    hr = (*(HRESULT(__thiscall **)(void *, void *, DWORD, const GUID *, void *))(*(DWORD *) p_svr_directions_ip + 12))(
        p_svr_directions_ip, p_svr_directions_ip, 0, &IID_IPromtSvrDirections, &p_svr_directions);
    Raise(hr, "spCF->CreateInstance");

    hr = (*(HRESULT(__stdcall **)(void *, const wchar_t *, const wchar_t *, VARIANTARG))(*(DWORD *) p_svr_directions + 28))(
        p_svr_directions, k_client, k_common, {});
    Raise(hr, "svr_directions->Initialize");

    hr = CoCreateInstance(CLSID_PromtCtlDocument, nullptr, 0x17u, IID_IUnknown, &p_unknown);
    Raise(hr, "CoCreateInstance");
    hr = OleRun((LPUNKNOWN) p_unknown);
    Raise(hr, "OleRun");

    hr = (**(HRESULT(__stdcall ***)(void *, const GUID *, void *)) p_unknown)(p_unknown, &IID_IPromtCtlDocument, &m_instance);
    Raise(hr, "CreatePromtCtlDocument");
    Release(p_unknown);

    VARIANTARG vararg{};
    vararg.vt = 9;
    *((DWORD *) &vararg + 2) = (DWORD) p_svr_directions;
    hr = (*(HRESULT(__stdcall **)(void *, const wchar_t *, int, int, int, int, VARIANTARG))(*(DWORD *) m_instance + 28))(
        m_instance, k_client, 0xa, 0, 0x80020004, 0, vararg);
    Raise(hr, "ctl_document->Initialize");

    Release(p_svr_directions_ip);
    Release(p_svr_directions);

    direction(Direction::kEngRus);
};

PromtCtlDocument::~PromtCtlDocument() {
    Release(m_instance);
    SysFreeString(k_client);
    SysFreeString(k_common);
}

void PromtCtlDocument::direction(const Direction direction) {
    if (m_direction != direction) {
        auto hr = (*(HRESULT(__stdcall **)(const void *, Direction))(*(DWORD *) m_instance + 52))(m_instance, direction);
        Raise(hr, "set_direction");
        m_direction = direction;
    }
}

PromtCtlDirection PromtCtlDocument::direction() const {
    void *ctldir = nullptr;
    auto hr = (*(HRESULT(__stdcall **)(const void *, const void *))(*(DWORD *) m_instance + 44))(m_instance, &ctldir);
    Raise(hr, "get_direction");
    return PromtCtlDirection(ctldir);
}

const char *PromtCtlDocument::classname() const {
    return k_classname;
};
