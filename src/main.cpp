#include <iostream>
#include "../include/vm.h"

int main() {
    VM vm;

    // Test: print 1 to 5
    std::vector<Instruction> program = {
        Instruction(OpCode::PUSH, 1),    // ip=0
        Instruction(OpCode::STORE, 0),   // ip=1
        Instruction(OpCode::LOAD, 0),    // ip=2  ← loop start
        Instruction(OpCode::PUSH, 5),    // ip=3
        Instruction(OpCode::GT),         // ip=4
        Instruction(OpCode::JNZ, 13),    // ip=5  ← exit if i > 5
        Instruction(OpCode::LOAD, 0),    // ip=6
        Instruction(OpCode::PRINTLN),    // ip=7
        Instruction(OpCode::LOAD, 0),    // ip=8
        Instruction(OpCode::PUSH, 1),    // ip=9
        Instruction(OpCode::ADD),        // ip=10
        Instruction(OpCode::STORE, 0),   // ip=11
        Instruction(OpCode::JMP, 2),     // ip=12 ← back to loop start
        Instruction(OpCode::HALT),       // ip=13
    };

    VMResult result = vm.run(program);

    if (result != VMResult::OK)
        std::cerr << "VM error: " << static_cast<int>(result) << "\n";

    return 0;
}