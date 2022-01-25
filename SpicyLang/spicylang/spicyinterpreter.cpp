#include "spicyinterpreter.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <optional>
#include <format>

#include "spicyscanner.h"
#include "spicyparser.h"
#include "spicyastprinter.h"
#include "spicyobjects.h"
#include "spicycompiler.h"
#include "spicyvm.h"

namespace spicy {

SpicyInterpreter::SpicyInterpreter(const std::string& scriptPath)
    : m_sScriptPath(scriptPath) {}

void SpicyInterpreter::runTreeWalk() {
    try {
        loadScript();
        SpicyScanner scanner(m_sRawScript);
        SpicyParser parser(scanner.scanTokens());
        m_program = std::move(parser.parseProgram());
        interpret();
    }  catch (const SpicyParser::ParseError& err) {
        std::cerr << "Script failed to parse." << '\n';
    }
}

void SpicyInterpreter::runByteCode() {
    try {
        loadScript();
        SpicyScanner scanner(m_sRawScript);
        SpicyParser parser(scanner.scanTokens());
        m_program = std::move(parser.parseProgram());
        interpretByteCode();
    }  catch (const SpicyParser::ParseError& err) {
        std::cerr << "Script failed to parse." << '\n';
    }
}

void SpicyInterpreter::repl() {
    auto line = std::string{};
    SpicyVM vm(true);
    getNextLine(line);
    while (line != "exit();") {
        SpicyScanner scanner(line);
        SpicyCompiler compiler(scanner);
        vm.execute(compiler.compile());
        getNextLine(line);
    }
}

void SpicyInterpreter::replLegacy() {
    auto line = std::string{};
    eval::SpicyEvaluator evaluator(true);
    eval::SpicyResolver resolver(evaluator);
    while (line != "exit();") {
        SpicyScanner scanner(line);
        SpicyParser parser(scanner.scanTokens());
        auto&& parsed = parser.parseProgram();
        m_program.insert(m_program.end(), std::make_move_iterator(parsed.begin()), std::make_move_iterator(parsed.end()));
        try {
            resolver.resolve(m_program.back());
            evaluator.execStmt(m_program.back());
            std::cout << std::format("{}\n", getObjString(evaluator.getLastObj()));
        } catch (RuntimeError err) {
            runtimeError(err);
        }
        getNextLine(line);
    }
}

bool SpicyInterpreter::hadError() {
    return m_hadError;
}

bool SpicyInterpreter::hadRuntimeError() {
    return m_hadRuntimeError;
}

void SpicyInterpreter::interpret() {
    try {
        eval::SpicyEvaluator evaluator;
        eval::SpicyResolver resolver(evaluator);
        resolver.resolve(m_program);
        evaluator.execStmts(m_program);
    } catch (RuntimeError err) {
        runtimeError(err);
    }
}

void SpicyInterpreter::interpretByteCode() {
}

void SpicyInterpreter::loadScript() {
    std::ifstream ifs(m_sScriptPath.c_str());
    m_sRawScript = std::string{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

void SpicyInterpreter::getNextLine(std::string& line) {
    line.clear();
    std::cout << ">> ";
    std::getline(std::cin, line);
    if (!line.ends_with(';'))
        line += ";";
}

} // namespace spicy
