#include "spicy.h"

#include <iostream>
#include <exception>
#include <format>

#include "types.h"

void spicy::error(int line, const std::string& msg) {
    spicy::report(line, "", msg);
}

void spicy::report(int line, const std::string& where, const std::string& msg) {
    std::cerr << std::format("[line {}] Error {}: {}\n", line, where, msg);
}

void spicy::error(Token token, const std::string &msg) {
    if (token.type == TokenType::END_OF_FILE) {
        report(token.line, " at end", msg);
    } else {
        report(token.line, "at '" + token.lexeme + "'", msg);
    }
}

void spicy::dbgPrint(const std::string &msg) {
    if (false)
        std::cout << msg << '\n';
}

void spicy::runtimeError(const RuntimeError& err) {
    error(err.token(), err.what());
}
