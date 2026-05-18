#include "Parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}

// ── Helpers ──────────────────────────────────────────────────

Token& Parser::peek()     { return tokens[current]; }
Token& Parser::previous() { return tokens[current - 1]; }
bool   Parser::isAtEnd()  { return peek().type == TokenType::END_OF_FILE; }

Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) {
        if (check(t)) { advance(); return true; }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw std::runtime_error(msg + " at line " + std::to_string(peek().line));
}

// ── Statements ───────────────────────────────────────────────

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::LET}))   return letStatement();
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::INPUT})) return inputStatement();
    if (match({TokenType::IF}))    return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::LBRACE}))return block();
    return exprStatement();
}

std::unique_ptr<Stmt> Parser::letStatement() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'let'");
    consume(TokenType::EQ, "Expected '=' after variable name");
    auto init = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<LetStmt>(name.lexeme, std::move(init));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    auto val = expression();
    consume(TokenType::RPAREN, "Expected ')' after print expression");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    return std::make_unique<PrintStmt>(std::move(val));
}

std::unique_ptr<Stmt> Parser::inputStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'input'");
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name in input()");
    consume(TokenType::RPAREN, "Expected ')' after input variable");
    consume(TokenType::SEMICOLON, "Expected ';' after input statement");
    return std::make_unique<InputStmt>(name.lexeme);
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after if condition");
    auto thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) elseBranch = statement();
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!check(TokenType::RBRACE) && !isAtEnd())
        stmts.push_back(statement());
    consume(TokenType::RBRACE, "Expected '}' to close block");
    return std::make_unique<BlockStmt>(std::move(stmts));
}

std::unique_ptr<Stmt> Parser::exprStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

// ── Expressions (precedence: low → high) ─────────────────────

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
    auto left = equality();
    if (match({TokenType::EQ})) {
        auto val = assignment();
        if (auto* v = dynamic_cast<VarExpr*>(left.get()))
            return std::make_unique<AssignExpr>(v->name, std::move(val));
        throw std::runtime_error("Invalid assignment target");
    }
    return left;
}

std::unique_ptr<Expr> Parser::equality() {
    auto left = comparison();
    while (match({TokenType::EQ_EQ, TokenType::BANG_EQ})) {
        std::string op = previous().lexeme;
        left = std::make_unique<BinaryExpr>(std::move(left), op, comparison());
    }
    return left;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto left = term();
    while (match({TokenType::LT, TokenType::GT, TokenType::LT_EQ, TokenType::GT_EQ})) {
        std::string op = previous().lexeme;
        left = std::make_unique<BinaryExpr>(std::move(left), op, term());
    }
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    auto left = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        std::string op = previous().lexeme;
        left = std::make_unique<BinaryExpr>(std::move(left), op, factor());
    }
    return left;
}

std::unique_ptr<Expr> Parser::factor() {
    auto left = unary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        std::string op = previous().lexeme;
        left = std::make_unique<BinaryExpr>(std::move(left), op, unary());
    }
    return left;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::MINUS})) {
        auto right = unary();
        // sugar: negate as (0 - right)
        return std::make_unique<BinaryExpr>(
            std::make_unique<NumberExpr>(0), "-", std::move(right));
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::NUMBER}))
        return std::make_unique<NumberExpr>(previous().intVal);
    if (match({TokenType::TRUE}))
        return std::make_unique<BoolExpr>(true);
    if (match({TokenType::FALSE}))
        return std::make_unique<BoolExpr>(false);
    if (match({TokenType::IDENTIFIER}))
        return std::make_unique<VarExpr>(previous().lexeme);
    if (match({TokenType::LPAREN})) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after grouped expression");
        return expr;
    }
    throw std::runtime_error("Unexpected token '" + peek().lexeme + "' at line " + std::to_string(peek().line));
}
