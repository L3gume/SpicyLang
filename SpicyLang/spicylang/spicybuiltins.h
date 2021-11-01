#pragma once
#ifndef SPICY_SPICYBUILTINS_H
#define SPICY_SPICYBUILTINS_H

#include "spicyobjects.h"

namespace spicy {

class ClockBuiltIn : public BuiltinFunc {
public:
    ClockBuiltIn();

    size_t arity() override;
    SpicyObj run() override;
    std::string getFuncName() override;
};

class StrBuiltIn : public BuiltinFunc {
public:
    StrBuiltIn();

    size_t arity() override;
    SpicyObj run() override;
    std::string getFuncName() override;
};

class SqrtBuiltIn : public BuiltinFunc {
public:
    SqrtBuiltIn();

    size_t arity() override;
    SpicyObj run() override;
    std::string getFuncName() override;
};

class LenBuiltIn : public BuiltinFunc {
public:
    LenBuiltIn();
    
    size_t arity() override;
    SpicyObj run() override;
    std::string getFuncName() override;
};

} // namespace spicy

#endif // SPICY_SPICYBUILTINS_H
