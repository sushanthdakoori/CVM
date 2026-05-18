#include "Lexer.h"
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"let",   TokenType::LET},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"if",    TokenType::IF},
    {"else",  TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"true",  TokenType::TRUE},
    {"false", TokenType::FALSE},
};

Lexer::Lexer(std::string source) : source(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}

bool Lexer::isAtEnd() const { return current >= (int)source.size(); }

char Lexer::advance() { return source[current++]; }

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= (int)source.size()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': tokens.emplace_back(TokenType::LPAREN,    "(", line); break;
        case ')': tokens.emplace_back(TokenType::RPAREN,    ")", line); break;
        case '{': tokens.emplace_back(TokenType::LBRACE,    "{", line); break;
        case '}': tokens.emplace_back(TokenType::RBRACE,    "}", line); break;
        case ';': tokens.emplace_back(TokenType::SEMICOLON, ";", line); break;
        case '+': tokens.emplace_back(TokenType::PLUS,      "+", line); break;
        case '-': tokens.emplace_back(TokenType::MINUS,     "-", line); break;
        case '*': tokens.emplace_back(TokenType::STAR,      "*", line); break;
        case '/':
            if (match('/')) skipLineComment();
            else tokens.emplace_back(TokenType::SLASH, "/", line);
            break;
        case '=': tokens.emplace_back(match('=') ? TokenType::EQ_EQ  : TokenType::EQ,     match('=') ? "==" : "=",  line); break;
        case '!': tokens.emplace_back(match('=') ? TokenType::BANG_EQ : TokenType::BANG_EQ,"!=", line); break;
        case '<': tokens.emplace_back(match('=') ? TokenType::LT_EQ  : TokenType::LT,     match('=') ? "<=" : "<",  line); break;
        case '>': tokens.emplace_back(match('=') ? TokenType::GT_EQ  : TokenType::GT,     match('=') ? ">=" : ">",  line); break;
        case ' ': case '\r': case '\t': break;
        case '\n': line++; break;
        default:
            if (std::isdigit(c)) { current--; number(); }
            else if (std::isalpha(c) || c == '_') { current--; identifier(); }
            else throw std::runtime_error("Unexpected character '" + std::string(1, c) + "' at line " + std::to_string(line));
    }
}

void Lexer::number() {
    start = current;
    while (!isAtEnd() && std::isdigit(peek())) advance();
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(TokenType::NUMBER, text, (int)std::stoi(text), line);
}

void Lexer::identifier() {
    start = current;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) advance();
    std::string text = source.substr(start, current - start);
    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        TokenType type = it->second;
        if (type == TokenType::TRUE)  tokens.emplace_back(type, text, true,  line);
        else if (type == TokenType::FALSE) tokens.emplace_back(type, text, false, line);
        else tokens.emplace_back(type, text, line);
    } else {
        tokens.emplace_back(TokenType::IDENTIFIER, text, line);
    }
}

void Lexer::skipLineComment() {
    while (!isAtEnd() && peek() != '\n') advance();
}
