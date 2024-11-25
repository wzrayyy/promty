#pragma once

#include <string>
#include <windows.h>
#include "COMWrapper.hpp"

class PromtCtlDirection : COMWrapper {
  public:
    const void *m_instance = nullptr;
    static constexpr char m_classname[] = "PromtCtlDirection";

  public:
    PromtCtlDirection() = delete;
    explicit PromtCtlDirection(const void *instance) : m_instance(instance){};
    ~PromtCtlDirection();
    std::wstring Translate(const std::wstring_view src) const;
    const char *classname() const override;
};
