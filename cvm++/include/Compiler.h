#pragma once
#include "AST.h"
#include <vector>
#include <string>
#include <unordered_map>

// ── Opcodes ──────────────────────────────────────────────────
enum class OpCode : uint8_t {
    PUSH_INT,
    PUSH_BOOL,
    LOAD,
    STORE,
    ADD, SUB, MUL, DIV,
    EQ, NEQ, LT, GT, LTE, GTE,
    PRINT,
    INPUT,
    JUMP,
    JUMP_IF_FALSE,
    POP,
    HALT
};

// ── Operand (no variant) ─────────────────────────────────────
struct Operand {
    int intVal;
    bool boolVal;
    std::string strVal;

    Operand() : intVal(0), boolVal(false) {}
    explicit Operand(int v)         : intVal(v),  boolVal(false) {}
    explicit Operand(bool v)        : intVal(0),  boolVal(v) {}
    explicit Operand(std::string v) : intVal(0),  boolVal(false), strVal(std::move(v)) {}
};

// ── Instruction ──────────────────────────────────────────────
struct Instruction {
    OpCode op;
    Operand operand;

    explicit Instruction(OpCode op) : op(op) {}
    Instruction(OpCode op, int v)         : op(op), operand(v) {}
    Instruction(OpCode op, bool v)        : op(op), operand(v) {}
    Instruction(OpCode op, std::string v) : op(op), operand(std::move(v)) {}
};

// ── Compiler ─────────────────────────────────────────────────
class Compiler {
public:
    std::vector<Instruction> compile(const std::vector<std::unique_ptr<Stmt>>& stmts);
    static void disassemble(const std::vector<Instruction>& code);

private:
    std::vector<Instruction> code;

    void compileStmt(const Stmt* stmt);
    void compileExpr(const Expr* expr);

    int  emit(Instruction instr);
    void patchJump(int idx, int target);
};
