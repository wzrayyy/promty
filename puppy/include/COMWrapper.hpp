#pragma once

#include <string>
#include <windows.h>


class COMWrapper {
  protected:
    void Raise(const HRESULT hr, const char *ctx) const;
    void Release(const void *cls) const;

  public:
    virtual const char *classname() const = 0;
};
