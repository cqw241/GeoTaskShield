#pragma once

#include "evaluation/EvaluationMetrics.h"
#include "model/ExperimentConfig.h"

#include <string>
#include <vector>

namespace gts {

struct ExperimentReportRow {
    std::string privacyName;
    std::string algorithmName;
    EvaluationMetrics metrics;
};

struct ExperimentReport {
    std::string requestText;
    SimulationConfig config;
    std::vector<ExperimentReportRow> rows;
};

} // namespace gts
