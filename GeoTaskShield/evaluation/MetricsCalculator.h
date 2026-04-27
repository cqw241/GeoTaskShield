#pragma once

#include "assignment/IAssignmentAlgorithm.h"
#include "evaluation/EvaluationMetrics.h"

namespace gts {

class MetricsCalculator {
public:
    static EvaluationMetrics calculate(const std::vector<Task>& tasks,
                                       const std::vector<Worker>& workers,
                                       const AssignmentResult& assignment,
                                       double averagePrivacyLoss);
};

} // namespace gts
