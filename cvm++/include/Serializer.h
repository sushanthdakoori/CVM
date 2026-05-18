#pragma once
#include "Compiler.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>


class Serializer {
public:
    static void write(const std::vector<Instruction>& code, const std::string& path) {
        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file for writing: " + path);

        f.write("CVM\0", 4);

        for (auto& instr : code) {
            uint8_t op = (uint8_t)instr.op;
            f.write((char*)&op, 1);

            
            switch (instr.op) {
                case OpCode::PUSH_INT:
                case OpCode::JUMP:
                case OpCode::JUMP_IF_FALSE: {
                    uint8_t type = 1; 
                    f.write((char*)&type, 1);
                    int32_t v = instr.operand.intVal;
                    f.write((char*)&v, 4);
                    break;
                }
                case OpCode::PUSH_BOOL: {
                    uint8_t type = 2; 
                    f.write((char*)&type, 1);
                    uint8_t v = instr.operand.boolVal ? 1 : 0;
                    f.write((char*)&v, 1);
                    break;
                }
                case OpCode::LOAD:
                case OpCode::STORE:
                case OpCode::INPUT: {
                    uint8_t type = 3; 
                    f.write((char*)&type, 1);
                    uint16_t len = (uint16_t)instr.operand.strVal.size();
                    f.write((char*)&len, 2);
                    f.write(instr.operand.strVal.c_str(), len);
                    break;
                }
                default: {
                    uint8_t type = 0; 
                    f.write((char*)&type, 1);
                    break;
                }
            }
        }
    }

    static std::vector<Instruction> read(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file: " + path);

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
