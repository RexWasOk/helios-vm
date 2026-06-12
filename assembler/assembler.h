#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../include/instruction.h"

class Assembler{
public:
    // assemble .hasm file into a vector of Instructions
    std::vector<Instruction> assemble(const std::string& filename);

private:
    // label name → instruction address , we'll map them.
    std::unordered_map<std::string, size_t> labels_;

    // helpers
    std::string trim(const std::string& line);
    bool is_label(const std::string& line);
    bool is_comment(const std::string& line);
    OpCode parse_opcode(const std::string& token);
    
};