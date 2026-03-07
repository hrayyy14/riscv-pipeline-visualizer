#include "../include/simulator.h"
#include <iostream>

// Helper function to detect Data Hazards (RAW - Read After Write)
bool detectDataHazard(const State& state, const std::vector<Instruction>& instructions) {
    int id_id = state.instructions[DECODE];
    if (id_id == -1) return false; // empty so no hazard

    const Instruction& inst_in_id = instructions[id_id];
    int future_stages[] = {EXECUTE, MEMORY_ACCESS, WRITE_BACK};

    for (int stage : future_stages) {
        int older_inst_id = state.instructions[stage];
        
        if (older_inst_id != -1) {
            const Instruction& older_inst = instructions[older_inst_id];
            
            // If the older instruction writes to a valid register (not x0)
            if (older_inst.writesRegister() && older_inst.rd > 0) {
                
                // STALL if the current instruction needs that exact register
                if (inst_in_id.readsRs1() && older_inst.rd == inst_in_id.rs1) return true;
                if (inst_in_id.readsRs2() && older_inst.rd == inst_in_id.rs2) return true;
            }
        }
    }
    return false;
}

std::vector<State> simulatePipeline(const std::vector<Instruction>& instructions) {
    std::vector<State> history;
    State current_state;
    current_state.cycle_number = 1;
    current_state.is_stalled = false;

    for (int i = 0; i < NUM_STAGES; i++) {
        current_state.instructions[i] = -1;
    }

    int pc = 0; // program counter to track which instruction we are fetching
    int total_instructions = instructions.size();

    // CLOCK CYCLE LOOP 
    // UNTIL ALL INSTUCTIONS ARE FETCHED AND THE PIPELINE IS EMPTY
    while (true) {
        // safety
        if (current_state.cycle_number > total_instructions * 10 + 50) {
            std::cerr << "\n[ERROR] Simulator hit safety limit. Infinite stall detected. Aborting.\n";
            break; 
        }

        history.push_back(current_state);

        State next_state;
        next_state.cycle_number = current_state.cycle_number + 1;

        // hazards
        bool stall = detectDataHazard(current_state, instructions);
        next_state.is_stalled = stall;

        if (stall) {
            // bottom half drains
            next_state.instructions[WRITE_BACK] = current_state.instructions[MEMORY_ACCESS];
            next_state.instructions[MEMORY_ACCESS] = current_state.instructions[EXECUTE];
            
            // bubble into execute
            next_state.instructions[EXECUTE] = -1; 
            
            // decode and fetch stay the same way 
            next_state.instructions[DECODE] = current_state.instructions[DECODE];
            next_state.instructions[FETCH] = current_state.instructions[FETCH];
        } 
        else {
            // no stall
            for (int i = WRITE_BACK; i > FETCH; --i) {
                next_state.instructions[i] = current_state.instructions[i - 1];
            }

            // fetch new instruction
            if (pc < total_instructions) {
                next_state.instructions[FETCH] = instructions[pc].id;
                pc++;
            } else {
                next_state.instructions[FETCH] = -1;
            }
        }
        current_state = next_state;

        // completition
        bool is_empty = true;
        for (int i = 0; i < NUM_STAGES; ++i) {
            if (current_state.instructions[i] != -1) {
                is_empty = false; 
                break;
            }
        }
        if (is_empty && pc >= total_instructions) {
            break; 
        }
    }
    return history;
}