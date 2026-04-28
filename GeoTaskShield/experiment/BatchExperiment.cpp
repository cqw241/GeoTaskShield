#include "experiment/BatchExperiment.h"

#include "assignment/AssignmentAlgorithmFactory.h"
#include "privacy/PrivacyFactory.h"
#include "simulation/SimulationEngine.h"

#include <stdexcept>
#include <utility>

namespace gts {

BatchExperimentResult BatchExperimentRunner::run(
    const std::vector<ExperimentScenario>& scenarios) const
{
    BatchExperimentResult result;
    result.rows.reserve(scenarios.size());

    for (const ExperimentScenario& scenario : scenarios) {
        auto privacy = PrivacyFactory::create(scenario.privacyType);
        auto algorithm = AssignmentAlgorithmFactory::create(scenario.algorithmType);
        if (!privacy || !algorithm) {
            throw std::runtime_error("BatchExperimentRunner could not create requested strategies.");
        }

        const std::string privacyName = privacy->name();
        const std::string algorithmName = algorithm->name();
        SimulationEngine engine(std::move(privacy), std::move(algorithm));
        const SimulationRunResult runResult = engine.run(scenario.config);

        result.rows.push_back(BatchExperimentRow{
            scenario.name,
            privacyName,
            algorithmName,
            scenario.config,
            runResult.metrics
        });
    }

    return result;
}

} // namespace gts
