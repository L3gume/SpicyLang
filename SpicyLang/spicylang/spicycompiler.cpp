#include "spicycompiler.h"

#include <format>
#include <variant>

namespace spicy {

SpicyCompiler::SpicyCompiler(SpicyScanner scanner) : m_scanner(std::move(scanner)) {
    m_rules[TokenType::APPEND]          = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::APPEND_FRONT]    = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::LEFT_PAREN]      = { [&]() { this->grouping(); }, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::RIGHT_PAREN]     = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::LEFT_BRACE]      = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::RIGHT_BRACE]     = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::COMMA]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::DOT]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::MINUS]           = { [&]() { this->unary(); } , [&]() { this->binary(); }, Precedence::PREC_TERM };
    m_rules[TokenType::PLUS]            = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_TERM };
    m_rules[TokenType::SEMICOLON]       = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::SLASH]           = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_FACTOR };
    m_rules[TokenType::STAR]            = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_FACTOR };
    m_rules[TokenType::BANG]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::BANG_EQUAL]      = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::EQUAL]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::EQUAL_EQUAL]     = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::GREATER]         = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::GREATER_EQUAL]   = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::LESS]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::LESS_EQUAL]      = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::IDENTIFIER]      = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::STRING]          = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::NUMBER]          = { [&]() { this->number(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::AND]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::CLASS]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::ELSE]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FALSE]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FOR]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FUN]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::IF]              = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::NIL]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::OR]              = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::PRINT]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::RETURN]          = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::SUPER]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::THIS]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::TRUE]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::VAR]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::WHILE]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::ERROR]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::END_OF_FILE]     = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
}

Chunk SpicyCompiler::compile() {
    m_chunk = Chunk(); // reset the current chunk
    advance();
    expression();
    consume(TokenType::END_OF_FILE, "Expect EOF.");
    return m_chunk;
}

void SpicyCompiler::advance() {
    m_previous = m_current;
    while (true) {
        m_current = m_scanner.scanSingle();
        if (m_current.type != TokenType::ERROR) break;
        errorAtCurrent(m_current.lexeme);
    }
}

void SpicyCompiler::consume(TokenType type, const std::string& errMsg) {
    if (m_current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(errMsg);
}

// ===============================================================================================================================
// BYTECODE GENERATION FUNCTIONS
// ===============================================================================================================================

void SpicyCompiler::emitByte(uint8_t byte) {
    m_chunk.appendByte(byte, m_previous.line);
}

void SpicyCompiler::emitByte(Chunk::OpCode byte) {
    emitByte(static_cast<uint8_t>(byte));
}

void SpicyCompiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void SpicyCompiler::emitReturn() {
    emitByte(Chunk::OpCode::OP_RETURN);
}

void SpicyCompiler::emitConstant(SpicyObj constant) {
    emitBytes(static_cast<uint8_t>(Chunk::OpCode::OP_CONSTANT), makeConstant(constant));
}

uint8_t SpicyCompiler::makeConstant(SpicyObj constant) {
    const auto idx = m_chunk.addConstant(constant);
    if (idx > std::numeric_limits<uint8_t>::max()) {
        error("Too many constants in one chunk");
        return 0;
    }
    return idx;
}

// ===============================================================================================================================
// ERROR FUNCTIONS
// ===============================================================================================================================

void SpicyCompiler::error(const std::string& msg) {
    errorAt(m_previous, msg);
}

void SpicyCompiler::errorAtCurrent(const std::string& msg) {
    errorAt(m_current, msg);
}

void SpicyCompiler::errorAt(const Token& token, const std::string& msg) {
    if (m_panicMode) return;
    m_panicMode = true;
    std::cout << std::format("[line {}] Error", token.line);
    if (token.type == TokenType::END_OF_FILE) std::cout << " at end";
    //else if (token.type == TokenType::)
    else std::cout << std::format(" at '{}'", token.lexeme);
    std::cout << std::format(": {}\n", msg);
    m_hadError = true;
}


// ===============================================================================================================================
// PARSER FUNCTIONS
// ===============================================================================================================================

void SpicyCompiler::parsePrecedence(Precedence prec) {
    advance();
    const auto& prefixRuleOpt = m_rules[m_previous.type].prefix;
    if (!prefixRuleOpt.has_value()) {
        error("Expect expression.");
        return;
    }
    const auto& prefixRule = prefixRuleOpt.value();
    prefixRule();
    
    while (prec <= m_rules[m_current.type].precedence) {
        advance();
        const auto& infixRuleOpt = m_rules[m_previous.type].infix;
        if (!infixRuleOpt.has_value()) {
            error("Expect infix expression.");
            return;
        }
        const auto& infixRule = infixRuleOpt.value();
        infixRule();
    }
}

void SpicyCompiler::expression() {
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void SpicyCompiler::number() {
    emitConstant(std::get<double>(m_previous.literal.value()));
}

void SpicyCompiler::grouping() {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void SpicyCompiler::unary() {
    const auto opType = m_previous.type;
    parsePrecedence(Precedence::PREC_UNARY);
    switch (opType) {
    case TokenType::MINUS: emitByte(Chunk::OpCode::OP_NEGATE); break;
    // TODO: inc, dec, bang
    default: return;
    }
}

void SpicyCompiler::binary() {
    const auto opType = m_previous.type;
    const auto& rule = m_rules[opType];
    parsePrecedence((Precedence)((int)rule.precedence + 1)); // TODO: that's bad
    
    switch (opType) {
    case TokenType::PLUS:   emitByte(Chunk::OpCode::OP_ADD); break;
    case TokenType::MINUS:  emitByte(Chunk::OpCode::OP_SUBTRACT); break;
    case TokenType::STAR:   emitByte(Chunk::OpCode::OP_MULTIPLY); break;
    case TokenType::SLASH:  emitByte(Chunk::OpCode::OP_DIVIDE); break;
    // TODO: append
    default: return;
    }
}

}
