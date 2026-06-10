#include <iostream>
#include <iomanip>
#include "../include/vm.h"

// CONSTRUCTOR

VM::VM(bool debug_mode)
    : debug_mode_(debug_mode), ip_(0)   // member initializer list
    // sets debug_mode_ to whatever was passed in, sets ip_ to 0. IP always start at the first instruction.
{
    stack_.reserve(STACK_MAX);   // pre-allocated memory for 4096 elements upfront.
    // Without this, std::vector would keep resizing itself as you push values which is slow.
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

//  Main execution loop 

VMResult VM::run(const std::vector<Instruction>& program) {
    ip_ = 0;

    while (ip_ < program.size()) {
        const Instruction& instr = program[ip_];

        int64_t a = 0, b = 0;

        switch (instr.opcode) {

            case OpCode::PUSH:
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
                locals_[instr.operand] = a;
                break;

            case OpCode::LOAD:
                if(auto r = push(locals_[instr.operand]); r != VMResult::OK) return r;
                break;
            
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


