#pragma once
#ifndef H_SPICYSCANNER
#define H_SPICYSCANNER

#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <unordered_map>
#include <iostream>

#include "types.h"

namespace spicy {

class SpicyScanner {
    const std::string m_source;
    std::vector<Token> m_tokens{};
    std::unordered_map<std::string, TokenType> m_keywords{};
    
    int m_start = 0;
    int m_current = 0;
    int m_line = 1;
    
public:
    SpicyScanner(const std::string& source);
    std::vector<Token> scanTokens();
    
    Token scanSingle();
    
private:
    bool isAtEnd();
    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifier();
    char advance();
    void addToken(TokenType type);
    void addToken(TokenType type, OptTokenLiteral literal);
    bool match(char expected);
    char peek();
    char peekNext();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNum(char c);
};

}
#endif
