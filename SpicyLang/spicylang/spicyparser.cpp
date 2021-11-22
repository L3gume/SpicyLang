#include "spicyparser.h"

namespace spicy {

SpicyParser::SpicyParser(const std::vector<Token>& tokens) : m_tokens(tokens) {
    m_current = 0;
}

std::optional<ast::ExprPtrVariant> SpicyParser::parseExpr() {
    try {
        return expression();
    }  catch (ParseError) {
        return std::nullopt;
    }
}

ast::SpicyProgram SpicyParser::parseProgram() {
    auto stmts = std::vector<ast::StmtPtrVariant>{};
    try {
        while (!isAtEnd()) {
            stmts.emplace_back(declaration());
        }
    }
    catch (ParseError) {
        synchronize();
    }
    return stmts;
}

ast::ExprPtrVariant SpicyParser::expression() {
    return assignment();
}

ast::ExprPtrVariant SpicyParser::assignment() {
    auto expr = logicalOr();
    if (match(TokenType::EQUAL)) {
        const auto equals = previous();
        auto value = assignment();
        if (std::holds_alternative<ast::VariableExprPtr>(expr)) {
            const auto& name = std::get<ast::VariableExprPtr>(expr)->varName;
            return ast::createAssignEPV(name, std::move(value));
        } else if (std::holds_alternative<ast::GetExprPtr>(expr)) {
            const auto& get = std::get<ast::GetExprPtr>(expr);
            return ast::createSetEPV(std::move(get->object), get->name, std::move(value));
        } else if (std::holds_alternative<ast::IndexGetExprPtr>(expr)) {
            const auto& idxGet = std::get<ast::IndexGetExprPtr>(expr);
            return ast::createIndexSetEPV(std::move(idxGet->lbracket), std::move(idxGet->lst), std::move(idxGet->idx), std::move(value));
        }
        error(equals, "Invalid assignment target.");
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::logicalOr() {
    auto expr = logicalAnd();
    while (match(TokenType::OR)) {
        const auto op = previous();
        auto rhs = logicalAnd();
        expr = ast::createLogicalEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::logicalAnd() {
    auto expr = equality();
    while (match(TokenType::AND)) {
        const auto op = previous();
        auto rhs = equality();
        expr = ast::createLogicalEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::equality() {
    auto expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        const auto& op = previous();
        auto rhs = comparison();
        expr = ast::createBinaryEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::comparison() {
    auto expr = append();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        const auto& op = previous();
        auto rhs = append();
        expr = ast::createBinaryEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::append() {
    auto expr = term();
    while (match({TokenType::APPEND, TokenType::APPEND_FRONT})) {
        const auto& op = previous();
        auto rhs = term();
        expr = ast::createBinaryEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::term() {
    auto expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        const auto& op = previous();
        auto rhs = factor();
        expr = ast::createBinaryEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::factor() {
    auto expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        const auto& op = previous();
        auto rhs = unary();
        expr = ast::createBinaryEPV(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
        const auto& op = previous();
        auto rhs = unary();
        return ast::createUnaryEPV(op, std::move(rhs));
    }
    return postfix();
}

ast::ExprPtrVariant SpicyParser::postfix() {
    auto expr = call();
    while (match({ TokenType::PLUS_PLUS, TokenType::MINUS_MINUS })) {
        expr = ast::createPostfixEPV(std::move(expr), previous());
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::call() {
    auto expr = primary();
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::DOT)){
            const auto name = consume(TokenType::IDENTIFIER, "Expect prop name after '.'.");
            expr = ast::createGetEPV(std::move(expr), name);
        } else if (match(TokenType::LEFT_BRACKET)) {
            auto lbracket = previous();
            auto idx = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = ast::createIndexGetEPV(std::move(lbracket), std::move(expr), std::move(idx));
        } else {
            break;
        }
    }
    return expr;
}

ast::ExprPtrVariant SpicyParser::primary() {
    if (match(TokenType::FALSE)) return ast::createLiteralEPV("false");
    if (match(TokenType::TRUE)) return ast::createLiteralEPV("true");
    if (match(TokenType::NIL)) return ast::createLiteralEPV("nil");
    if (match(TokenType::LIST)) return ast::createLiteralEPV("<spicy_list>");

    if (match({TokenType::NUMBER, TokenType::STRING}))
        return ast::createLiteralEPV(previous().literal);

    if (match(TokenType::SUPER)) {
        const auto keyword = previous();
        consume(TokenType::DOT, "Expect '.' after 'super'.");
        const auto method = consume(TokenType::IDENTIFIER, "Expect superclass method name.");
        return ast::createSuperEPV(keyword, method);
    }

    if (match(TokenType::FUN))
        return lambdaFunction();

    if (match(TokenType::THIS))
        return ast::createThisEPV(previous());

    if (match(TokenType::IDENTIFIER))
        return ast::createVarEPV(previous());

    if (match(TokenType::LEFT_PAREN)) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return ast::createGroupingEPV(std::move(expr));
    }

    throw error(peek(), "Expected an expression.");
}

ast::ExprPtrVariant SpicyParser::lambdaFunction() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after lambda declaration.");
    std::vector<Token> params;
    if (!check(TokenType::RIGHT_PAREN)) {
    do {
        if (params.size() >= MAX_ARGS) {
        error(peek(), "Can't have move than " + std::to_string(MAX_ARGS) + " parameters.");
        }
        params.emplace_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
    } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after lambda parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before lambda body.");
    auto body = block();
    return ast::createFuncEPV(std::move(params), std::move(body));
}

ast::StmtPtrVariant SpicyParser::statement() {
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::PRINT)) return printStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::LEFT_BRACE)) return ast::createBlockSPV(block());
    return expressionStatement();
}

ast::StmtPtrVariant SpicyParser::declaration() {
    try {
        if (match(TokenType::CLASS)) return classDeclaration();
        if (match(TokenType::FUN)) return functionDeclaration("function");
        if (match(TokenType::VAR)) return varDeclaration();
        return statement();
    } catch (std::exception& ex) {
        synchronize();
        return ast::ExprStmtPtr{nullptr};
    }
}

ast::StmtPtrVariant SpicyParser::varDeclaration() {
    const auto& name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    std::optional<ast::ExprPtrVariant> initializer = std::nullopt;
    if (match(TokenType::EQUAL)) {
        initializer = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return ast::createVarSPV(name, std::move(initializer));
}

ast::StmtPtrVariant SpicyParser::printStatement() {
    auto value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
    return ast::createPrintSPV(std::move(value));
}

ast::StmtPtrVariant SpicyParser::returnStatement() {
    const auto& keyword = previous();
    std::optional<ast::ExprPtrVariant> expr = std::nullopt;
    if (!check(TokenType::SEMICOLON))
        expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return ast::createRetSPV(keyword, std::move(expr));
}

ast::StmtPtrVariant SpicyParser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after while.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    auto body = statement();
    return ast::createWhileSPV(std::move(condition), std::move(body));
}

ast::StmtPtrVariant SpicyParser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after for.");
    std::optional<ast::StmtPtrVariant> initializer;
    if (match(TokenType::SEMICOLON)) {
        initializer = std::nullopt;
    } else if (match(TokenType::VAR)) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    std::optional<ast::ExprPtrVariant> condition = std::nullopt;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ':' after loop condition.");

    std::optional<ast::ExprPtrVariant> increment = std::nullopt;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    auto body = statement();

    if (increment.has_value()) {
        std::vector<ast::StmtPtrVariant> bodyVec;
        bodyVec.emplace_back(std::move(body));
        bodyVec.emplace_back(ast::createExprSPV(std::move(increment.value())));
        body = ast::createBlockSPV(std::move(bodyVec));
    }

    if (!condition.has_value())
        condition = ast::createLiteralEPV("true");

    body = ast::createWhileSPV(std::move(condition.value()), std::move(body));

    if (initializer.has_value()) {
        std::vector<ast::StmtPtrVariant> bodyVec;
        bodyVec.emplace_back(std::move(initializer.value()));
        bodyVec.emplace_back(std::move(body));
        body = ast::createBlockSPV(std::move(bodyVec));
    }

    return body;
}

ast::StmtPtrVariant SpicyParser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return ast::createExprSPV(std::move(expr));
}

ast::StmtPtrVariant SpicyParser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if confition.");

    auto thenBranch = statement();
    std::optional<ast::StmtPtrVariant> elseBranch = std::nullopt;
    if (match(TokenType::ELSE))
        elseBranch = statement();
    return ast::createIfSPV(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

ast::StmtPtrVariant SpicyParser::functionDeclaration(const std::string &kind) {
    const auto& name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
    std::vector<Token> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (params.size() >= MAX_ARGS) {
                error(peek(), "Can't have move than " + std::to_string(MAX_ARGS) + " parameters.");
            }
            params.emplace_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after " + kind + " parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    auto body = block();
    auto func = ast::createFuncEPV(std::move(params), std::move(body));
    return ast::createFuncSPV(name, std::move(std::get<ast::FuncExprPtr>(func)));
}

ast::StmtPtrVariant SpicyParser::classDeclaration() {
    const auto name = consume(TokenType::IDENTIFIER, "Expect class name.");
    std::optional<ast::ExprPtrVariant> superClass = std::nullopt;
    if (match(TokenType::COLON)) {
        consume(TokenType::IDENTIFIER, "Expect superclass name.");
        superClass = ast::createVarEPV(previous());
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");
    std::vector<ast::FuncStmtPtr> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        methods.emplace_back(std::get<ast::FuncStmtPtr>(functionDeclaration("method")));
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return ast::createClassSPV(name, std::move(superClass), std::move(methods));
}

std::vector<ast::StmtPtrVariant> SpicyParser::block() {
    std::vector<ast::StmtPtrVariant> stmts{};
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        stmts.emplace_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block statement.");
    return stmts;
}

ast::ExprPtrVariant SpicyParser::finishCall(ast::ExprPtrVariant expr) {
    std::vector<ast::ExprPtrVariant> args;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (args.size() >= MAX_ARGS) {
                error(peek(), "Argument count exceeds maximum of " + std::to_string(MAX_ARGS) + ".");
            }
            args.emplace_back(expression());
        } while (match(TokenType::COMMA));
    }
    const auto paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after args.");
    return ast::createCallEPV(std::move(expr), paren, std::move(args));
}

bool SpicyParser::match(TokenType tokenType) {
    return match({tokenType});
}

bool SpicyParser::match(std::initializer_list<TokenType> types) {
    for (const auto& tokenType : types) {
        if (check(tokenType)) {
            advance();
            return true;
        }
    }
    return false;
}

bool SpicyParser::check(TokenType tokenType) {
    if (isAtEnd()) return false;
    return peek().type == tokenType;
}

bool SpicyParser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}

Token SpicyParser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

Token SpicyParser::previous() {
    return m_tokens[m_current - 1];
}

Token SpicyParser::peek() {
    return m_tokens[m_current];
}

Token SpicyParser::consume(TokenType tokenType, const std::string &msg) {
    if (check(tokenType)) return advance();
    throw error(peek(), msg);
}

SpicyParser::ParseError SpicyParser::error(Token token, const std::string &msg) {
    spicy::error(token, msg);
    return ParseError{};
}

void SpicyParser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch(peek().type) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default:
            break;
        }
        advance();
    }
}

} // namespace spicy
