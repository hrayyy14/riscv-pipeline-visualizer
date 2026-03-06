# Project Timeline: 5-Stage RISC-V Pipeline Visualizer

**Goal:** Build a real-time, interactive CPU pipeline visualizer in C++ with Dear ImGui, compiled to WebAssembly for seamless browser access via GitHub Pages.
**Estimated Timeframe:** 4 to 5 Weeks (assuming part-time/weekend development).

## Week 1: The Engine Room (Core C++ & Logic)

_Focus: Build the simulator completely independent of any graphics or UI. It should run in the terminal._

- [ ] Initialize Git repository, create `.gitignore`, and set up the `include/` and `src/` folder structure.
- [ ] Define shared structs in `types.h` (`Instruction`, `CycleState`, `Opcode`).
- [ ] Build the **Parser** (`parser.cpp`): Write a function that takes a hardcoded string like `"add x1, x2, x3"` and converts it into your `Instruction` struct.
- [ ] Build the **Simulator** (`simulator.cpp`): Implement the 5-stage loop (IF, ID, EX, MEM, WB).
- [ ] Implement basic Data Hazard detection (stalls/bubbles).
- [ ] Write a simple `main.cpp` that prints the cycle-by-cycle output to the terminal to verify the math is correct.

## Week 2: The Canvas (CMake & Native UI)

_Focus: Get a blank Dear ImGui window rendering on your native OS (Windows/Mac/Linux)._

- [ ] Add Dear ImGui and GLFW as Git submodules in the `vendor/` folder.
- [ ] Write the initial `CMakeLists.txt` to link C++, ImGui, and OpenGL3.
- [ ] Update `main.cpp` to initialize GLFW, OpenGL, and the Dear ImGui context.
- [ ] Create the split-screen layout in `ui.cpp`: Left panel (Text Input), Right panel (Blank Canvas), Top panel (Toggles).
- [ ] Verify you can type in the ImGui text box and click a dummy checkbox.

## Week 3: Wiring It Together (Data $\rightarrow$ Visuals)

_Focus: Connect the text editor to your Week 1 engine, and draw the basic Gantt chart._

- [ ] Connect the ImGui text input buffer to your Parser. Trigger a re-parse and re-simulation only when the text changes.
- [ ] Pass the simulator's output (`std::vector<CycleState>`) to your UI renderer.
- [ ] Use `ImGui::GetWindowDrawList()->AddRectFilled()` to draw colored blocks on the right panel based on the cycle states (e.g., IF = Yellow, ID = Blue).
- [ ] Implement grid lines and basic X/Y axis labels (Instruction text on Y, Cycle numbers on X).

## Week 4: The "Wow" Factor (Forwarding & Polish)

_Focus: Add the advanced hardware toggles and the visual polish that will impress recruiters._

- [ ] Implement Data Forwarding logic in `simulator.cpp`.
- [ ] Connect the "Enable Forwarding" UI checkbox to the simulator.
- [ ] Ensure the simulator records `ForwardingEvents` (where data bypassed the pipeline).
- [ ] Use `ImGui::GetWindowDrawList()->AddBezierCubic()` to draw sleek, curved arrows showing data forwarding visually on the Gantt chart.
- [ ] Polish colors, padding, and text visibility.

## Week 5: The Holy Grail (WebAssembly Deployment)

_Focus: Port the native desktop app to the web so anyone can view it with zero friction._

- [ ] Install Emscripten (emsdk) on your local machine.
- [ ] Modify `CMakeLists.txt` with conditional logic (`if(EMSCRIPTEN)`) to build for the web instead of native desktop.
- [ ] Wrap your native `main.cpp` loop inside `emscripten_set_main_loop()` so it runs correctly in a browser.
- [ ] Test the compiled `.html` and `.wasm` files locally.
- [ ] Write the `.github/workflows/deploy.yml` GitHub Action to automatically compile to Wasm and push to GitHub Pages every time you commit to `main`.
- [ ] Update the `README.md` with the live GitHub Pages link and a GIF of the app.

---

Would you like me to draft the conditional `CMakeLists.txt` logic that allows your project to compile for _both_ native desktop and Emscripten WebAssembly seamlessly?
