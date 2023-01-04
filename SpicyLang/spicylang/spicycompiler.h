#pragma once
#ifndef H_SPICYCOMPILER
#define H_SPICYCOMPILER

#include <functional>
#include <map>
#include <optional>
#include "spicyscanner.h"
#include "vmtypes.h"

namespace spicy {

enum class Precedence {
    PREC_NONE = 0,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_APPEND,     // <- ->
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . () []
    PREC_PRIMARY
};

using ParseFn = std::optional<std::function<void(bool)>>;
struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence = Precedence::PREC_NONE;
};

struct Local {
    Token name;
    int32_t depth;
};

class SpicyCompiler {
    
public:
    explicit SpicyCompiler(SpicyScanner scanner);
    
    [[nodiscard]] 
    auto compile() -> Chunk;
    
private:
    void advance();
    void consume(TokenType type, const std::string& errMsg);
    bool match(TokenType type);
    bool check(TokenType type);

    void emitByte(uint8_t byte);
    void emitByte(Chunk::OpCode byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitBytes(Chunk::OpCode byte1, uint8_t byte2);
    void emitBytes(Chunk::OpCode byte1, Chunk::OpCode byte2);
    void emitReturn();
    void emitConstant(SpicyObj constant);
    void emitLoop(uint32_t loopStart);
    uint16_t emitJump(Chunk::OpCode byte);
    
    void patchJump(uint16_t offset);

    [[nodiscard]] uint8_t makeConstant(SpicyObj constant);
    
    void error(const std::string& msg);
    void errorAtCurrent(const std::string& msg);
    void errorAt(const Token& token, const std::string& msg);
    void synchronize();
    
    void parsePrecedence(Precedence prec);
    
    void declaration();
    void varDeclaration();
    void statement();
    void block();
    void printStatement();
    void ifStatement();
    void whileStatement();
    void forStatement();
    void expressionStatement();
    
    void expression();
    void variable(bool canAssign);
    void namedVariable(const spicy::Token& name, bool canAssign);
    void number();
    void literal();
    void grouping();
    void unary();
    void binary();
    void string();
    void and_();
    void or_();
    
    void beginScope();
    void endScope();
    
    [[nodiscard]] 
    uint8_t parseVar(const std::string& errMsg);
    [[nodiscard]] 
    uint8_t identifierConst(const spicy::Token& name);
    void declareVariable();
    void defineVariable(uint8_t global);
    void addLocal(const spicy::Token& name);
    void markInitialized();
    uint32_t resolveLocal(const spicy::Token& name);
    
private: 
    // Scanner and output
    SpicyScanner m_scanner;
    Chunk m_chunk;

    // Parser;
    Token m_previous;
    Token m_current;
    std::map<TokenType, ParseRule> m_rules;
    
    // Scoping
    std::vector<Local> m_locals;
    uint32_t m_scopeDepth = 0u;
    
    // Util
    bool m_hadError = false;
    bool m_panicMode = false;
};
} // namespace spicy;

#endif H_SPICYCOMPILER
