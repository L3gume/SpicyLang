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
};

class StrBuiltIn : public BuiltinFunc {
public:
    StrBuiltIn();

    size_t arity() override;
    SpicyObj run() override;
};

class SqrtBuiltIn : public BuiltinFunc {
public:
    SqrtBuiltIn();

    size_t arity() override;
    SpicyObj run() override;
};

class LenBuiltIn : public BuiltinFunc {
public:
    LenBuiltIn();
    
    size_t arity() override;
    SpicyObj run() override;
};

class FrontBuiltIn : public BuiltinFunc {
public:
    FrontBuiltIn();
    
    size_t arity() override;
    SpicyObj run() override;
};

class BackBuiltIn : public BuiltinFunc {
public:
    BackBuiltIn();
    
    size_t arity() override;
    SpicyObj run() override;
};

} // namespace spicy

#endif // SPICY_SPICYBUILTINS_H
