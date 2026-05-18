#pragma once
#include <string>

enum class TokenType {
    // Literals
    NUMBER, BOOL, IDENTIFIER,

    // Keywords
    LET, PRINT, INPUT, IF, ELSE, WHILE, TRUE, FALSE,

    // Operators
    PLUS, MINUS, STAR, SLASH,
    EQ_EQ, BANG_EQ, LT, GT, LT_EQ, GT_EQ,
    EQ,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON,

    // Special
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int intVal;
    bool boolVal;
    int line;

    Token(TokenType type, std::string lexeme, int line)
        : type(type), lexeme(std::move(lexeme)), intVal(0), boolVal(false), line(line) {}

    Token(TokenType type, std::string lexeme, int val, int line)
        : type(type), lexeme(std::move(lexeme)), intVal(val), boolVal(false), line(line) {}

    Token(TokenType type, std::string lexeme, bool val, int line)
        : type(type), lexeme(std::move(lexeme)), intVal(0), boolVal(val), line(line) {}
};
