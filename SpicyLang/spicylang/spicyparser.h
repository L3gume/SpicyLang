#pragma once
#ifndef H_SPICY_SPICYPARSER
#define H_SPICY_SPICYPARSER

#include <optional>
#include <vector>
#include <initializer_list>

#include "spicyast.h"
#include "spicy.h"

namespace spicy {

/* spicylang grammar (this language is essentially lox from the Crafting Interpreters book)
    TODO: complete the grammar

    expression     → equality ;
    equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    term           → factor ( ( "-" | "+" ) factor )* ;
    factor         → unary ( ( "/" | "*" ) unary )* ;
    unary          → ( "!" | "-" ) unary
                    | primary ;
    primary        → NUMBER | STRING | "true" | "false" | "nil"
                    | "(" expression ")" ;
 */

class SpicyParser
{
    static constexpr auto MAX_ARGS = 255;
    std::vector<Token> m_tokens;
    uint32_t m_current = 0;

public:
    explicit SpicyParser(const std::vector<Token>& tokens);

    class ParseError : std::exception {};

    std::optional<ast::ExprPtrVariant> parseExpr();
    ast::SpicyProgram parseProgram();
private:
    ast::ExprPtrVariant expression();
    ast::ExprPtrVariant assignment();
    ast::ExprPtrVariant logicalOr();
    ast::ExprPtrVariant logicalAnd();
    ast::ExprPtrVariant equality();
    ast::ExprPtrVariant comparison();
    ast::ExprPtrVariant append();
    ast::ExprPtrVariant chain();
    ast::ExprPtrVariant term();
    ast::ExprPtrVariant factor();
    ast::ExprPtrVariant unary();
    ast::ExprPtrVariant postfix();
    ast::ExprPtrVariant call();
    ast::ExprPtrVariant primary();
    ast::ExprPtrVariant lambdaFunction();

    ast::StmtPtrVariant statement();
    ast::StmtPtrVariant declaration();
    ast::StmtPtrVariant varDeclaration();
    ast::StmtPtrVariant printStatement();
    ast::StmtPtrVariant returnStatement();
    ast::StmtPtrVariant whileStatement();
    ast::StmtPtrVariant forStatement();
    ast::StmtPtrVariant expressionStatement();
    ast::StmtPtrVariant ifStatement();
    ast::StmtPtrVariant functionDeclaration(const std::string& kind);
    ast::StmtPtrVariant classDeclaration();
    std::vector<ast::StmtPtrVariant> block();

    ast::ExprPtrVariant finishCall(ast::ExprPtrVariant expr);

    [[nodiscard]] bool match(TokenType tokenType);
    [[nodiscard]] bool match(std::initializer_list<TokenType> types);
    [[nodiscard]] bool check(TokenType tokenType);
    [[nodiscard]] bool isAtEnd();
    Token advance();
    Token previous();
    [[nodiscard]] Token peek();
    Token consume(TokenType tokenType, const std::string& msg);

    ParseError error(Token token, const std::string& msg);
    void synchronize();
};

} // namespace spicy

#endif // SPICY_SPICYPARSER_H
