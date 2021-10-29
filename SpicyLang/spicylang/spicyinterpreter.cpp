#include "spicyinterpreter.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <optional>

#include "spicyscanner.h"
#include "spicyparser.h"
#include "spicyastprinter.h"
#include "spicyobjects.h"

namespace spicy {

SpicyInterpreter::SpicyInterpreter(const std::string& scriptPath)
    : m_sScriptPath(scriptPath) {}

void SpicyInterpreter::run() {
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

void SpicyInterpreter::repl() {
    auto line = std::string{};
    eval::SpicyEvaluator evaluator;
    eval::SpicyResolver resolver(evaluator);
    std::cout << ">> ";
    std::getline( std::cin, line );
    if (!line.ends_with(';'))
        line += ";";
    while (line != "exit();") {
        SpicyScanner scanner(line);
        SpicyParser parser(scanner.scanTokens());
        for (auto& stmt : parser.parseProgram()) {
            m_program.emplace_back(std::move(stmt));
        }
        try {
            resolver.resolve(m_program.back());
            const auto& obj = evaluator.execStmt(m_program.back());
            if (obj.has_value())
                std::cout << getObjString(obj.value()) << '\n';
        } catch (RuntimeError err) {
            runtimeError(err);
        }
        line.clear();
        std::cout << ">> ";
        std::getline( std::cin, line );
    }
}   

bool SpicyInterpreter::hadError() {
    return m_hadError;
}

bool SpicyInterpreter::hadRuntimeError() {
    return m_hadRuntimeError;
}

//void SpicyInterpreter::interpret(const ast::ExprPtrVariant& ast) {
//    try {
//        const auto& val = m_evaluator.evalExpr(ast);
//        std::cout << getObjString(val) << '\n';
//    } catch (RuntimeError err) {
//        m_hadRuntimeError = true;
//        spicy::runtimeError(err);
//    }
//}
//
//void SpicyInterpreter::interpret(const ast::StmtPtrVariant &ast) {
//    try {
//        m_resolver.resolve(ast);
//        m_evaluator.execStmt(ast);
//    } catch (RuntimeError err) {
//        runtimeError(err);
//    }
//}

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

void SpicyInterpreter::loadScript() {
    std::ifstream ifs(m_sScriptPath.c_str());
    m_sRawScript = std::string{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

} // namespace spicy
