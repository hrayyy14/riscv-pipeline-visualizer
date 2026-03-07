cmake_minimum_required(VERSION 3.10)
project(PipelineVisualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define our executable and the source files it needs
add_executable(Visualizer
    src/main.cpp
    src/parser.cpp
    src/simulator.cpp
)

# Tell the compiler where to find our header files
target_include_directories(Visualizer PRIVATE include)