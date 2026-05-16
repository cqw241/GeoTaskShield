#include "simulation/DataGenerator.h"

#include <algorithm>
#include <random>

namespace gts {

namespace {

double sample(std::uniform_real_distribution<double>& distribution, std::mt19937& rng)
{
    return distribution(rng);
}

Location uniformLocation(std::uniform_real_distribution<double>& xDist,
                         std::uniform_real_distribution<double>& yDist,
                         std::mt19937& rng)
{
    return Location{sample(xDist, rng), sample(yDist, rng)};
}

double clamped(double value, double lower, double upper)
{
    return std::max(lower, std::min(value, upper));
}

} // namespace

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

    const bool workerShortage = config.dataProfile == "worker-shortage";
    const bool deadlineTight = config.dataProfile == "deadline-tight";
    const bool highPrivacyNoise = config.dataProfile == "high-privacy-noise";
    const bool rewardSkew = config.dataProfile == "reward-skew";
    const bool heterogeneousSpeed = config.dataProfile == "heterogeneous-speed";

    SimulationData data;
    const int workerCount = std::max(0, config.workerCount);
    const int taskCount = std::max(0, config.taskCount);
    data.workers.reserve(static_cast<std::size_t>(workerCount));
    data.tasks.reserve(static_cast<std::size_t>(taskCount));

    for (int i = 0; i < workerCount; ++i) {
        const Location location = uniformLocation(xDist, yDist, rng);
        int maxTasks = maxTasksDist(rng);
        double speed = speedDist(rng);
        double privacyBudget = privacyDist(rng);

        if (workerShortage) {
            maxTasks = 1;
            std::uniform_real_distribution<double> shortageSpeedDist(0.8, 1.8);
            speed = sample(shortageSpeedDist, rng);
        } else if (deadlineTight) {
            maxTasks = 3;
            std::uniform_real_distribution<double> tightSpeedDist(0.25, 0.65);
            speed = sample(tightSpeedDist, rng);
        } else if (highPrivacyNoise) {
            maxTasks = 3;
            std::uniform_real_distribution<double> privateBudgetDist(0.05, 0.8);
            privacyBudget = sample(privateBudgetDist, rng);
        } else if (rewardSkew) {
            maxTasks = (i % 4 == 0) ? 2 : 1;
        } else if (heterogeneousSpeed) {
            std::uniform_real_distribution<double> slowSpeedDist(0.25, 0.55);
            std::uniform_real_distribution<double> fastSpeedDist(2.5, 4.0);
            const bool fastWorker = i % 3 == 0;
            maxTasks = fastWorker ? 4 : 1;
            speed = fastWorker ? sample(fastSpeedDist, rng) : sample(slowSpeedDist, rng);
        }

        data.workers.push_back(Worker{
            i + 1,
            location,
            location,
            maxTasks,
            speed,
            privacyBudget,
            reliabilityDist(rng)
        });
    }

    for (int i = 0; i < taskCount; ++i) {
        Location location = uniformLocation(xDist, yDist, rng);
        double reward = rewardDist(rng);
        double deadline = deadlineDist(rng);

        if (deadlineTight) {
            std::uniform_real_distribution<double> tightDeadlineDist(4.0, 12.0);
            deadline = sample(tightDeadlineDist, rng);
        } else if (rewardSkew) {
            std::uniform_real_distribution<double> lowRewardDist(2.0, 12.0);
            std::uniform_real_distribution<double> highRewardDist(180.0, 420.0);
            reward = (i % 6 == 5) ? sample(highRewardDist, rng) : sample(lowRewardDist, rng);
        } else if (heterogeneousSpeed) {
            std::uniform_real_distribution<double> heterogeneousDeadlineDist(15.0, 40.0);
            deadline = sample(heterogeneousDeadlineDist, rng);
        } else if (highPrivacyNoise) {
            const double centerX = config.areaWidth * 0.5;
            const double centerY = config.areaHeight * 0.5;
            std::normal_distribution<double> clusterX(centerX, config.areaWidth * 0.16);
            std::normal_distribution<double> clusterY(centerY, config.areaHeight * 0.16);
            location = Location{
                clamped(clusterX(rng), 0.0, config.areaWidth),
                clamped(clusterY(rng), 0.0, config.areaHeight)
            };
        }

        data.tasks.push_back(Task{
            i + 1,
            location,
            reward,
            deadline,
            1,
            priorityDist(rng),
            static_cast<TaskType>(taskTypeDist(rng))
        });
    }

    return data;
}

} // namespace gts
