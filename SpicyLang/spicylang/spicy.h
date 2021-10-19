#pragma once
#ifndef H_SPICY
#define H_SPICY

#include <string>
#include <iostream>
#include "types.h"

namespace spicy {
void error(int line, const std::string& msg);
void error(Token token, const std::string& msg);
void report(int line, const std::string& where, const std::string& msg);
void dbgPrint(const std::string& msg);
void runtimeError(const RuntimeError& err);
}

#endif
