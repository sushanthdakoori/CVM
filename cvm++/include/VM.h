#pragma once
#include "Compiler.h"
#include <vector>
#include <unordered_map>
#include <string>

enum class ValueType { INT, BOOL };

struct Value {
    ValueType type;
    int intVal;
    bool boolVal;

    static Value fromInt(int v)  { Value val; val.type = ValueType::INT;  val.intVal = v;  val.boolVal = false; return val; }
    static Value fromBool(bool v){ Value val; val.type = ValueType::BOOL; val.intVal = 0;  val.boolVal = v;     return val; }

    bool operator==(const Value& o) const {
        if (type != o.type) return false;
        return type == ValueType::INT ? intVal == o.intVal : boolVal == o.boolVal;
    }
    bool operator!=(const Value& o) const { return !(*this == o); }
};

class VM {
public:
    void run(const std::vector<Instruction>& code);

    void runLine(const std::vector<Instruction>& code);

private:
    std::vector<Value> stack;
    std::unordered_map<std::string, Value> variables;

    void  push(Value v);
    Value pop();
    void  execute(const std::vector<Instruction>& code);
};
