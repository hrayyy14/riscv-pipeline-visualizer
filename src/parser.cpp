#include "../include/parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <unordered_map>

// try and catch in the parser functions so the vis does not crash
// REVIEW later

int parseRegister (std::string reg) {
    reg.erase(std::remove(reg.begin(), reg.end(), ','), reg.end());
    reg.erase(std::remove(reg.begin(), reg.end(), ' '), reg.end());
    if (!reg.empty() && reg[0] == 'x') {
        try {
            return std::stoi(reg.substr(1));
        } catch (...) {
            return -1; 
        }
    }
    return -1;
}

int parseImmediate (std::string imm) {
    imm.erase(std::remove(imm.begin(), imm.end(), ','), imm.end());
    if (imm.empty()) return 0;
    try {
        return std::stoi(imm);
    } catch (...) {
        return 0; 
    }
}

static const std::unordered_map<std::string, Opcode> name_map = {
    {"add",  Opcode::ADD},
    {"addi", Opcode::ADDI},
    {"beq",  Opcode::BEQ},
    {"lw",   Opcode::LW},
    {"sw",   Opcode::SW}
    // ADD HERE THE OTHER 32 INSTRUCTIONS
};

std::vector<Instruction> parseLine(const std::string& source) {
    std::vector<Instruction> instructions;
    std::istringstream stream(source);
    std::string line;
    int current_id = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::istringstream line_stream(line);
        std::string name;
        line_stream >> name; // works as cin >> x; but pulls from line

        Instruction inst;
        inst.id = current_id++;
        inst.text = line;
        // initialize to -1
        inst.rd = -1;
        inst.rs1 = -1;
        inst.rs2 = -1;
        inst.imm = 0;
        inst.opcode = Opcode::NOP; // default to NOP

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        auto it = name_map.find(name);
        if (it != name_map.end()) {
            inst.opcode = it->second;
        }

        switch (inst.opcode) {
            case Opcode::ADD: { 
                // R-Type format (rd, rs1, rs2)
                std::string rd, rs1, rs2;
                line_stream >> rd >> rs1 >> rs2;
                inst.rd = parseRegister(rd);
                inst.rs1 = parseRegister(rs1);
                inst.rs2 = parseRegister(rs2);
                break;
            }
            case Opcode::ADDI: 
            case Opcode::LW: {  
                // I-TypefFormat (rd, rs1, imm)
                std::string rd, rs1, imm;
                line_stream >> rd >> rs1 >> imm;
                inst.rd = parseRegister(rd);
                inst.rs1 = parseRegister(rs1);
                inst.imm = parseImmediate(imm);
                break;
            }
            case Opcode::SW:   
            case Opcode::BEQ: { 
                // S-Type & B-Type format (rs1, rs2, imm)
                std::string rs1, rs2, imm;
                line_stream >> rs1 >> rs2 >> imm;
                inst.rs1 = parseRegister(rs1);
                inst.rs2 = parseRegister(rs2);
                inst.imm = parseImmediate(imm);
                break;
            }
            default:
                break; // NOP 
        }
        instructions.push_back(inst);
    }
    return instructions;
}