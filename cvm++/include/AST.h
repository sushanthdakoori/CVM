#pragma once
#include <memory>
#include <string>
#include <vector>

// Forward declarations
struct NumberExpr;
struct BoolExpr;
struct VarExpr;
struct BinaryExpr;
struct AssignExpr;

struct PrintStmt;
struct InputStmt;
struct LetStmt;
struct IfStmt;
struct WhileStmt;
struct BlockStmt;
struct ExprStmt;

// ── Expressions ─────────────────────────────────────────────

struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr : Expr {
    int value;
    explicit NumberExpr(int v) : value(v) {}
};

struct BoolExpr : Expr {
    bool value;
    explicit BoolExpr(bool v) : value(v) {}
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string n) : name(std::move(n)) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, std::string op, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
};

struct AssignExpr : Expr {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignExpr(std::string n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}
};

// ── Statements ───────────────────────────────────────────────

struct Stmt {
    virtual ~Stmt() = default;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

struct LetStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;
    LetStmt(std::string n, std::unique_ptr<Expr> init)
        : name(std::move(n)), initializer(std::move(init)) {}
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit PrintStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

struct InputStmt : Stmt {
    std::string varName;
    explicit InputStmt(std::string n) : varName(std::move(n)) {}
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> s) : stmts(std::move(s)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // may be null
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> els)
        : condition(std::move(cond)), thenBranch(std::move(then)), elseBranch(std::move(els)) {}
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body)
        : condition(std::move(cond)), body(std::move(body)) {}
};
