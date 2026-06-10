#pragma once
#include <vector>
#include <string>
#include "instruction.h"

// This is a header file, here we'll only declare class and the functions.
// We'll not define here what the actual function does, it will be done in .cpp file.
// Header declares what exists. The class name, its fields, its method names and parameters. Think of it as a table of contents
// This is called separation of interface and implementation.

constexpr size_t STACK_MAX = 4096; // max stack depth
// constexpr compiles it at compile time. (if possible)

enum class VMResult{   // VM needs to report what went wrong, these are basically the error codes.
    OK,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    DIVISION_BY_ZERO,
    UNKNOWN_OPCODE,
};

class VM{
public:
   
   VM(bool debug_mode = false);

   // load a program and run it
   VMResult run(const std::vector<Instruction>& program);

   // print the stack contents (for debug)
   void dump_stack() const;    // const means the object of the class cant be modified in this func.

private:

   bool debug_mode_;
   size_t ip_;
   std::vector<int64_t> stack_;
   int64_t locals_[256] = {};   // for storing local variables,SLOT

   // stack helpers
   VMResult push(int64_t val);
   VMResult pop(int64_t& out);
   int64_t peek() const;

   // debug helper
   void debug_print(const Instruction& instr) const;
   std::string opcode_name(OpCode op) const;


};