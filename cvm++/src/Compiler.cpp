#include "Compiler.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

std::vector<Instruction> Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& stmts) {
    code.clear();
    for (auto& s : stmts) compileStmt(s.get());
    emit(Instruction(OpCode::HALT));
    return std::move(code);
}

int Compiler::emit(Instruction instr) {
    code.push_back(std::move(instr));
    return (int)code.size() - 1;
}

void Compiler::patchJump(int idx, int target) {
    code[idx].operand.intVal = target;
}


void Compiler::compileStmt(const Stmt* stmt) {
    if (auto* s = dynamic_cast<const LetStmt*>(stmt)) {
        compileExpr(s->initializer.get());
        emit(Instruction(OpCode::STORE, s->name));

    } else if (auto* s = dynamic_cast<const ExprStmt*>(stmt)) {
        compileExpr(s->expr.get());
        emit(Instruction(OpCode::POP));

    } else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        compileExpr(s->expr.get());
        emit(Instruction(OpCode::PRINT));

    } else if (auto* s = dynamic_cast<const InputStmt*>(stmt)) {
        emit(Instruction(OpCode::INPUT, s->varName));

    } else if (auto* s = dynamic_cast<const BlockStmt*>(stmt)) {
        for (auto& inner : s->stmts) compileStmt(inner.get());

    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        compileExpr(s->condition.get());
        int jumpIfFalse = emit(Instruction(OpCode::JUMP_IF_FALSE, 0));

        compileStmt(s->thenBranch.get());

        if (s->elseBranch) {
            int jumpOver = emit(Instruction(OpCode::JUMP, 0));
            patchJump(jumpIfFalse, (int)code.size());
            compileStmt(s->elseBranch.get());
            patchJump(jumpOver, (int)code.size());
        } else {
            patchJump(jumpIfFalse, (int)code.size());
        }

    } else if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        int loopStart = (int)code.size();
        compileExpr(s->condition.get());
        int exitJump = emit(Instruction(OpCode::JUMP_IF_FALSE, 0));
        compileStmt(s->body.get());
        emit(Instruction(OpCode::JUMP, loopStart));
        patchJump(exitJump, (int)code.size());

    } else {
        throw std::runtime_error("Unknown statement type in compiler");
    }
}


void Compiler::compileExpr(const Expr* expr) {
    if (auto* e = dynamic_cast<const NumberExpr*>(expr)) {
        emit(Instruction(OpCode::PUSH_INT, e->value));

    } else if (auto* e = dynamic_cast<const BoolExpr*>(expr)) {
        emit(Instruction(OpCode::PUSH_BOOL, e->value));

    } else if (auto* e = dynamic_cast<const VarExpr*>(expr)) {
        emit(Instruction(OpCode::LOAD, e->name));

    } else if (auto* e = dynamic_cast<const AssignExpr*>(expr)) {
        compileExpr(e->value.get());
        emit(Instruction(OpCode::STORE, e->name));
        emit(Instruction(OpCode::LOAD,  e->name));

    } else if (auto* e = dynamic_cast<const BinaryExpr*>(expr)) {
        compileExpr(e->left.get());
        compileExpr(e->right.get());
        if      (e->op == "+")  emit(Instruction(OpCode::ADD));
        else if (e->op == "-")  emit(Instruction(OpCode::SUB));
        else if (e->op == "*")  emit(Instruction(OpCode::MUL));
        else if (e->op == "/")  emit(Instruction(OpCode::DIV));
        else if (e->op == "==") emit(Instruction(OpCode::EQ));
        else if (e->op == "!=") emit(Instruction(OpCode::NEQ));
        else if (e->op == "<")  emit(Instruction(OpCode::LT));
        else if (e->op == ">")  emit(Instruction(OpCode::GT));
        else if (e->op == "<=") emit(Instruction(OpCode::LTE));
        else if (e->op == ">=") emit(Instruction(OpCode::GTE));
        else throw std::runtime_error("Unknown operator: " + e->op);

    } else {
        throw std::runtime_error("Unknown expression type in compiler");
    }
}


void Compiler::disassemble(const std::vector<Instruction>& code) {
    auto opName = [](OpCode op) -> std::string {
        switch(op) {
            case OpCode::PUSH_INT:      return "PUSH_INT";
            case OpCode::PUSH_BOOL:     return "PUSH_BOOL";
            case OpCode::LOAD:          return "LOAD";
            case OpCode::STORE:         return "STORE";
            case OpCode::ADD:           return "ADD";
            case OpCode::SUB:           return "SUB";
            case OpCode::MUL:           return "MUL";
            case OpCode::DIV:           return "DIV";
            case OpCode::EQ:            return "EQ";
            case OpCode::NEQ:           return "NEQ";
            case OpCode::LT:            return "LT";
            case OpCode::GT:            return "GT";
            case OpCode::LTE:           return "LTE";
            case OpCode::GTE:           return "GTE";
            case OpCode::PRINT:         return "PRINT";
            case OpCode::INPUT:         return "INPUT";
            case OpCode::JUMP:          return "JUMP";
            case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
            case OpCode::POP:           return "POP";
            case OpCode::HALT:          return "HALT";
            default:                    return "UNKNOWN";
        }
    };

    std::cout << "\n── Bytecode Disassembly ─────────────────\n";
    for (int i = 0; i < (int)code.size(); i++) {
        std::cout << std::setw(4) << i << "  " << std::left << std::setw(16) << opName(code[i].op);
        // print operand based on opcode type
        switch(code[i].op) {
            case OpCode::PUSH_INT:
            case OpCode::JUMP:
            case OpCode::JUMP_IF_FALSE:
                std::cout << code[i].operand.intVal; break;
            case OpCode::PUSH_BOOL:
                std::cout << (code[i].operand.boolVal ? "true" : "false"); break;
            case OpCode::LOAD:
            case OpCode::STORE:
            case OpCode::INPUT:
                std::cout << code[i].operand.strVal; break;
            default: break;
        }
        std::cout << "\n";
    }
    std::cout << "─────────────────────────────────────────\n\n";
}
