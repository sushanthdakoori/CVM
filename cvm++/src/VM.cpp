#include "VM.h"
#include <iostream>
#include <stdexcept>

void VM::push(Value v) { stack.push_back(v); }

Value VM::pop() {
    if (stack.empty()) throw std::runtime_error("Stack underflow");
    Value v = stack.back();
    stack.pop_back();
    return v;
}

static void printValue(const Value& v) {
    if (v.type == ValueType::INT)  std::cout << v.intVal;
    else                           std::cout << (v.boolVal ? "true" : "false");
}

void VM::execute(const std::vector<Instruction>& code) {
    int ip = 0;
    while (ip < (int)code.size()) {
        const Instruction& instr = code[ip++];

        switch (instr.op) {

            case OpCode::PUSH_INT:
                push(Value::fromInt(instr.operand.intVal));
                break;

            case OpCode::PUSH_BOOL:
                push(Value::fromBool(instr.operand.boolVal));
                break;

            case OpCode::LOAD: {
                auto it = variables.find(instr.operand.strVal);
                if (it == variables.end())
                    throw std::runtime_error("Undefined variable '" + instr.operand.strVal + "'");
                push(it->second);
                break;
            }

            case OpCode::STORE:
                variables[instr.operand.strVal] = pop();
                break;

            case OpCode::ADD: { Value b = pop(); Value a = pop(); push(Value::fromInt(a.intVal + b.intVal)); break; }
            case OpCode::SUB: { Value b = pop(); Value a = pop(); push(Value::fromInt(a.intVal - b.intVal)); break; }
            case OpCode::MUL: { Value b = pop(); Value a = pop(); push(Value::fromInt(a.intVal * b.intVal)); break; }
            case OpCode::DIV: {
                Value b = pop(); Value a = pop();
                if (b.intVal == 0) throw std::runtime_error("Division by zero");
                push(Value::fromInt(a.intVal / b.intVal));
                break;
            }

            case OpCode::EQ:  { Value b = pop(); Value a = pop(); push(Value::fromBool(a == b)); break; }
            case OpCode::NEQ: { Value b = pop(); Value a = pop(); push(Value::fromBool(a != b)); break; }
            case OpCode::LT:  { Value b = pop(); Value a = pop(); push(Value::fromBool(a.intVal <  b.intVal)); break; }
            case OpCode::GT:  { Value b = pop(); Value a = pop(); push(Value::fromBool(a.intVal >  b.intVal)); break; }
            case OpCode::LTE: { Value b = pop(); Value a = pop(); push(Value::fromBool(a.intVal <= b.intVal)); break; }
            case OpCode::GTE: { Value b = pop(); Value a = pop(); push(Value::fromBool(a.intVal >= b.intVal)); break; }

            case OpCode::PRINT:
                printValue(pop());
                std::cout << "\n";
                break;

            case OpCode::INPUT: {
                int val;
                std::cout << instr.operand.strVal << " = ";
                std::cin >> val;
                variables[instr.operand.strVal] = Value::fromInt(val);
                break;
            }

            case OpCode::JUMP:
                ip = instr.operand.intVal;
                break;

            case OpCode::JUMP_IF_FALSE: {
                Value cond = pop();
                bool isFalse = (cond.type == ValueType::BOOL) ? !cond.boolVal : (cond.intVal == 0);
                if (isFalse) ip = instr.operand.intVal;
                break;
            }

            case OpCode::POP:
                pop();
                break;

            case OpCode::HALT:
                return;

            default:
                throw std::runtime_error("Unknown opcode");
        }
    }
}

// full program run — fresh state every time
void VM::run(const std::vector<Instruction>& code) {
    stack.clear();
    variables.clear();
    execute(code);
}

// REPL run — keeps variables alive between lines
void VM::runLine(const std::vector<Instruction>& code) {
    stack.clear(); // clear stack but keep variables
    execute(code);
}

