#pragma once
#ifndef H_SPICYCLI
#define H_SPICYCLI

#include <iostream>
#include <format>
#include "parsers.h"

namespace spicy {
    
struct SpicyConfig {
    bool is_repl = false;
    bool dump_bytecode = false;
    bool dump_ast = false;
    bool help = false;
    bool treewalk = false;
    bool trace = false;
    std::string script_path = "";
    
    friend SpicyConfig operator+(const SpicyConfig& lhs, const SpicyConfig& rhs) {
        return SpicyConfig{
            .is_repl = lhs.is_repl || rhs.is_repl,
            .dump_bytecode = lhs.dump_bytecode || rhs.dump_bytecode,
            .dump_ast = lhs.dump_ast || rhs.dump_ast,
            .help = lhs.help || rhs.help,
            .treewalk = lhs.treewalk || rhs.treewalk,
            .trace = lhs.trace || rhs.trace,
            .script_path = rhs.script_path
        };
    }
};

namespace parsers {
    template<typename Result>
    using parser_t = std::function<parse_result_t<Result>(std::string)>;
    
    parser_t<SpicyConfig> match_flag(const std::string& flag, bool SpicyConfig::* ptr) {
        return fmap([=](std::string) {
                    auto conf = SpicyConfig{};
                    conf.*ptr = true;
                    return conf; 
                }, match_string(flag));
    }
    
    parser_t<SpicyConfig> parseFlag() {
        return    match_flag("repl", &SpicyConfig::is_repl)
                | match_flag("bytecode", &SpicyConfig::dump_bytecode)
                | match_flag("help", &SpicyConfig::help)
                | match_flag("treewalk", &SpicyConfig::treewalk)
                | match_flag("trace", &SpicyConfig::trace)
                | match_flag("ast", &SpicyConfig::dump_ast);
    }
    
    parser_t<SpicyConfig> parseArg() {
        return (exactly_n(match_char('-'), std::monostate{}, 2, [](auto m, auto) { return m; })
            < parseFlag())
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
        return {};
    }
    const auto& config = res.value().first;
    //std::cout << std::format("repl: {}\ndump_bytecode: {}\nhelp: {}\ntreewalk: {}\ntrace: {}\nast: {}\nscript: {}\n",
    //    config.is_repl, 
    //    config.dump_bytecode, 
    //    config.help, 
    //    config.treewalk, 
    //    config.trace, 
    //    config.dump_ast, 
    //    config.script_path);
    
    return config;
}

}

#endif
