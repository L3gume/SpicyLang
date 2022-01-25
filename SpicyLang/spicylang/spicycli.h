#pragma once
#ifndef H_SPICYCLI
#define H_SPICYCLI

#include <iostream>
#include "parsers.h"

namespace spicy {
    
struct SpicyConfig {
    bool is_repl = false;
    bool dump_bytecode = false;
    //bool dump_ast = false;
    bool help = false;
    bool treewalk = false;
    bool trace = false;
    std::string script_path = "";
    
    friend SpicyConfig operator+(const SpicyConfig& lhs, const SpicyConfig& rhs) {
        return SpicyConfig{
            lhs.is_repl || rhs.is_repl,
            lhs.dump_bytecode || rhs.dump_bytecode,
            lhs.help || rhs.help,
            lhs.treewalk || rhs.treewalk,
            lhs.trace || rhs.trace,
            rhs.script_path
        };
    }
};

namespace parsers {
    template<typename Result>
    using parser_t = std::function<parse_result_t<Result>(std::string)>;
    
    parser_t<SpicyConfig> parseFlag() {
        return fmap([](std::string flag) {
            SpicyConfig conf;
            if (flag == "repl") conf.is_repl = true;
            if (flag == "bytecode") conf.dump_bytecode = true;
            if (flag == "help") conf.help = true;
            if (flag == "treewalk") conf.treewalk = true;
            if (flag == "trace") conf.trace = true;
            return conf;
            },
            match_string("repl")
                | match_string("bytecode")
                | match_string("help")
                | match_string("treewalk")
                | match_string("trace"));
    }
    
    parser_t<SpicyConfig> parseArg() {
        return exactly_n(
            match_char('-'), std::monostate{}, 2, [](auto m, auto) { return m; }
            )
            < some(parseFlag(), SpicyConfig{},
            [](SpicyConfig acc, SpicyConfig conf) {
                return acc + conf;
            })
            | fmap(
                [](std::string path) {
                    SpicyConfig conf;
                    conf.script_path = std::move(path);
                    return conf;
                }, 
                many(one_of_chars("._abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/\\"),
                    std::string{}, [](std::string acc, char c) { return acc + c; }
                ));
    }
    
    parse_result_t<SpicyConfig> parseArgs(const std::string& input) {
        return many(tokenize(parseArg()), SpicyConfig{}, [](SpicyConfig acc, SpicyConfig conf) { return acc + conf; })(input);
    }
}

static SpicyConfig parseArguments(const int argc, const char* argv[]) {
    auto args = std::string{};
    for (auto i = 1; i < argc; ++i) {
        args += argv[i];
    }
    auto res = parsers::parseArgs(args);
    if (!res.has_value()) {
        std::cerr << "failed to parse arguments." << '\n';
        return { false, false, true, false, false, "" };
    }
    return res.value().first;
}

}

#endif
