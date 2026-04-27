#include "simulation/DataGenerator.h"

#include <algorithm>
#include <random>

namespace gts {

SimulationData DataGenerator::generate(const SimulationConfig& config) const
{
    std::mt19937 rng(config.randomSeed);
    std::uniform_real_distribution<double> xDist(0.0, config.areaWidth);
    std::uniform_real_distribution<double> yDist(0.0, config.areaHeight);
    std::uniform_real_distribution<double> rewardDist(10.0, 100.0);
    std::uniform_real_distribution<double> deadlineDist(30.0, 300.0);
    std::uniform_real_distribution<double> speedDist(0.5, 3.0);
    std::uniform_real_distribution<double> privacyDist(0.1, 2.0);
    std::uniform_real_distribution<double> reliabilityDist(0.5, 1.0);
    std::uniform_int_distribution<int> maxTasksDist(1, 3);
    std::uniform_int_distribution<int> priorityDist(1, 5);
    std::uniform_int_distribution<int> taskTypeDist(0, 4);

    SimulationData data;
    const int workerCount = std::max(0, config.workerCount);
    const int taskCount = std::max(0, config.taskCount);
    data.workers.reserve(static_cast<std::size_t>(workerCount));
    data.tasks.reserve(static_cast<std::size_t>(taskCount));

    for (int i = 0; i < workerCount; ++i) {
        const Location location{xDist(rng), yDist(rng)};
        data.workers.push_back(Worker{
            i + 1,
            location,
            location,
            maxTasksDist(rng),
            speedDist(rng),
            privacyDist(rng),
            reliabilityDist(rng)
        });
    }

    for (int i = 0; i < taskCount; ++i) {
        data.tasks.push_back(Task{
            i + 1,
            Location{xDist(rng), yDist(rng)},
            rewardDist(rng),
            deadlineDist(rng),
            1,
            priorityDist(rng),
            static_cast<TaskType>(taskTypeDist(rng))
        });
    }

    return data;
}

} // namespace gts
