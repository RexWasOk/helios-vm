#include <iostream>
#include "../include/vm.h"

int main() {
    VM vm;

    // factorial(5) = 120
    // Convention: caller pushes argument, 
    // caller stores it in slot 0, leaves result on stack before RET
    //
    // Layout:
    //   ip 0-4 : main
    //   ip 5+  : factorial function

    std::vector<Instruction> program = {
        // ── main ──────────────────────────
        // ip=0  push argument
        Instruction(OpCode::PUSH, 5),

        // ip=1  call factorial at ip=5
        Instruction(OpCode::CALL, 5),

        // ip=2  print result
        Instruction(OpCode::PRINTLN),

        // ip=3  halt
        Instruction(OpCode::HALT),

        // filler so factorial starts cleanly at ip=5
        Instruction(OpCode::HALT),        // ip=4

        // ── factorial(n) at ip=5 ──────────
        // ip=5  store argument into slot 0
        Instruction(OpCode::STORE, 0),

        // ip=6  load n
        Instruction(OpCode::LOAD, 0),

        // ip=7  push 1
        Instruction(OpCode::PUSH, 1),

        // ip=8  n <= 1?
        Instruction(OpCode::LTE),

        // ip=9  if n <= 1 jump to base case at ip=13
        Instruction(OpCode::JNZ, 17),

        // recursive case: n * factorial(n-1)
        // ip=10 load n
        Instruction(OpCode::LOAD, 0),

        // ip=11 load n again, subtract 1
        Instruction(OpCode::LOAD, 0),
        Instruction(OpCode::PUSH, 1),  // ip=12
        Instruction(OpCode::SUB),      // ip=13

        // ip=14 call factorial(n-1)
        Instruction(OpCode::CALL, 5),

        // ip=15 multiply n * factorial(n-1)
        Instruction(OpCode::MUL),

        // ip=16 return result
        Instruction(OpCode::RET),

        // base case: return 1
        // ip=17 push 1
        Instruction(OpCode::PUSH, 1),

        // ip=18 return
        Instruction(OpCode::RET),
    };

    VMResult result = vm.run(program);
    if (result != VMResult::OK)
        std::cerr << "VM error: " << static_cast<int>(result) << "\n";

    return 0;
}