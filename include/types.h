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
};

