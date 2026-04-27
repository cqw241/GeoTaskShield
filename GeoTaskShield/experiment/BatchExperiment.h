#pragma once

#include "evaluation/EvaluationMetrics.h"
#include "model/ExperimentConfig.h"

#include <string>
#include <vector>

namespace gts {

struct ExperimentScenario {
    std::string name;
    SimulationConfig config;
    std::string privacyType{"grid"};
    std::string algorithmType{"nearest"};
};

struct BatchExperimentRow {
    std::string scenarioName;
    std::string privacyName;
    std::string algorithmName;
    SimulationConfig config;
    EvaluationMetrics metrics;
};

struct BatchExperimentResult {
    std::vector<BatchExperimentRow> rows;
};

class BatchExperimentRunner {
public:
    BatchExperimentResult run(const std::vector<ExperimentScenario>& scenarios) const;
};

} // namespace gts
