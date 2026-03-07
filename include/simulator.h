#pragma once
#include <vector>
#include "types.h"

// Takes the parsed instructions and returns a history of the pipeline state at every cycle
std::vector<State> simulatePipeline(const std::vector<Instruction>& instructions);
