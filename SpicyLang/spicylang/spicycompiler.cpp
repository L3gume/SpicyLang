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
    m_rules[TokenType::BANG]            = { [&]() { this->unary(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::BANG_EQUAL]      = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_EQUALITY };
    m_rules[TokenType::EQUAL]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::EQUAL_EQUAL]     = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_EQUALITY };
    m_rules[TokenType::GREATER]         = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_COMPARISON };
    m_rules[TokenType::GREATER_EQUAL]   = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_COMPARISON };
    m_rules[TokenType::LESS]            = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_COMPARISON };
    m_rules[TokenType::LESS_EQUAL]      = { std::nullopt, [&]() { this->binary(); }, Precedence::PREC_COMPARISON };
    m_rules[TokenType::IDENTIFIER]      = { [&]() { this->variable(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::STRING]          = { [&]() { this->string(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::NUMBER]          = { [&]() { this->number(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::AND]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::CLASS]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::ELSE]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FALSE]           = { [&]() { this->literal(); }, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FOR]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::FUN]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::IF]              = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::NIL]             = { [&]() { this->literal(); } , std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::OR]              = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::PRINT]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::RETURN]          = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::SUPER]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::THIS]            = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::TRUE]            = { [&]() { this->literal(); }, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::VAR]             = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::WHILE]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::ERROR]           = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
    m_rules[TokenType::END_OF_FILE]     = { std::nullopt, std::nullopt, Precedence::PREC_NONE };
}

Chunk SpicyCompiler::compile() {
    m_chunk = Chunk(); // reset the current chunk
    advance();
    //expression();
    ////consume(TokenType::END_OF_FILE, "Expect EOF.");
    //emitReturn();
    while (!match(TokenType::END_OF_FILE)) {
        declaration();
    }
    
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

bool SpicyCompiler::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool SpicyCompiler::check(TokenType type) {
    return m_current.type == type;
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

void SpicyCompiler::emitBytes(Chunk::OpCode byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void SpicyCompiler::emitBytes(Chunk::OpCode byte1, Chunk::OpCode byte2) {
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

void SpicyCompiler::synchronize() {
    m_panicMode = false;
    while (check(TokenType::END_OF_FILE)) {
        if (m_previous.type == TokenType::SEMICOLON) return;
        switch (m_current.type) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default: advance();
        }
    }
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

void SpicyCompiler::declaration() {
    if (match(TokenType::VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    
    if (m_panicMode) {
        synchronize();
    }
}

void SpicyCompiler::varDeclaration() {
    const auto global = parseVar("Expect variable name.");
    if (match(TokenType::EQUAL)) {
        expression();
    } else {
        emitByte(Chunk::OpCode::OP_NIL);
    }
    consume(TokenType::SEMICOLON, "Expect ';' after var declaration");
    defineVariable(global);
}

void SpicyCompiler::statement() {
    if (match(TokenType::PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}

void SpicyCompiler::printStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(Chunk::OpCode::OP_PRINT);
}

void SpicyCompiler::expressionStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emitByte(Chunk::OpCode::OP_POP);
}

void SpicyCompiler::expression() {
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void SpicyCompiler::variable() {
    namedVariable(m_previous);
}

void SpicyCompiler::namedVariable(const spicy::Token& name) {
    const auto arg = identifierConst(name);
    emitBytes(Chunk::OpCode::OP_GET_GLOBAL, arg);
}

void SpicyCompiler::number() {
    emitConstant(std::get<double>(m_previous.literal.value()));
}

void SpicyCompiler::literal() {
    switch (m_previous.type) {
    case TokenType::FALSE: emitByte(Chunk::OpCode::OP_FALSE); break;
    case TokenType::TRUE: emitByte(Chunk::OpCode::OP_TRUE); break;
    case TokenType::NIL: emitByte(Chunk::OpCode::OP_NIL); break;
    default: return;
    }
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
    case TokenType::BANG: emitByte(Chunk::OpCode::OP_NOT); break;
    // TODO: inc, dec
    default: return;
    }
}

void SpicyCompiler::binary() {
    const auto opType = m_previous.type;
    const auto& rule = m_rules[opType];
    parsePrecedence((Precedence)((int)rule.precedence + 1)); // TODO: that's bad
    
    switch (opType) {
    case TokenType::BANG_EQUAL:     emitBytes(Chunk::OpCode::OP_EQUAL, Chunk::OpCode::OP_NOT); break;
    case TokenType::EQUAL_EQUAL:    emitByte(Chunk::OpCode::OP_EQUAL); break;
    case TokenType::GREATER:        emitByte(Chunk::OpCode::OP_GREATER); break;
    case TokenType::GREATER_EQUAL:  emitBytes(Chunk::OpCode::OP_LESS, Chunk::OpCode::OP_NOT); break;
    case TokenType::LESS:           emitByte(Chunk::OpCode::OP_LESS); break;
    case TokenType::LESS_EQUAL:     emitBytes(Chunk::OpCode::OP_GREATER, Chunk::OpCode::OP_NOT); break;
    case TokenType::PLUS:           emitByte(Chunk::OpCode::OP_ADD); break;
    case TokenType::MINUS:          emitByte(Chunk::OpCode::OP_SUBTRACT); break;
    case TokenType::STAR:           emitByte(Chunk::OpCode::OP_MULTIPLY); break;
    case TokenType::SLASH:          emitByte(Chunk::OpCode::OP_DIVIDE); break;
    // TODO: append
    default: return;
    }
}

void SpicyCompiler::string() {
    if (m_previous.literal.has_value() &&
        std::holds_alternative<std::string>(m_previous.literal.value())) {
        emitConstant(std::get<std::string>(m_previous.literal.value()));
    } else {
        errorAt(m_previous, "Expected string literal.");
    }
}

uint8_t SpicyCompiler::parseVar(const std::string& errMsg) {
    consume(TokenType::IDENTIFIER, errMsg);
    return identifierConst(m_previous);
}

uint8_t SpicyCompiler::identifierConst(const spicy::Token& name) {
    return uint8_t(makeConstant(name.lexeme));
}

void SpicyCompiler::defineVariable(uint8_t global) {
    emitBytes(Chunk::OpCode::OP_DEFINE_GLOBAL, global);
}

}
