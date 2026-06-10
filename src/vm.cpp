#include <iostream>
#include <iomanip>
#include "../include/vm.h"

/*
 * =============================================================
 *  Helios VM — vm.cpp
 * =============================================================
 *
 *  DAY 2 — Execution Engine
 *  -------------------------
 *  Built the core fetch-decode-execute loop inside VM::run().
 *  The loop reads one instruction at a time using the instruction
 *  pointer (ip_), decodes the opcode via a switch statement, and
 *  executes the corresponding logic. ip_ increments at the bottom
 *  of every iteration EXCEPT for jump instructions (JMP, JZ, JNZ,
 *  CALL, RET) which use `continue` to skip the increment and land
 *  exactly where they intended.
 *
 *  Stack is a std::vector<int64_t>. push() checks for overflow,
 *  pop() checks for underflow. Both return VMResult so errors
 *  propagate cleanly up to the caller without exceptions.
 *
 *  pop() uses an int64_t& reference parameter instead of a return
 *  value because the return value is already used for VMResult.
 *  The & means the caller's variable is updated directly.
 *
 *  DAY 3 — Call Frames and Functions
 *  -----------------------------------
 *  Problem: a single flat locals_[] array meant every function
 *  shared the same variable slots. factorial(5) calling factorial(4)
 *  would overwrite slot 0 and corrupt the parent's data.
 *
 *  Solution: call frames. Each function call gets its own Frame
 *  struct containing:
 *    - return_ip : the instruction to jump back to after RET
 *    - locals[]  : 256 isolated variable slots for that function
 *
 *  call_stack_ is a std::vector<Frame> (used as a stack).
 *  A root frame is pushed in the constructor so main() always
 *  has a valid frame to read/write locals from.
 *
 *  CALL <addr>:
 *    - Creates a new Frame, sets return_ip = ip_ + 1
 *    - Pushes frame onto call_stack_
 *    - Jumps ip_ to the function address
 *    - Uses `continue` to skip ip_ increment
 *
 *  RET:
 *    - Reads return_ip from the current (top) frame
 *    - Pops the frame off call_stack_
 *    - Jumps ip_ to return_ip
 *    - Uses `continue` to skip ip_ increment
 *
 *  LOAD/STORE now operate on current_frame().locals[] instead
 *  of a flat array, so each function's variables are fully isolated.
 *
 *  Tested with recursive factorial(5) = 120. At peak recursion
 *  there are 6 frames on the call stack (root + 5 factorial frames),
 *  each with its own n in slot 0, never interfering.
 * =============================================================
 */

// CONSTRUCTOR

VM::VM(bool debug_mode)
    : debug_mode_(debug_mode), ip_(0)
{
    stack_.reserve(STACK_MAX);
    call_stack_.push_back(Frame{});  // root frame for main
}


// STACK HELPERS

VMResult VM::push(int64_t val){

    if(stack_.size() >= STACK_MAX) return VMResult::STACK_OVERFLOW;

    stack_.push_back(val);

    return VMResult::OK;

}

VMResult VM::pop(int64_t& out){

    if(stack_.empty()) return VMResult::STACK_UNDERFLOW;

    out = stack_.back();
    stack_.pop_back();
    
    return VMResult::OK;
}

int64_t VM::peek() const{
    return stack_.back();
}

void VM::dump_stack() const{

    std::cout<<"Stack ["<<stack_.size()<<"]: ";
    for(auto v : stack_) std::cout<<v<<" ";
    std::cout<<"\n";
}

Frame& VM::current_frame() {
    return call_stack_.back();
}

//  Main execution loop 

VMResult VM::run(const std::vector<Instruction>& program) {
    ip_ = 0;

    while (ip_ < program.size()) {
        const Instruction& instr = program[ip_];  // fetch

        int64_t a = 0, b = 0;

        switch (instr.opcode) {       // decode -> Instruction is broken down into opcode and operand, that is the decoding of instr.

            case OpCode::PUSH:        // the switch case part is the execution
                if (auto r = push(instr.operand); r != VMResult::OK) return r;
                break;

            case OpCode::POP:
                if (auto r = pop(a); r != VMResult::OK) return r;
                break;

            case OpCode::DUP:
                if (stack_.empty()) return VMResult::STACK_UNDERFLOW;
                if (auto r = push(peek()); r != VMResult::OK) return r;
                break;

            case OpCode::SWAP:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(a); r != VMResult::OK) return r;
                if (auto r = push(b); r != VMResult::OK) return r;
                break;

            case OpCode::ADD:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b + a); r != VMResult::OK) return r;
                break;

            case OpCode::SUB:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b - a); r != VMResult::OK) return r;
                break;

            case OpCode::MUL:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b * a); r != VMResult::OK) return r;
                break;

            case OpCode::DIV:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (a == 0) {
                    std::cerr << "[ERROR] Division by zero at ip=" << ip_ << "\n";
                    return VMResult::DIVISION_BY_ZERO;
                }
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b / a); r != VMResult::OK) return r;
                break;

            case OpCode::MOD:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (a == 0) return VMResult::DIVISION_BY_ZERO;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b % a); r != VMResult::OK) return r;
                break;

            case OpCode::NEG:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = push(-a); r != VMResult::OK) return r;
                break;

            case OpCode::EQ:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b == a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::NEQ:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b != a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::LT:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b < a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::LTE:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b <= a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::GT:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b > a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::GTE:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (auto r = pop(b); r != VMResult::OK) return r;
                if (auto r = push(b >= a ? 1 : 0); r != VMResult::OK) return r;
                break;

            case OpCode::JMP:
                ip_ = static_cast<size_t>(instr.operand);
                continue;

            case OpCode::JZ:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (a == 0) { ip_ = static_cast<size_t>(instr.operand); continue; }
                break;

            case OpCode::JNZ:
                if (auto r = pop(a); r != VMResult::OK) return r;
                if (a != 0) { ip_ = static_cast<size_t>(instr.operand); continue; }
                break;

            case OpCode::STORE:
                if(auto r = pop(a); r != VMResult::OK) return r;
                current_frame().locals[instr.operand] = a;
                break;

            case OpCode::LOAD:
                if(auto r = push(current_frame().locals[instr.operand]); r != VMResult::OK) return r;
                break;

            case OpCode::CALL:
                if(call_stack_.size() >= CALLSTACK_MAX) return VMResult::CALLSTACK_OVERFLOW;

                {
                    Frame f;
                    f.return_ip = ip_+1;
                    call_stack_.push_back(f);
                    ip_ = static_cast<size_t>(instr.operand);
                    continue;
                }
            
            case OpCode::RET:
                {
                    size_t ret_ip = current_frame().return_ip;
                    call_stack_.pop_back();
                    ip_ = ret_ip;
                    continue;
                }

            
            case OpCode::PRINT:
                if (auto r = pop(a); r != VMResult::OK) return r;
                std::cout << a;
                break;

            case OpCode::PRINTLN:
                if (auto r = pop(a); r != VMResult::OK) return r;
                std::cout << a << "\n";
                break;

            case OpCode::HALT:
                return VMResult::OK;

            default:
                std::cerr << "[ERROR] Unknown opcode at ip=" << ip_ << "\n";
                return VMResult::UNKNOWN_OPCODE;
        }

        ++ip_;
    }

    return VMResult::OK;
}


