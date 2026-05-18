#pragma once
#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0, current = 0, line = 1;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    void scanToken();
    void number();
    void identifier();
    void skipLineComment();
};
