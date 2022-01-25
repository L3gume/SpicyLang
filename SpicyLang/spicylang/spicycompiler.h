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

using ParseFn = std::optional<std::function<void()>>;
struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

class SpicyCompiler {
    SpicyScanner m_scanner;
    Chunk m_chunk;

    Token m_previous;
    Token m_current;
    std::map<TokenType, ParseRule> m_rules;
    bool m_hadError = false;
    bool m_panicMode = false;
public:
    explicit SpicyCompiler(SpicyScanner scanner);
    [[nodiscard]] Chunk compile();
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

    [[nodiscard]] uint8_t makeConstant(SpicyObj constant);
    
    void error(const std::string& msg);
    void errorAtCurrent(const std::string& msg);
    void errorAt(const Token& token, const std::string& msg);
    void synchronize();
    
    void parsePrecedence(Precedence prec);
    
    void declaration();
    void varDeclaration();
    void statement();
    void printStatement();
    void expressionStatement();
    
    void expression();
    void variable();
    void namedVariable(const spicy::Token& name);
    void number();
    void literal();
    void grouping();
    void unary();
    void binary();
    void string();
    
    [[nodiscard]] uint8_t parseVar(const std::string& errMsg);
    [[nodiscard]] uint8_t identifierConst(const spicy::Token& name);
    void defineVariable(uint8_t global);
};
} // namespace spicy;

#endif H_SPICYCOMPILER
