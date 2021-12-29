// SpicyLang.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "spicylang/spicyinterpreter.h"
#include "spicylang/spicycli.h"

int main(const int argc, const char* argv[]) {
    //if (argc == 1) {
    //    spicy::SpicyInterpreter interpreter("");
    //    std::cout << "== SpicyLang ==" << '\n';
    //    std::cout << "version 0.1\n\n";
    //    interpreter.repl();
    //} else {
    //    auto scriptPath = std::string{ argv[1] };
    //    spicy::SpicyInterpreter interpreter(scriptPath);
    //    if (argc >= 3 && argv[2] == "--bytecode") {
    //        interpreter.runByteCode();
    //    } else {
    //        interpreter.runTreeWalk();
    //    }
    //}
    const auto config = spicy::parseArguments(argc, argv);
    if (config.is_repl) {
        spicy::SpicyInterpreter interpreter("");
        std::cout << "== SpicyLang ==" << '\n';
        std::cout << "version 0.1\n\n";
        interpreter.repl();
    } else if (!config.help) {
        spicy::SpicyInterpreter interpreter(config.script_path);
        if (config.treewalk) interpreter.runTreeWalk();
        else interpreter.runByteCode();
    } else {
        std::cout << "USAGE: TODO" << '\n';
    }
    return 0;
}
