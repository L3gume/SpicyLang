#pragma once
#ifndef H_SPICYTYPES
#define H_SPICYTYPES

#include <optional>
#include <variant>
#include <string>
#include <exception>

namespace spicy {

enum class TokenType {
    // single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    COLON, LEFT_BRACKET, RIGHT_BRACKET, PIPE, BACKSLASH,
    // one of two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    ARROW, RARROW,
    PLUS_PLUS, MINUS_MINUS,
    // literals
    IDENTIFIER, STRING, NUMBER, LIST,
    // keywords
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    END_OF_FILE, IMPORT,
    ERROR
};

using TokenLiteral = std::variant<double, std::string>;
using OptTokenLiteral = std::optional<TokenLiteral>;
struct Token {
    TokenType type;
    std::string lexeme;
    OptTokenLiteral literal;
    int line;

    Token();
    Token(TokenType type, const std::string& lexeme, OptTokenLiteral literal, int line);
    
    Token(const Token&) = default;
    Token(Token&&) = default;
    
    void operator=(const Token& rhs) noexcept;
    void operator=(Token&& rhs) noexcept;
    
    const std::string toString() const noexcept;
};

class RuntimeError : public std::exception {
    const Token m_token;
    const std::string m_msg;

public:
    RuntimeError(Token token, const std::string& msg);
    const char* what() const noexcept override;
    Token token() const;
};


}
#endif
