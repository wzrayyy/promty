#pragma once

#include <string_view>
#include "COMWrapper.hpp"

class PromtFileTranslator : COMWrapper {
  private:
    const void *mInstance = nullptr;
    const void *mDirection = nullptr;
    static constexpr char mClassName[] = "PromtFileTranslator";

  public:
    PromtFileTranslator() = delete;
    explicit PromtFileTranslator(const void *instance, const void *direction) : mInstance(instance), mDirection(direction){};
    ~PromtFileTranslator();
    void Translate(const std::string_view src, const std::string_view dest) const;
    const char* classname() const override {
        return mClassName;
    };
};
