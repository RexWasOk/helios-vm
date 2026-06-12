#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "assembler.h"

/*
 * =============================================================
 *  Helios VM — assembler.cpp
 * =============================================================
 *
 *  DAY 4 — Two-Pass Assembler
 *  ---------------------------
 *  Problem: writing programs as raw C++ vectors of Instructions
 *  with hardcoded ip numbers is painful. One inserted instruction
 *  shifts every address and breaks every jump. We need a way to
 *  write human-readable assembly and let the assembler figure out
 *  all the addresses automatically.
 *
 *  Solution: a two-pass assembler that reads .hasm text files and
 *  converts them into a vector<Instruction> the VM can execute.
 *
 *  ── .hasm file format ────────────────────────────────────────
 *  ; lines starting with ; are comments — skipped entirely
 *  labels:   lines ending with : define a named address
 *  instructions: opcode followed by optional operand
 *
 *  Example:
 *      main:
 *          PUSH 5
 *          CALL factorial   ; label reference, not a number
 *          PRINTLN
 *          HALT
 *
 *  ── Pass 1: label collection ─────────────────────────────────
 *  Scan every line. Skip blank lines and comments. When a label
 *  is found (line ends with ':'), strip the ':' and record it in
 *  an unordered_map<string, size_t> with its current ip address.
 *  When a real instruction is found, increment the ip counter.
 *  Labels themselves don't occupy an ip slot — only real
 *  instructions do.
 *
 *  After pass 1, labels_ contains something like:
 *      "main"       → 0
 *      "factorial"  → 5
 *      "base_case"  → 17
 *
 *  ── Pass 2: instruction emission ─────────────────────────────
 *  Scan every line again. Skip blank lines, comments, and labels.
 *  For each instruction line:
 *    1. Wrap the line in std::istringstream to read token by token
 *    2. Read first token → opcode string → convert via parse_opcode()
 *    3. Try to read second token → operand
 *       - If operand is a label name (found in labels_ map) →
 *         use its resolved ip address as the operand
 *       - If operand is a number → convert with std::stoll()
 *       - If no operand → create Instruction with opcode only
 *
 *  ── Why unordered_map for labels? ────────────────────────────
 *  Label lookup happens potentially thousands of times during
 *  assembly. unordered_map gives O(1) average lookup via hashing
 *  versus O(log n) for ordered map. Label order doesn't matter —
 *  we only care about name → address lookup speed.
 *
 *  ── Why std::istringstream? ──────────────────────────────────
 *  istringstream wraps a string and lets us read it token by token
 *  using the >> operator, exactly like std::cin but from a string
 *  instead of keyboard input. So "CALL factorial" gives us "CALL"
 *  on first >> and "factorial" on second >>.
 *
 *  ── Why two passes and not one? ──────────────────────────────
 *  When assembling line by line in a single pass, a forward
 *  reference like "JNZ base_case" can't be resolved because
 *  "base_case" is defined further down the file and hasn't been
 *  seen yet. Pass 1 solves this by pre-scanning all labels before
 *  any instructions are emitted.
 *
 *  ── Helper functions ─────────────────────────────────────────
 *  trim()        — strips leading/trailing whitespace from a line
 *                  using find_first_not_of and find_last_not_of
 *  is_label()    — returns true if line ends with ':'
 *  is_comment()  — returns true if line starts with ';'
 *  parse_opcode()— converts opcode string to OpCode enum value,
 *                  throws runtime_error for unknown opcodes
 *
 *  ── Tested with ──────────────────────────────────────────────
 *  examples/factorial.hasm  → factorial(5)   = 120
 *  examples/fibonacci.hasm  → fibonacci(10)  = 55
 * =============================================================
 */

// HELPERS

// remove leading and trailing whitespace from a line, extracting the main content.
std::string Assembler::trim(const std::string& line){

    size_t start = line.find_first_not_of(" \t\r\n");    // scans from the left, finds the first character that is NOT a space, tab, carriage return, or newline. That's where the real content starts.
    if(start == std::string::npos) return ""; // npos means "not found". If every character was whitespace, the line is empty so return empty string.
    
    size_t end = line.find_last_not_of(" \t\r\n"); // same thing but from the right. Finds where content ends.

    return line.substr(start, end-start+1); // extract just the content between start and end. The +1 is because substr takes a length not an end index.
}

// a label is any line ending with ':'
bool Assembler::is_label(const std::string& line) {
    return !line.empty() && line.back() == ':';           // the line empty check prevents crashing on empty lines.
}

// a comment is any line starting with ';'
bool Assembler::is_comment(const std::string& line) {
    return !line.empty() && line.front() == ';';
}

// convert string like "PUSH" to OpCode::PUSH
OpCode Assembler::parse_opcode(const std::string& token) {
    if (token == "PUSH")    return OpCode::PUSH;
    if (token == "POP")     return OpCode::POP;
    if (token == "DUP")     return OpCode::DUP;
    if (token == "SWAP")    return OpCode::SWAP;
    if (token == "ADD")     return OpCode::ADD;
    if (token == "SUB")     return OpCode::SUB;
    if (token == "MUL")     return OpCode::MUL;
    if (token == "DIV")     return OpCode::DIV;
    if (token == "MOD")     return OpCode::MOD;
    if (token == "NEG")     return OpCode::NEG;
    if (token == "EQ")      return OpCode::EQ;
    if (token == "NEQ")     return OpCode::NEQ;
    if (token == "LT")      return OpCode::LT;
    if (token == "LTE")     return OpCode::LTE;
    if (token == "GT")      return OpCode::GT;
    if (token == "GTE")     return OpCode::GTE;
    if (token == "JMP")     return OpCode::JMP;
    if (token == "JZ")      return OpCode::JZ;
    if (token == "JNZ")     return OpCode::JNZ;
    if (token == "CALL")    return OpCode::CALL;
    if (token == "RET")     return OpCode::RET;
    if (token == "LOAD")    return OpCode::LOAD;
    if (token == "STORE")   return OpCode::STORE;
    if (token == "PRINT")   return OpCode::PRINT;
    if (token == "PRINTLN") return OpCode::PRINTLN;
    if (token == "HALT")    return OpCode::HALT;
    throw std::runtime_error("Unknown opcode: " + token);  // throw immediately stops the function and sends the error up to whoever called assemble().

}

// TWO PASS ASSEMBLER

std::vector<Instruction> Assembler::assemble(const std::string& filename){

    std::ifstream file(filename);
    if(!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);

    // read all lines into memory once
    std::vector<std::string> lines;
    std::string line;

    while(std::getline(file, line)) lines.push_back(trim(line));

    // PASS-1: COLLECT LABELS

    size_t ip=0;
    for(const auto&l : lines){

        if(l.empty() || is_comment(l)) continue;

        if(is_label(l)){
            // strip the ':' and store label → current ip
            std::string name = l.substr(0, l.size()-1);
            labels_[name] = ip;
        }

        else{
            // it's a real instruction, increment ip counter. Labels themselves don't get an ip, only real instructions do.
            ip++;
        }
    }

    /*
    After pass 1, labels_ contains something like:
    "main"       → 0
    "factorial"  → 5
    "base_case"  → 17
    */

    // PASS 2: EMIT INSTRUCTIONS

    std::vector<Instruction> program;
    for(const auto& l : lines){

        if (l.empty() || is_comment(l) || is_label(l)) continue;

        std::istringstream ss(l);
        std::string token;
        // wraps the line string into a stream so we can read it token by token using >>.
        // Think of it like cin but reading from a string instead of keyboard. So for line "PUSH 5", ss >> token gives us "PUSH", then ss >> operand_str gives us "5".

        ss>>token;  // reads the first word (the opcode string).

        OpCode op = parse_opcode(token);

        std::string operand_str;
        if(ss >> operand_str){      //tries to read a second token. If it succeeds there's an operand. If it fails (nothing left on the line) there's no operand.
           
           if(labels_.count(operand_str)){          // checking if the operand is a label name or a normal operand.
            program.push_back(Instruction(op, labels_[operand_str]));
           }

           else{
            program.push_back(Instruction(op, std::stoll(operand_str)));
           }
        }

        else{
            program.push_back(Instruction(op));
        }
    }

    return program;

}






