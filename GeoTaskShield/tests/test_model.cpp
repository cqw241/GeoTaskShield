#include "model/ExperimentConfig.h"
#include "model/Location.h"
#include "privacy/GridPrivacy.h"
#include "assignment/NearestGreedyAlgorithm.h"
#include "simulation/DataGenerator.h"
#include "simulation/SimulationEngine.h"
#include "tests/TestSupport.h"

#include <memory>

int main()
{
    using namespace gts;
    using namespace gts_test;

    require(near(Location{0.0, 0.0}.distanceTo(Location{3.0, 4.0}), 5.0),
            "Location distance should use Euclidean distance.");

    SimulationConfig config;
    config.workerCount = 3;
    config.taskCount = 2;
    config.randomSeed = 7;
    config.areaWidth = 100.0;
    config.areaHeight = 100.0;

    DataGenerator generator;
    const SimulationData data = generator.generate(config);
    require(data.workers.size() == 3,
            "DataGenerator should create the requested workers.");
    require(data.tasks.size() == 2,
            "DataGenerator should create the requested tasks.");

    SimulationEngine engine(
        std::make_unique<GridPrivacy>(),
        std::make_unique<NearestGreedyAlgorithm>());
    SimulationConfig runConfig;
    runConfig.workerCount = 10;
    runConfig.taskCount = 4;
    runConfig.randomSeed = 42;
    runConfig.privacy.gridSize = 10.0;
    const SimulationRunResult runResult = engine.run(runConfig);
    require(runResult.data.workers.size() == 10,
            "SimulationEngine should generate the configured number of workers.");
    require(runResult.data.tasks.size() == 4,
            "SimulationEngine should generate the configured number of tasks.");
    require(runResult.privacy.workers.size() == 10,
            "SimulationEngine should apply privacy to generated workers.");
    require(runResult.assignment.assignments.size() <= 4,
            "SimulationEngine should not assign more tasks than generated.");
    require(runResult.metrics.completionRate >= 0.0 &&
                runResult.metrics.completionRate <= 1.0,
            "SimulationEngine should produce a bounded completion rate.");

    return 0;
}
