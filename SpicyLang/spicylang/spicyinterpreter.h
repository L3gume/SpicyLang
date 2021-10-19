#pragma once
#ifndef H_SPICYINTERPRETER
#define H_SPICYINTERPRETER

#include <string>

#include "spicyscanner.h"
#include "spicyast.h"
#include "spicyeval.h"
#include "spicyresolver.h"

namespace spicy {

class SpicyInterpreter {
    std::string m_sRawScript;
    std::string m_sScriptPath;
    bool m_hadError = false;
    bool m_hadRuntimeError = false;

    ast::SpicyProgram m_program;
    //eval::SpicyEvaluator m_evaluator;
    //eval::SpicyResolver m_resolver;
public:
    SpicyInterpreter(const std::string& scriptPath);
    
    void run();
    void repl();

    bool hadError();
    bool hadRuntimeError();
    
private:
    //void interpret(const ast::ExprPtrVariant& ast);
    //void interpret(const ast::StmtPtrVariant& ast);
    void interpret();
    void interpretAndPrint(const std::vector<ast::StmtPtrVariant>& ast);
    void loadScript();
};
    
}

#endif

