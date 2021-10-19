#include "types.h"

namespace spicy {
// Token
Token::Token(TokenType type, const std::string& lexeme, OptTokenLiteral literal, int line) :
        type(type), lexeme(lexeme), literal(literal), line(line) {}

const std::string Token::toString() const noexcept {
    switch (type) {
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::MINUS: return "MINUS";
        case TokenType::PLUS: return "PLUS";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::SLASH: return "SLASH";
        case TokenType::STAR: return "STAR";
        case TokenType::BANG: return "BANG";
        case TokenType::COLON: return "COLON";
        case TokenType::BANG_EQUAL: return "BANG_EQUAL";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::IDENTIFIER: return "IDENTIFIER(" + lexeme + ")";
        case TokenType::STRING: return "STRING(" + lexeme + ")";
        case TokenType::NUMBER: return "NUMBER(" + lexeme + ")";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::CLASS: return "CLASS";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::FUN: return "FUN";
        case TokenType::THIS: return "THIS";
        case TokenType::SUPER: return "SUPER";
        case TokenType::VAR: return "VAR";
        case TokenType::NIL: return "NIL";
        case TokenType::WHILE: return "WHILE";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::PRINT: return "PRINT";
        default: return "Unkown(" + lexeme + ")";
    }
}

// RuntimeError
RuntimeError::RuntimeError(Token token, const std::string &msg) : m_token(std::move(token)), m_msg(msg) {}

const char* RuntimeError::what() const noexcept {
    return m_msg.c_str();
}

Token RuntimeError::token() const {
    return m_token;
}

} // namespace spicy
