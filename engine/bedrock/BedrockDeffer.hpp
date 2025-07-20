#pragma once

#include "BedrockCommon.hpp"

#include <functional>

struct Deffer
{
    explicit Deffer(std::function<void()> function) : function(std::move(function)) {}
    ~Deffer()
    {
        function();
    }
    std::function<void()> function;
};
#define MFA_DEFFER(function) Deffer MFA_UNIQUE_NAME(deffer)(function)