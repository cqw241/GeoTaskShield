#pragma once

#include "model/ExperimentConfig.h"

namespace gts {

class DataGenerator {
public:
    SimulationData generate(const SimulationConfig& config) const;
};

} // namespace gts
