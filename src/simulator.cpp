#include "../include/simulator.h"
#include <iostream>

// hazard detection
bool detectDataHazard(const State& state, const std::vector<Instruction>& instructions, bool enable_forwarding) {
    int id_id = state.instructions[DECODE];
    if (id_id == -1) return false; // empty so no hazard

    const Instruction& inst_in_id = instructions[id_id];
    if (!enable_forwarding) {
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
    } else {
        // only need to stall for a Load-Use hazard
        // happens when the instruction immediately ahead in EX is a Load
        // and we need its data right now in EX (but it won't be ready until MEM)
        int older_inst_id = state.instructions[EXECUTE];
        if (older_inst_id != -1) {
            const Instruction& older_inst = instructions[older_inst_id];
            if (older_inst.isLoad() && older_inst.rd > 0) {
                if (inst_in_id.readsRs1() && older_inst.rd == inst_in_id.rs1) return true;
                if (inst_in_id.readsRs2() && older_inst.rd == inst_in_id.rs2) return true;
            }
        }
        return false;
    }
}

SimulationResult simulatePipeline(const std::vector<Instruction>& instructions, bool enable_forwarding) {
    SimulationResult result;
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

        result.history.push_back(current_state);
        // FORWARDING LOGIC
        if (enable_forwarding) {
            int ex_id = current_state.instructions[EXECUTE];
            if (ex_id != -1) {
                const Instruction& ex_inst = instructions[ex_id];
                
                // from MEM (EX/MEM pipeline register)
                int mem_id = current_state.instructions[MEMORY_ACCESS];
                bool forwarded_rs1 = false;
                bool forwarded_rs2 = false;

                if (mem_id != -1) {
                    const Instruction& mem_inst = instructions[mem_id];
                    if (mem_inst.writesRegister() && mem_inst.rd > 0) {
                        if (ex_inst.readsRs1() && mem_inst.rd == ex_inst.rs1) {
                            result.forwarding_events.push_back({current_state.cycle_number, mem_id, ex_id, MEMORY_ACCESS});
                            forwarded_rs1 = true;
                        }
                        if (ex_inst.readsRs2() && mem_inst.rd == ex_inst.rs2) {
                            result.forwarding_events.push_back({current_state.cycle_number, mem_id, ex_id, MEMORY_ACCESS});
                            forwarded_rs2 = true;
                        }
                    }
                }

                // from WB (MEM/WB pipeline register)
                // only forward if MEM didn't already provide the more recent data
                int wb_id = current_state.instructions[WRITE_BACK];
                if (wb_id != -1) {
                    const Instruction& wb_inst = instructions[wb_id];
                    if (wb_inst.writesRegister() && wb_inst.rd > 0) {
                        if (!forwarded_rs1 && ex_inst.readsRs1() && wb_inst.rd == ex_inst.rs1) {
                            result.forwarding_events.push_back({current_state.cycle_number, wb_id, ex_id, WRITE_BACK});
                        }
                        if (!forwarded_rs2 && ex_inst.readsRs2() && wb_inst.rd == ex_inst.rs2) {
                            result.forwarding_events.push_back({current_state.cycle_number, wb_id, ex_id, WRITE_BACK});
                        }
                    }
                }
            }
        }

        State next_state;
        next_state.cycle_number = current_state.cycle_number + 1;

        // hazards
        bool stall = detectDataHazard(current_state, instructions, enable_forwarding);
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
    return result;
}