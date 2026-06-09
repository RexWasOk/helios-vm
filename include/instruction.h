#pragma once
#include <cstdint>
#include "opcodes.h"

// int64_t -> signed 64bit integer - lets you push negative numbers too.

struct Instruction{
    OpCode opcode;       // opcode - which operation (PUSH, ADD, JMP, etc.)
    int64_t operand;     // its the argument. example: for "PUSH 5" opcode-> PUSH and operand -> 
    
    // this is the constructor func. for instructions without an operand. opcode(op), operand(0) {} is a member initializer list.
    explicit Instruction(OpCode op)  // The explicit keyword means you can't accidentally convert an OpCode to an Instruction implicitly.
        : opcode(op), operand(0) {}  // You have to be intentional about it.
    
    // this is for instructions with an operand.
    Instruction(OpCode op, int64_t val)
        : opcode(op), operand(val) {}

};

