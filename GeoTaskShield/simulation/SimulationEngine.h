#pragma once

#include "assignment/IAssignmentAlgorithm.h"
#include "evaluation/EvaluationMetrics.h"
#include "privacy/IPrivacyMechanism.h"

#include <memory>

namespace gts {

struct SimulationRunResult {
    SimulationData data;
    PrivacyResult privacy;
    AssignmentResult assignment;
    EvaluationMetrics metrics;
};

class SimulationEngine {
public:
    SimulationEngine(std::unique_ptr<IPrivacyMechanism> privacyMechanism,
                     std::unique_ptr<IAssignmentAlgorithm> assignmentAlgorithm);

    SimulationRunResult run(const SimulationConfig& config) const;

private:
    std::unique_ptr<IPrivacyMechanism> privacyMechanism_;
    std::unique_ptr<IAssignmentAlgorithm> assignmentAlgorithm_;
};

} // namespace gts
