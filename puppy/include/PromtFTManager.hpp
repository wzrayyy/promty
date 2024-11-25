#pragma once

#include "COMWrapper.hpp"
#include "PromtCtlDirection.hpp"
#include "PromtFileTranslator.hpp"

const GUID CLSID_PromtFTManager = {0xdcc0c930, 0x25cc, 0x4043, {0xb0, 0xf6, 0xb1, 0xed, 0x7a, 0xca, 0xd7, 0x1b}};
const GUID IID_IPromtFTManager = {0xe64125d8, 0x14dc, 0x4993, {0x8f, 0x84, 0x8b, 0xf4, 0x94, 0xc0, 0x94, 0xb}};
const GUID IID_Inknown2 = {0x0, 0x0, 0x0, {0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46}};

class PromtFTManager : COMWrapper {
  private:
    void *mInstance = nullptr;
    static constexpr char mClassname[] = "PromtFTManager";

  public:
    enum class FileType { kHTML = 130, kRTF = 32 };

  public:
    PromtFTManager();
    ~PromtFTManager();
    PromtFileTranslator Translator(PromtFTManager::FileType ft, PromtCtlDirection &dir) const;
    const char* classname() const override { return mClassname; };
};
