#pragma once
#include <cstdint>

// An enum (short for enumeration) is a user-defined type that lets you give names to a set of integer constants.
// basically, we are creating our own data type.
/*
example:
Instead of writing:
int direction = 0;

you can write:
enum Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

Direction direction = NORTH;

This makes the code much easier to read.

values start from 0, so push has value 0 pop has 1 and so on..
*/

enum class OpCode : uint8_t {   // uint8_t : each opcode is stored as an unsigned 8bit integer. It means our entire ISA fits in a single byte per instruction.
    // Stack ops
    PUSH, POP, DUP, SWAP,

    // Arithmetic
    ADD, SUB, MUL, DIV, MOD, NEG,

    // Comparisons
    EQ, NEQ, LT, LTE, GT, GTE,

    // Control flow
    JMP, JZ, JNZ,

    // Functions
    CALL, RET,

    // Variables
    LOAD, STORE,

    // I/O + control
    PRINT, PRINTLN, HALT,
};