#include "../include/ui.h"
#include "../include/parser.h"
#include "../include/simulator.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace UI {
    static char code_buffer[2048] = "addi x1, x0, 100\nadd x2, x1, x3\nsw x2, x4, 4\n";
    static bool enable_forwarding = false;
    
    // cached simulation data
    static std::vector<Instruction> cached_instructions;
    static std::vector<State> cached_history;
    static bool needs_update = true; // set to true initially so it parses on startup

    // colors
    ImU32 GetStageColor(int stage, bool is_stall) {
        if (is_stall) return IM_COL32(120, 120, 120, 255); // grey for stalls
        switch(stage) {
            case FETCH:         return IM_COL32(245, 197,  66, 230); // Yellow
            case DECODE:        return IM_COL32( 74, 144, 217, 230); // Blue
            case EXECUTE:       return IM_COL32( 80, 200, 120, 230); // Green
            case MEMORY_ACCESS: return IM_COL32(255, 140,  66, 230); // Orange
            case WRITE_BACK:    return IM_COL32(231,  76,  60, 230); // Red
            default:            return IM_COL32( 50,  50,  50, 100);
        }
    }

    // short stage names
    const char* GetStageName(int stage) {
        switch(stage) {
            case FETCH: return "IF";
            case DECODE: return "ID";
            case EXECUTE: return "EX";
            case MEMORY_ACCESS: return "MEM";
            case WRITE_BACK: return "WB";
            default: return "??";
        }
    }

    void Render() {
        if (needs_update) {
            cached_instructions = parseLine(code_buffer);
            cached_history = simulatePipeline(cached_instructions);
            needs_update = false;
        }
        // main window
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("Main Workspace", nullptr, window_flags);

        // top bar
        ImGui::BeginChild("TopBar", ImVec2(0, 40), true);
        ImGui::Text("Hardware Toggles:");
        ImGui::SameLine();
        ImGui::Checkbox("Enable Data Forwarding (Bypass Paths)", &enable_forwarding);
        ImGui::EndChild();

        // left panel
        float left_width = ImGui::GetContentRegionAvail().x * 0.3f;
        ImGui::BeginChild("EditorPanel", ImVec2(left_width, 0), true);
        ImGui::Text("Assembly Editor");
        ImGui::Separator();
        
        // if the user types anything, re-simulate on the next frame
        if (ImGui::InputTextMultiline("##source", code_buffer, IM_ARRAYSIZE(code_buffer), ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput)) {
            needs_update = true;
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // right panel 
        // Added HorizontalScrollbar flag so we can scroll if the timeline gets really long
        ImGui::BeginChild("CanvasPanel", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Pipeline Execution Canvas");
        ImGui::Separator();

        if (cached_instructions.empty() || cached_history.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Type valid assembly to see the Gantt chart...");
        } else {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            
            // Grid sizing parameters
            float cell_w = 40.0f;
            float cell_h = 30.0f;
            float label_w = 160.0f;
            float header_h = 30.0f;

            int total_cycles = cached_history.back().cycle_number;
            int total_insts = cached_instructions.size();

            // To allow scrolling, we need to tell ImGui how big our custom drawing actually is
            ImGui::Dummy(ImVec2(label_w + (total_cycles * cell_w), header_h + (total_insts * cell_h)));

            // 1. DRAW HEADER (Cycle Numbers)
            for (int c = 1; c <= total_cycles; ++c) {
                float x = cursor.x + label_w + ((c - 1) * cell_w);
                std::string cycle_str = std::to_string(c);
                draw_list->AddText(ImVec2(x + (cell_w/2) - 5, cursor.y + 5), IM_COL32(200, 200, 200, 255), cycle_str.c_str());
            }

            // 2. DRAW Y-AXIS (Instruction Labels) & GRID LINES
            for (int r = 0; r < total_insts; ++r) {
                float y = cursor.y + header_h + (r * cell_h);
                
                // Draw horizontal row line
                draw_list->AddLine(ImVec2(cursor.x, y), ImVec2(cursor.x + label_w + (total_cycles * cell_w), y), IM_COL32(60, 60, 60, 255));
                
                // Draw Instruction Text
                std::string inst_text = cached_instructions[r].text;
                // Truncate if too long
                if (inst_text.size() > 20) inst_text = inst_text.substr(0, 18) + ".."; 
                draw_list->AddText(ImVec2(cursor.x + 5, y + 5), IM_COL32(255, 255, 255, 255), inst_text.c_str());
            }

            // Draw vertical cycle lines
            for (int c = 0; c <= total_cycles; ++c) {
                float x = cursor.x + label_w + (c * cell_w);
                draw_list->AddLine(ImVec2(x, cursor.y + header_h), ImVec2(x, cursor.y + header_h + (total_insts * cell_h)), IM_COL32(60, 60, 60, 255));
            }

            // 3. DRAW THE BLOCKS
            // We loop through our history array from simulator.cpp
            for (size_t c_idx = 0; c_idx < cached_history.size(); ++c_idx) {
                const State& state = cached_history[c_idx];
                int cycle_col = state.cycle_number - 1;

                // Loop through the 5 stages inside this cycle
                for (int stage = 0; stage < NUM_STAGES; ++stage) {
                    int inst_id = state.instructions[stage];
                    
                    if (inst_id != -1) {
                        // Math to find the exact top-left corner of the rectangle
                        float rect_x = cursor.x + label_w + (cycle_col * cell_w);
                        float rect_y = cursor.y + header_h + (inst_id * cell_h);
                        
                        // Detect a STALL: If the instruction was in this EXACT SAME STAGE during the previous cycle
                        bool is_stall = false;
                        if (c_idx > 0 && cached_history[c_idx - 1].instructions[stage] == inst_id) {
                            is_stall = true;
                        }

                        // Draw the colored box
                        ImU32 color = GetStageColor(stage, is_stall);
                        draw_list->AddRectFilled(ImVec2(rect_x + 2, rect_y + 2), ImVec2(rect_x + cell_w - 2, rect_y + cell_h - 2), color, 4.0f); // 4.0f gives rounded corners!

                        // Draw the text (e.g. "ID") inside the box
                        const char* stage_name = is_stall ? "WAIT" : GetStageName(stage);
                        draw_list->AddText(ImVec2(rect_x + 8, rect_y + 6), IM_COL32(0, 0, 0, 255), stage_name);
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }
}