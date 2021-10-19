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

std::string ClockBuiltIn::getFuncName() {
    return m_funcName;
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

std::string StrBuiltIn::getFuncName() {
    return m_funcName;
}

// ======================= sqrt ===========================
SqrtBuiltIn::SqrtBuiltIn()
    : BuiltinFunc("sqrt") {}

size_t SqrtBuiltIn::arity() {
    return 1;
}

SpicyObj SqrtBuiltIn::run() {
    auto val = m_args[0];
    m_args.clear();
    if (!std::holds_alternative<double>(val))
        return nullptr;
    auto valDbl = std::get<double>(val);
    return std::sqrt(valDbl);
}

std::string SqrtBuiltIn::getFuncName() {
    return m_funcName;
}

} // namespace spicy
