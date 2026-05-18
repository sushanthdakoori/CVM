#pragma once
#include "Compiler.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

// Binary format per instruction:
//   1 byte  : opcode
//   1 byte  : operand type  (0=none, 1=int, 2=bool, 3=string)
//   N bytes : operand value
//     int    -> 4 bytes little-endian
//     bool   -> 1 byte (0 or 1)
//     string -> 2 bytes length + N bytes chars

class Serializer {
public:
    // Write bytecode to a .bytecode file
    static void write(const std::vector<Instruction>& code, const std::string& path) {
        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file for writing: " + path);

        // magic header
        f.write("CVM\0", 4);

        for (auto& instr : code) {
            // opcode
            uint8_t op = (uint8_t)instr.op;
            f.write((char*)&op, 1);

            // figure out operand type from opcode
            switch (instr.op) {
                case OpCode::PUSH_INT:
                case OpCode::JUMP:
                case OpCode::JUMP_IF_FALSE: {
                    uint8_t type = 1; // int
                    f.write((char*)&type, 1);
                    int32_t v = instr.operand.intVal;
                    f.write((char*)&v, 4);
                    break;
                }
                case OpCode::PUSH_BOOL: {
                    uint8_t type = 2; // bool
                    f.write((char*)&type, 1);
                    uint8_t v = instr.operand.boolVal ? 1 : 0;
                    f.write((char*)&v, 1);
                    break;
                }
                case OpCode::LOAD:
                case OpCode::STORE:
                case OpCode::INPUT: {
                    uint8_t type = 3; // string
                    f.write((char*)&type, 1);
                    uint16_t len = (uint16_t)instr.operand.strVal.size();
                    f.write((char*)&len, 2);
                    f.write(instr.operand.strVal.c_str(), len);
                    break;
                }
                default: {
                    uint8_t type = 0; // none
                    f.write((char*)&type, 1);
                    break;
                }
            }
        }
    }

    // Read bytecode from a .bytecode file
    static std::vector<Instruction> read(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file: " + path);

        // check magic header
        char magic[4];
        f.read(magic, 4);
        if (magic[0] != 'C' || magic[1] != 'V' || magic[2] != 'M')
            throw std::runtime_error("Not a valid .bytecode file");

        std::vector<Instruction> code;
        uint8_t op, operandType;

        while (f.read((char*)&op, 1)) {
            if (!f.read((char*)&operandType, 1))
                throw std::runtime_error("Unexpected end of bytecode file");

            OpCode opcode = (OpCode)op;

            if (operandType == 0) {
                code.emplace_back(Instruction(opcode));
            } else if (operandType == 1) {
                int32_t v;
                f.read((char*)&v, 4);
                code.emplace_back(Instruction(opcode, (int)v));
            } else if (operandType == 2) {
                uint8_t v;
                f.read((char*)&v, 1);
                code.emplace_back(Instruction(opcode, (bool)v));
            } else if (operandType == 3) {
                uint16_t len;
                f.read((char*)&len, 2);
                std::string s(len, '\0');
                f.read(&s[0], len);
                code.emplace_back(Instruction(opcode, s));
            }
        }
        return code;
    }
};
