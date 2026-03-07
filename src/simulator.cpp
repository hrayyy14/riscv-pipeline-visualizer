#include "../include/simulator.h"

std::vector<State> simulatePipeline(const std::vector<Instruction>& instructions) {
    std::vector<State> history;
    State current_state;
    current_state.cycle_number = 1;

    for (int i = 0; i < NUM_STAGES; i++) {
        current_state.instructions[i] = -1;
    }

    int pc = 0; // program counter to track which instruction we are fetching
    int total_instructions = instructions.size();

    // CLOCK CYCLE LOOP 
    // UNTIL ALL INSTUCTIONS ARE FETCHED AND THE PIPELINE IS EMPTY
    while (true) {
        // save the current state
        history.push_back(current_state);
        // prepare next state
        State next_state;
        next_state.cycle_number = current_state.cycle_number + 1;
        // move pipeline
        for (int i = WRITE_BACK; i > FETCH; --i) {
            next_state.instructions[i] = current_state.instructions[i - 1];
        }
        // FETCH 
        if (pc < total_instructions) {
            next_state.instructions[FETCH] = instructions[pc].id;
            pc++;
        } else {
            next_state.instructions[FETCH] = -1;
        }
        // update
        current_state = next_state;
        // complete ?
        bool is_empty = true;
        for (int i = 0; i < NUM_STAGES; i++) {
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