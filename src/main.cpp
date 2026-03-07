#include <iostream>
#include <iomanip>
#include "../include/parser.h"
#include "../include/simulator.h"

int main() {
    std::string test_code = 
        "addi x1, x0, 100\n"
        "add x2, x1, x3\n"
        "sw x2, x4, 4\n";

    std::cout << "=== RISC-V Pipeline Visualizer ===\n\n";
    std::cout << "--- 1. Parsing Assembly ---\n";
    
    std::vector<Instruction> instructions = parseLine(test_code);
    for (const auto& inst : instructions) {
        std::cout << "ID " << inst.id << ": " << inst.text << "\n";
    }
    
    std::cout << "\n--- 2. Running Simulation ---\n\n";
    std::vector<State> history = simulatePipeline(instructions);
    // print
    for (const auto& state : history) {
        std::cout << "Cycle " << state.cycle_number;
        if (state.is_stalled) {
            std::cout << "  [*** STALL DETECTED ***]";
        }
        std::cout << "\n";

        // lambda function
        auto printStage = [&](const std::string& name, int stage_index) {
            std::cout << "  " << name << " : ";
            int inst_id = state.instructions[stage_index];
            if (inst_id == -1) {
                std::cout << "--- (Bubble)\n";
            } else {
                std::cout << "[" << inst_id << "] " << instructions[inst_id].text << "\n";
            }
        };

        printStage("FETCH ", FETCH);
        printStage("DECODE", DECODE);
        printStage("EXEC  ", EXECUTE);
        printStage("MEM   ", MEMORY_ACCESS);
        printStage("WB    ", WRITE_BACK);
        std::cout << "-----------------------------------\n";
    }

    std::cout << "Simulation complete in " << history.size() << " cycles.\n";
    return 0;
}