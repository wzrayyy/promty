#pragma once

#include <windows.h>
#include "COMWrapper.hpp"
#include "PromtCtlDirection.hpp"

// GUIDs
const GUID CLSID_PromtCtlDocument = {0xbdf7dc81, 0xdaae, 0x11d5, {0xb3, 0x92, 0x0, 0xe0, 0x29, 0x42, 0x4b, 0x73}};
const GUID CLSID_PromtSvrDirectionsIP = {0x212aae21, 0xdaa2, 0x11d5, {0xb3, 0x92, 0x0, 0xe0, 0x29, 0x42, 0x4b, 0x73}};
const GUID IID_IPromtCtlDocument = {0xfd163702, 0x2c47, 0x11d6, {0xb3, 0x92, 0x0, 0xe0, 0x29, 0x42, 0x4b, 0x73}};
const GUID IID_IPromtSvrDirections = {0x67c8f1c1, 0x2c4b, 0x11d6, {0xb3, 0x92, 0x0, 0xe0, 0x29, 0x42, 0x4b, 0x73}};
const GUID IID_IPromtSvrDirectionsIP = {0x67c8f1cf, 0x2c4b, 0x11d6, {0xb3, 0x92, 0x0, 0x0e0, 0x29, 0x42, 0x4b, 0x73}};

class PromtCtlDocument : COMWrapper {
  public:
    enum class Direction { kEngRus = 0x20001, kRusEng = 0x10002 };

  private:
    const BSTR k_client = SysAllocString(L"MG3");
    const BSTR k_common = SysAllocString(L"Common");
    static constexpr char k_registry_key[] = "Software\\PROject MT\\Kernel6MG\\Directions";
    static constexpr char k_classname[] = "PromtCtlDocument";
    void *m_instance = nullptr;
    Direction m_direction;

  public:
    PromtCtlDocument();
    ~PromtCtlDocument();
    void direction(const Direction direction);
    PromtCtlDirection direction() const;
    const char* classname() const override;
};
