#include "spicybuiltins.h"

#include <variant>
#include <chrono>

namespace spicy {

// ======================= clock ==========================
ClockBuiltIn::ClockBuiltIn() : BuiltinFunc("clock") {}

size_t ClockBuiltIn::arity() {
    return 0;
}

SpicyObj ClockBuiltIn::run() {
    const std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
    return static_cast<double>(now.time_since_epoch().count());
}

// ======================== str ===========================
StrBuiltIn::StrBuiltIn() : BuiltinFunc("str") {}

size_t StrBuiltIn::arity() {
    return 1;
}

SpicyObj StrBuiltIn::run() {
    auto value = m_args[0];
    m_args.clear();
    return getObjString(value);
}

// ======================= sqrt ===========================
SqrtBuiltIn::SqrtBuiltIn()
    : BuiltinFunc("sqrt") {}

size_t SqrtBuiltIn::arity() {
    return 1;
}

SpicyObj SqrtBuiltIn::run() {
    auto& val = m_args[0];
    if (!std::holds_alternative<double>(val))
        return nullptr;
    auto valDbl = std::get<double>(val);
    m_args.clear();
    return std::sqrt(valDbl);
}

// ======================== len ===========================
LenBuiltIn::LenBuiltIn()
    : BuiltinFunc("len") {}

size_t LenBuiltIn::arity() {
    return 1;
}

SpicyObj LenBuiltIn::run() {
    auto& val = m_args[0];
    SpicyObj ret;
	if (std::holds_alternative<SpicyListSharedPtr>(val)) {
        ret = std::get<SpicyListSharedPtr>(val)->size();
    } else if (std::holds_alternative<std::string>(val)) {
        ret = static_cast<double>(std::get<std::string>(val).length());
    } else {
        ret = nullptr;
    }
    m_args.clear();
    return ret;
}

// ======================= front ==========================
FrontBuiltIn::FrontBuiltIn()
    : BuiltinFunc("front") {}

size_t FrontBuiltIn::arity() {
    return 1;
}

SpicyObj FrontBuiltIn::run() {
    auto& val = m_args[0];
    SpicyObj ret;
    if (std::holds_alternative<SpicyListSharedPtr>(val)) {
        ret = std::get<SpicyListSharedPtr>(val)->front();
    } else if (std::holds_alternative<std::string>(val)) {
        ret = std::string{ std::get<std::string>(val).front() };
    } else {
        ret = nullptr;
    }
    m_args.clear();
    return ret;
}

// ======================= back ===========================
BackBuiltIn::BackBuiltIn()
    : BuiltinFunc("front") {}

size_t BackBuiltIn::arity() {
    return 1;
}

SpicyObj BackBuiltIn::run() {
    auto& val = m_args[0];
    SpicyObj ret;
    if (std::holds_alternative<SpicyListSharedPtr>(val)) {
        ret = std::get<SpicyListSharedPtr>(val)->back();
    } else if (std::holds_alternative<std::string>(val)) {
        ret = std::string{ std::get<std::string>(val).back() };
    } else {
        ret = nullptr;
    }
    m_args.clear();
    return ret;
}
} // namespace spicy
