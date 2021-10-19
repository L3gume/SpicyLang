// SpicyLang.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "spicylang/spicyinterpreter.h"

int main(const int argc, const char* argv[]) {
    spicy::SpicyInterpreter interpreter("");
    std::cout << "== SpicyLang ==" << '\n';
    std::cout << "version 0.1\n\n";
    interpreter.repl();
    return 0;
}
