#include "simulation/SimulationEngine.h"

#include "evaluation/MetricsCalculator.h"
#include "simulation/DataGenerator.h"

#include <stdexcept>

namespace gts {

SimulationEngine::SimulationEngine(std::unique_ptr<IPrivacyMechanism> privacyMechanism,
                                   std::unique_ptr<IAssignmentAlgorithm> assignmentAlgorithm)
    : privacyMechanism_(std::move(privacyMechanism)),
      assignmentAlgorithm_(std::move(assignmentAlgorithm))
{
    if (!privacyMechanism_ || !assignmentAlgorithm_) {
        throw std::invalid_argument("SimulationEngine requires privacy and assignment strategies.");
    }
}

SimulationRunResult SimulationEngine::run(const SimulationConfig& config) const
{
    DataGenerator generator;
    SimulationRunResult result;
    result.data = generator.generate(config);
    result.privacy = privacyMechanism_->apply(result.data.workers, config.privacy);
    result.assignment = assignmentAlgorithm_->assign(
        result.data.tasks, result.privacy.workers, config.assignment);
    result.metrics = MetricsCalculator::calculate(
        result.data.tasks,
        result.data.workers,
        result.assignment,
        result.privacy.averagePrivacyLoss);
    return result;
}

} // namespace gts
