#include <iostream>
#include "../include/parser.h"

// function to print Opcode names instead of numbers
std::string getOpcodeName(Opcode op) {
    switch(op) {
        case Opcode::ADD:  return "ADD";
        case Opcode::ADDI: return "ADDI";
        case Opcode::BEQ:  return "BEQ";
        case Opcode::LW:   return "LW";
        case Opcode::SW:   return "SW";
        case Opcode::NOP:  return "NOP";
        default:           return "UNKNOWN";
    }
}

int main() {
    std::string test_code = 
        "addi x1, x0, 100\n"
        "add x2, x1, x3\n"
        "sw x2, x1, 4\n";

    std::cout << "--- RISC-V Parser Test ---\n\n";

    std::vector<Instruction> instructions = parseLine(test_code);

    // Loop through the output and print data
    for (const auto& inst : instructions) {
        std::cout << "ID: " << inst.id 
                  << " | Text: '" << inst.text << "'\n"
                  << "    -> Opcode: " << getOpcodeName(inst.opcode)
                  << ", rd: " << inst.rd 
                  << ", rs1: " << inst.rs1 
                  << ", rs2: " << inst.rs2 
                  << ", imm: " << inst.imm << "\n\n";
    }

    return 0;
}