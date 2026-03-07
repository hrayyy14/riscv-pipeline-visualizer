#pragma once
#include <string>
#include <vector>

enum class Opcode {
    // Bubble
    NOP,
    // Register to Register
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // Register Immediate and loads
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, LW, LH, LHU, LB, LBU,
    // Store
    SW, SH, SB,
    // Branches
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    // Upper immediate
    LUI, AUIPC,
    // Jumps
    JAL, JALR
};

struct Instruction {
    int id;
    std::string text;
    Opcode opcode;
    int rd, rs1, rs2; // -1 means unused
    int imm; // 0 means unused or zero value
    
    // hazards
    // Does this instruction write data back to a destination register (rd)?
    bool writesRegister() const {
        switch (opcode) {
            case Opcode::NOP:
            case Opcode::SW: case Opcode::SH: case Opcode::SB: // Stores don't write to registers
            case Opcode::BEQ: case Opcode::BNE: case Opcode::BLT: 
            case Opcode::BGE: case Opcode::BLTU: case Opcode::BGEU: // Branches don't write
                return false;
            default:
                return true; // everything else (R-Type, I-Type, U-Type, Jumps) writes to rd
        }
    }

    // Is this a memory load?
    bool isLoad() const {
        return (opcode == Opcode::LW || opcode == Opcode::LH || 
                opcode == Opcode::LHU || opcode == Opcode::LB || opcode == Opcode::LBU);
    }

    // Does this instruction read from source register 1 (rs1)?
    bool readsRs1() const {
        switch (opcode) {
            case Opcode::NOP:
            case Opcode::LUI: case Opcode::AUIPC: // U-Types only use immediates
            case Opcode::JAL:                     // JAL only uses immediate/labels
                return false;
            default:
                return true; // almost everything else needs rs1
        }
    }

    // Does this instruction read from source register 2 (rs2)?
    bool readsRs2() const {
        switch (opcode) {
            // R-Type
            case Opcode::ADD: case Opcode::SUB: case Opcode::SLL: case Opcode::SLT:
            case Opcode::SLTU: case Opcode::XOR: case Opcode::SRL: case Opcode::SRA:
            case Opcode::OR: case Opcode::AND:
            // S-Type (Stores need rs2 for the data being saved)
            case Opcode::SW: case Opcode::SH: case Opcode::SB:
            // B-Type (Branches compare rs1 and rs2)
            case Opcode::BEQ: case Opcode::BNE: case Opcode::BLT: 
            case Opcode::BGE: case Opcode::BLTU: case Opcode::BGEU:
                return true;
            default:
                return false; // I-Types, Loads, U-Types, and Jumps do not read rs2
        }
    }
};

enum PipelineStage {
    FETCH = 0,
    DECODE = 1,
    EXECUTE = 2,
    MEMORY_ACCESS = 3,
    WRITE_BACK = 4,
    NUM_STAGES = 5
};

struct State {
    int cycle_number;
    int instructions[NUM_STAGES]; // array to hold stage to be efficient
    bool is_stalled = false;
};

