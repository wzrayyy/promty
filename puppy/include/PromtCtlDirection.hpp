#pragma once

#include <string>
#include <windows.h>
#include "COMWrapper.hpp"

class PromtCtlDirection : COMWrapper {
  friend class PromtCtlDocument;

  public:
    const void *m_instance = nullptr;
    static constexpr char m_classname[] = "PromtCtlDirection";

  protected:
    explicit PromtCtlDirection(const void *instance) : m_instance(instance){};

  public:
    PromtCtlDirection() = delete;
    ~PromtCtlDirection();
    std::wstring Translate(const std::wstring_view src) const;
    const char *classname() const override;
};
