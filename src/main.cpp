#include <iostream>
#include "../include/vm.h"
#include "../assembler/assembler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: helios <file.hasm>\n";
        return 1;
    }

    // example: ./helios examples/factorial.hasm --debug
    // argc = 3
    // argv[0] = "./helios"
    // argv[1] = "examples/factorial.hasm"
    // argv[2] = "--debug"

    bool debug = false;
    for (int i = 2; i < argc; i++)
        if (std::string(argv[i]) == "--debug") debug = true;

    try {
        Assembler assembler;
        std::vector<Instruction> program = assembler.assemble(argv[1]);

        VM vm(debug);
        VMResult result = vm.run(program);

        if (result != VMResult::OK)
            std::cerr << "VM error: " << static_cast<int>(result) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}