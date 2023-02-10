#include "spicycompiler.h"

#include <format>
#include <variant>

namespace spicy {

SpicyCompiler::SpicyCompiler(SpicyScanner scanner) : m_scanner(std::move(scanner)) {
    /*
     * This monstrosity is all the rules for our Pratt parser
     */
    m_rules[TokenType::ARROW]          = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::RARROW]    = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::LEFT_PAREN]      = { [&](bool) { this->grouping(); },    std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::RIGHT_PAREN]     = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::LEFT_BRACE]      = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::RIGHT_BRACE]     = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::COMMA]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::DOT]             = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::MINUS]           = { [&](bool) { this->unary(); },       [&](bool) { this->binary(); },  Precedence::PREC_TERM };
    m_rules[TokenType::PLUS]            = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_TERM };
    m_rules[TokenType::SEMICOLON]       = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::SLASH]           = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_FACTOR };
    m_rules[TokenType::STAR]            = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_FACTOR };
    m_rules[TokenType::BANG]            = { [&](bool) { this->unary(); },       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::BANG_EQUAL]      = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_EQUALITY };
    m_rules[TokenType::EQUAL]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::EQUAL_EQUAL]     = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_EQUALITY };
    m_rules[TokenType::GREATER]         = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_COMPARISON };
    m_rules[TokenType::GREATER_EQUAL]   = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_COMPARISON };
    m_rules[TokenType::LESS]            = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_COMPARISON };
    m_rules[TokenType::LESS_EQUAL]      = { std::nullopt,                       [&](bool) { this->binary(); },  Precedence::PREC_COMPARISON };
    m_rules[TokenType::STRING]          = { [&](bool) { this->string(); },      std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::NUMBER]          = { [&](bool) { this->number(); },      std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::AND]             = { std::nullopt,                       [&](bool) { this->and_(); },    Precedence::PREC_AND };
    m_rules[TokenType::CLASS]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::ELSE]            = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::FALSE]           = { [&](bool) { this->literal(); },     std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::FOR]             = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::FUN]             = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::IF]              = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::NIL]             = { [&](bool) { this->literal(); },     std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::OR]              = { std::nullopt,                       [&](bool) { this->or_(); },     Precedence::PREC_OR };
    m_rules[TokenType::PRINT]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::RETURN]          = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::SUPER]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::THIS]            = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::TRUE]            = { [&](bool) { this->literal(); },     std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::VAR]             = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::WHILE]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::ERROR]           = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::END_OF_FILE]     = { std::nullopt,                       std::nullopt,                   Precedence::PREC_NONE };
    m_rules[TokenType::IDENTIFIER]      = { [&](bool canAssign) { this->variable(canAssign); }, std::nullopt,   Precedence::PREC_NONE }; }

auto SpicyCompiler::compile() -> Func {
    m_function = { .object = nullptr, .arity = 0, .chunk = {}, .name = "" };
    //m_locals.emplace_back({ .name = { .type = TokenType::ERROR, .lexeme = "", .literal = std::nullopt, .line = -1 }, .depth = 0});
    //m_chunk = Chunk(); // reset the current chunk
    advance();
    
    while (!match(TokenType::END_OF_FILE)) {
        declaration();
    }
    
    return m_function;
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
    m_function.chunk.appendByte(byte, m_previous.line);
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

void SpicyCompiler::emitLoop(uint32_t loopStart) {
    emitByte(Chunk::OpCode::OP_LOOP);

    auto offset = m_chunk.getBytecodeCount() - loopStart + 2;
    if (offset > std::numeric_limits<uint16_t>::max()) {
        error("Too much code to jump over in loop.");
    }
    
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

uint16_t SpicyCompiler::emitJump(Chunk::OpCode byte) {
    emitByte(byte);
    // emit temporary offset (two bytes) to be set later when the proper values are known
    emitByte(0xff);
    emitByte(0xff);
    return m_chunk.getBytecodeCount() - 2;
}

void SpicyCompiler::patchJump(uint16_t offset) {
    auto jump = m_chunk.getBytecodeCount() - offset - 2;
    
    if (jump > std::numeric_limits<uint16_t>::max()) {
        error("Too much code to jump over.");
    }
    
    m_chunk.setBytecodeValue(offset, (jump >> 8) & 0xff);
    m_chunk.setBytecodeValue(offset + 1, jump & 0xff);
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
    const auto canAssign = prec <= Precedence::PREC_ASSIGNMENT;
    prefixRule(canAssign);
    
    while (prec <= m_rules[m_current.type].precedence) {
        advance();
        const auto& infixRuleOpt = m_rules[m_previous.type].infix;
        if (!infixRuleOpt.has_value()) {
            error("Expect infix expression.");
            return;
        }
        const auto& infixRule = infixRuleOpt.value();
        infixRule(canAssign);
    }
    
    if (canAssign && match(TokenType::EQUAL)) {
        error("Invalid assignment target.");
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
    } else if (match(TokenType::IF)) {
        ifStatement();
    } else if (match(TokenType::WHILE)) {
        whileStatement();
    } else if (match(TokenType::FOR)) {
        forStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

void SpicyCompiler::block() {
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
        declaration();
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of block.");
}

void SpicyCompiler::printStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(Chunk::OpCode::OP_PRINT);
}

void SpicyCompiler::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    auto thenJump = emitJump(Chunk::OpCode::OP_JUMP_IF_FALSE);
    emitByte(Chunk::OpCode::OP_POP);
    statement();
    
    auto elseJump = emitJump(Chunk::OpCode::OP_JUMP);
    patchJump(thenJump);
    emitByte(Chunk::OpCode::OP_POP);
    
    if (match(TokenType::ELSE)) statement();
    patchJump(elseJump);
}

void SpicyCompiler::whileStatement() {
    auto loopStart = m_chunk.getBytecodeCount();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    auto exitJump = emitJump(Chunk::OpCode::OP_JUMP_IF_FALSE);
    emitByte(Chunk::OpCode::OP_POP);
    statement();
    emitLoop(loopStart);
    
    patchJump(exitJump);
    emitByte(Chunk::OpCode::OP_POP);
}

void SpicyCompiler::forStatement() {
    beginScope();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    //consume(TokenType::SEMICOLON, "Expect ';'.");
    if (match(TokenType::SEMICOLON)) {
        // no initializer
    } else if (match(TokenType::VAR)) {
        varDeclaration();
    } else {
        // this looks for a ';' at the end and also emits a pop instruction, which is what we want
        expressionStatement();
    }
    auto loopStart = m_chunk.getBytecodeCount();
    auto exitJump = -1;
    
    if (!match(TokenType::SEMICOLON)) {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

        exitJump = emitJump(Chunk::OpCode::OP_JUMP_IF_FALSE);
        emitByte(Chunk::OpCode::OP_POP);
    }
    
    if (!match(TokenType::RIGHT_PAREN)) {
        auto bodyJump = emitJump(Chunk::OpCode::OP_JUMP);
        auto incStart = m_chunk.getBytecodeCount();
        
        expression();
        
        emitByte(Chunk::OpCode::OP_POP);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after 'for' clauses.");

        emitLoop(loopStart);
        loopStart = incStart;
        patchJump(bodyJump);
    }
    
    statement();
    emitLoop(loopStart);
    
    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(Chunk::OpCode::OP_POP);
    }
    endScope();
}

void SpicyCompiler::expressionStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emitByte(Chunk::OpCode::OP_POP);
}

void SpicyCompiler::expression() {
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void SpicyCompiler::variable(bool canAssign) {
    namedVariable(m_previous, canAssign);
}

void SpicyCompiler::namedVariable(const spicy::Token& name, bool canAssign) {
    Chunk::OpCode getOp, setOp;
    auto arg = resolveLocal(name);
    if (arg != -1) {
        getOp = Chunk::OpCode::OP_GET_LOCAL;
        setOp = Chunk::OpCode::OP_SET_LOCAL;
    } else {
        arg = identifierConst(name);
        getOp = Chunk::OpCode::OP_GET_GLOBAL;
        setOp = Chunk::OpCode::OP_SET_GLOBAL;
    }
    
    if (canAssign && match(TokenType::EQUAL)) {
        expression();
        emitBytes(setOp, arg);
    } else {
		emitBytes(getOp, arg);
    }
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

void SpicyCompiler::and_() {
    auto endJump = emitJump(Chunk::OpCode::OP_JUMP_IF_FALSE);
    emitByte(Chunk::OpCode::OP_POP);
    parsePrecedence(Precedence::PREC_AND);
    patchJump(endJump);
}

void SpicyCompiler::or_() {
    auto elseJump = emitJump(Chunk::OpCode::OP_JUMP_IF_FALSE);
    auto endJump = emitJump(Chunk::OpCode::OP_JUMP);
    
    patchJump(elseJump);
    emitByte(Chunk::OpCode::OP_POP);
    
    parsePrecedence(Precedence::PREC_OR);
    patchJump(endJump);
}

void SpicyCompiler::noop() {
    // noOp
}

void SpicyCompiler::beginScope() {
    m_scopeDepth++; 
}

void SpicyCompiler::endScope() {
    m_scopeDepth--;
    const auto erased = std::erase_if(m_locals, [&](const Local& local) { return local.depth > m_scopeDepth; });
    for (auto i = 0; i < erased; ++i) {
        emitByte(Chunk::OpCode::OP_POP);
    }
}

uint8_t SpicyCompiler::parseVar(const std::string& errMsg) {
    consume(TokenType::IDENTIFIER, errMsg);
    
    declareVariable();
    if (m_scopeDepth > 0) {
        return 0;
    }
    
    return identifierConst(m_previous);
}

uint8_t SpicyCompiler::identifierConst(const spicy::Token& name) {
    return uint8_t(makeConstant(name.lexeme));
}

void SpicyCompiler::declareVariable() {
    if (m_scopeDepth == 0) {
        return;
    }
    
    const auto& name = m_previous;
    for (int i = m_locals.size() - 1; i >= 0; --i) {
        const auto& local = m_locals[i];
        if (local.depth != -1 && local.depth < m_scopeDepth) {
            break;
        }
        if (name.lexeme == local.name.lexeme) {
            error(std::format("Variable [{}] is alread defined in this scope.", name.lexeme));
        }
    }
    addLocal(name);
}

void SpicyCompiler::defineVariable(uint8_t global) {
    if (m_scopeDepth > 0) {
        markInitialized();
        return;
    }
    
    emitBytes(Chunk::OpCode::OP_DEFINE_GLOBAL, global);
}

void SpicyCompiler::addLocal(const spicy::Token& name) {
    m_locals.emplace_back(Local{ .name = name, .depth = -1 });
}

void SpicyCompiler::markInitialized() {
    m_locals.back().depth = m_scopeDepth;
}

uint32_t SpicyCompiler::resolveLocal(const spicy::Token& name)
{
    for (auto i = static_cast<int>(m_locals.size()) - 1; i >= 0; --i) {
        if (name.lexeme == m_locals[i].name.lexeme) {
            if (m_locals[i].depth == -1) {
                error(std::format("Can't read variable [{}] during in its own initializer.", name.lexeme));
            }
            return i;
        }
    }
    return -1;
}

}
