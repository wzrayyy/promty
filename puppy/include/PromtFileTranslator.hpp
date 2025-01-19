#pragma once

#include <string_view>
#include "COMWrapper.hpp"

class PromtFileTranslator : COMWrapper {
  friend class PromtFTManager;

  private:
    const void *m_instance = nullptr;
    const void *m_direction = nullptr;
    static constexpr char k_classname[] = "PromtFileTranslator";

  protected:
    explicit PromtFileTranslator(const void *instance, const void *direction) : m_instance(instance), m_direction(direction) {};

  public:
    PromtFileTranslator() = delete;
    ~PromtFileTranslator();
    void Translate(const std::string_view src, const std::string_view dest) const;
    const char *classname() const override {
        return k_classname;
    };
};
