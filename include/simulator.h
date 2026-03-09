#pragma once
#include <vector>
#include "types.h"

// Takes the parsed instructions and returns a history of the pipeline state at every cycle
SimulationResult simulatePipeline(const std::vector<Instruction>& instructions, bool enable_forwarding);
