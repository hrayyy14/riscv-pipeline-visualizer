# 5-Stage RISC-V Pipeline Visualizer: Project Blueprint

## 1. Project Setup & Version Control

Treat this like a production application from day one. A clean repository is the first thing recruiters will notice.

### GitHub & Git Setup

1. **Initialize the Repo:** Create a new local folder and run `git init`.
2. **Create `.gitignore`:** Use a standard C++ `.gitignore` (ignore `build/`, `.vscode/`, `.vs/`, `*.o`, `*.exe`, `*.out`).
3. **Public Repository:** Create a public repo on GitHub. Add an engaging `README.md` (you will update this later with a GIF of the working visualizer).
4. **Push:** Connect your local repo to GitHub and push the initial commit.

### Dependency Management (The `vendor` folder)

Instead of installing libraries globally, keep them self-contained using Git submodules.

- **Dear ImGui:** Run `git submodule add https://github.com/ocornut/imgui.git vendor/imgui`
- **GLFW:** (If not using system packages) `git submodule add https://github.com/glfw/glfw.git vendor/glfw`

---

## 2. Directory Structure

A scalable C++ project separates headers, source files, and third-party dependencies. Set up your folder tree exactly like this:

```text
PipelineVisualizer/
├── .gitignore
├── README.md
├── INSTRUCTIONS.md
├── CMakeLists.txt
├── vendor/                 # Third-party libraries (ImGui, GLFW)
├── include/                # Project header files (.h / .hpp)
│   ├── types.h             # Global structs and enums
│   ├── parser.h            # String-to-instruction logic
│   ├── simulator.h         # CPU pipeline logic
│   └── ui.h                # Dear ImGui rendering logic
└── src/                    # Project source files (.cpp)
    ├── main.cpp            # Entry point & window initialization
    ├── parser.cpp
    ├── simulator.cpp
    └── ui.cpp

```

---

## 3. CMake Configuration Strategy

Your `CMakeLists.txt` is the glue. It needs to do three things:

1. Define the executable (`add_executable`).
2. Include the source files from `src/` and the necessary ImGui files from `vendor/imgui/` (specifically the core files and the OpenGL3/GLFW backend files).
3. Link the required libraries (OpenGL, GLFW) and tell the compiler where your `include/` and `vendor/` folders are (`target_include_directories`).

---

## 4. Architectural Thinking (The Mental Model)

The most common mistake in GUI applications is "spaghetti code" where the UI logic is tangled up with the business logic. **You must strictly decouple your components.**

Adopt a **Unidirectional Data Flow**:
`Text Input` $\rightarrow$ `Parser` $\rightarrow$ `Simulator` $\rightarrow$ `UI Renderer`

- The **Parser** should only know about strings and outputting generic instructions.
- The **Simulator** should know _nothing_ about ImGui, windows, or user text. It just takes `Instructions` and outputs `CycleStates`.
- The **UI** takes those `CycleStates` and draws shapes on the screen.

---

## 5. Essential Files Breakdown

### `include/types.h` (The Shared Language)

Keep all your shared data structures here to avoid circular dependency nightmares.

- **`enum class Opcode`**: `ADD`, `SUB`, `LW`, `SW`, etc.
- **`struct Instruction`**: Holds the `Opcode`, `rd`, `rs1`, `rs2`, and the original string text.
- **`struct CycleState`**: Represents the state of the pipeline at one specific clock cycle. It needs to track which instruction is in IF, ID, EX, MEM, and WB.
- **`struct ForwardingEvent`**: Tracks from which stage to which stage data was passed, so the UI can draw the arrows later.

### `src/parser.cpp` (The Translator)

- **Input:** `std::string` (raw assembly from the ImGui text box).
- **Output:** `std::vector<Instruction>`
- **Logic:** Split the string by newlines, ignore empty lines, tokenize by commas/spaces, and map the strings ("x1", "add") to your `types.h` enums and structs.

### `src/simulator.cpp` (The Brains)

- **Input:** `std::vector<Instruction>`, `bool use_forwarding`
- **Output:** `std::vector<CycleState>`
- **Logic:** A `while` loop representing the clock cycle.
- Shift instructions down the pipeline stages.
- Check for data hazards (e.g., does the instruction in ID need `rs1` which is currently being written by the instruction in EX?).
- Apply stalls (bubbles) or forwarding based on the boolean toggle.
- Save the snapshot of the pipeline to your output vector at the end of every loop iteration.

### `src/ui.cpp` (The Canvas)

- **Input:** `std::vector<CycleState>` from the simulator.
- **Logic:** \* `RenderLeftPanel()`: Holds `ImGui::InputTextMultiline` and the hardware toggles (Forwarding ON/OFF).
- `RenderRightPanel()`: Uses `ImGui::GetWindowDrawList()` to draw the Gantt chart. Loops through the `CycleState` history and uses `AddRectFilled` to draw the colored stage boxes and `AddBezierCubic` for the forwarding arrows.

### `src/main.cpp` (The Engine Room)

- Initializes GLFW and an OpenGL context.
- Initializes Dear ImGui.
- Contains the main `while (!glfwWindowShouldClose(window))` loop.
- Inside the loop: Polls events, checks if the text or toggles changed, calls the Parser and Simulator _only if the input changed_, and then calls your UI rendering functions.

---

Would you like me to write the exact `CMakeLists.txt` for this specific architecture, or would you prefer I draft the C++ boilerplate for `include/types.h` so you can see how the data structures map together?
