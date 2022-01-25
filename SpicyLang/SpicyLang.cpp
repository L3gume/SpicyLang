// SpicyLang.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <format>
#include <iostream>
#include "spicylang/spicyinterpreter.h"
#include "spicylang/spicycli.h"

int main(const int argc, const char* argv[]) {
    const auto spicyLangHeader = []() {
        std::cout << "== SpicyLang ==" << '\n';
        std::cout << "version 0.1\n\n";
    };
    const auto usageMessage = [&spicyLangHeader]() {
        spicyLangHeader();
        std::cout << "USAGE: SpicyLang.exe options [path]" << '\n';
        std::cout << '\n' << "options:" << '\n';
        std::cout << "--repl\t\trepl loop" << '\n';
        std::cout << "--bytecode\tdump bytecode" << '\n';
        std::cout << "--treewalk\texecute using treewalk interpreter" << '\n';
        std::cout << "--trace\t\ttrace execution of the bytecode" << '\n';
        std::cout << "--help\t\tdisplay this message" << '\n';
    };
    const auto config = spicy::parseArguments(argc, argv);
    if (config.is_repl) {
        spicy::SpicyInterpreter interpreter("");
        spicyLangHeader();
        std::cout << std::format("Interpreter: {}\n\n", config.treewalk ? "treewalk" : "bytecode (WIP)");
        if (config.treewalk) {
            interpreter.replLegacy();
        } else {
            interpreter.repl();
        }
    } else if (!config.help) {
        spicy::SpicyInterpreter interpreter(config.script_path);
        if (config.treewalk) {
            interpreter.runTreeWalk();
        } else {
            interpreter.runByteCode();
        }
    } else {
        usageMessage();
    }
    return 0;
}
