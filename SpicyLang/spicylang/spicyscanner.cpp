#include "spicy.h"
#include "spicyscanner.h"

#include <cctype>

namespace spicy {

SpicyScanner::SpicyScanner(const std::string& source) : m_source(source) {
    m_keywords["and"] = TokenType::AND;
    m_keywords["class"] = TokenType::CLASS;
    m_keywords["else"] = TokenType::ELSE;
    m_keywords["false"] = TokenType::FALSE;
    m_keywords["for"] = TokenType::FOR;
    m_keywords["if"] = TokenType::IF;
    m_keywords["nil"] = TokenType::NIL;
    m_keywords["or"] = TokenType::OR;
    m_keywords["print"] = TokenType::PRINT;
    m_keywords["return"] = TokenType::RETURN;
    m_keywords["super"] = TokenType::SUPER;
    m_keywords["this"] = TokenType::THIS;
    m_keywords["true"] = TokenType::TRUE;
    m_keywords["var"] = TokenType::VAR;
    m_keywords["while"] = TokenType::WHILE;
    m_keywords["fun"] = TokenType::FUN;
}

std::vector<Token> SpicyScanner::scanTokens() {
    m_start = m_current = 0;
    m_line = 1;
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    m_tokens.emplace_back(Token(TokenType::END_OF_FILE, "", std::nullopt, m_line));
    return m_tokens;
}

bool SpicyScanner::isAtEnd() {
    return m_current >= m_source.length();
}

void SpicyScanner::scanToken() {
    const auto c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case ':': addToken(TokenType::COLON); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); 
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); 
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); 
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); 
            break;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();       
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            m_line++;
            break;
        case '"':
            scanString();
            break;
        default:
            if (isDigit(c)) {
                scanNumber(); 
            } else if (isAlpha(c)) {
                scanIdentifier();
            } else {
                spicy::error(m_line, "Unexpected character.");
            }
            break;
    }
}

void SpicyScanner::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') m_line++;
        advance();
    }
    
    if (isAtEnd()) {
        spicy::error(m_line, "Unterminated string.");
        return;
    }
    
    advance();

    const auto& stringVal = m_source.substr(m_start + 1, m_current - m_start - 2);
    addToken(TokenType::STRING, stringVal);
}

void SpicyScanner::scanNumber() {
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }
    
    addToken(TokenType::NUMBER, std::stod(m_source.substr(m_start, m_current - m_start)));
}

void SpicyScanner::scanIdentifier() {
    while (isAlphaNum(peek())) advance();
    
    const auto& text = m_source.substr(m_start, m_current - m_start);
    if (m_keywords.find(text) != m_keywords.end()) {
        addToken(m_keywords[text]);
        return;
    }
    
    addToken(TokenType::IDENTIFIER);
}

char SpicyScanner::advance() {
    return m_source[m_current++];
}

void SpicyScanner::addToken(TokenType type) {
    addToken(type, std::nullopt);
}

void SpicyScanner::addToken(TokenType type, OptTokenLiteral literal) {
    const auto& text = m_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(Token(type, text, literal, m_line));
}

bool SpicyScanner::match(char expected) {
    if (isAtEnd() || m_source[m_current] != expected) return false;
    m_current++;
    return true;
}

char SpicyScanner::peek() {
    return isAtEnd() ? '\0' : m_source[m_current];
}

char SpicyScanner::peekNext() {
    if (m_current + 1 >= m_source.length()) return '\0';
    return m_source[m_current + 1];
}

bool SpicyScanner::isDigit(char c) {
    return (std::isdigit(static_cast<unsigned char>(c)) != 0);
}

bool SpicyScanner::isAlpha(char c) {
    return (std::isalpha(static_cast<unsigned char>(c)) != 0) || c == '_';
}

bool SpicyScanner::isAlphaNum(char c) {
    return isDigit(c) || isAlpha(c);
}

}
