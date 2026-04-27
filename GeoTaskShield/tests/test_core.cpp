#include "model/Location.h"
#include "model/Task.h"
#include "model/Worker.h"
#include "model/ExperimentConfig.h"
#include "simulation/DataGenerator.h"
#include "privacy/GridPrivacy.h"
#include "privacy/KAnonymityPrivacy.h"
#include "privacy/LaplaceNoisePrivacy.h"
#include "privacy/PrivacyFactory.h"
#include "assignment/NearestGreedyAlgorithm.h"
#include "assignment/ScoreGreedyAlgorithm.h"
#include "assignment/HungarianAlgorithm.h"
#include "assignment/AssignmentAlgorithmFactory.h"
#include "evaluation/MetricsCalculator.h"
#include "simulation/SimulationEngine.h"
#include "data/CsvExporter.h"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(double lhs, double rhs, double tolerance = 1e-9)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

} // namespace

int main()
{
    using namespace gts;

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
    require(data.workers.size() == 3, "DataGenerator should create the requested workers.");
    require(data.tasks.size() == 2, "DataGenerator should create the requested tasks.");

    std::vector<Worker> workers{
        Worker{1, Location{12.0, 18.0}, Location{}, 2, 1.0, 1.0, 0.9},
        Worker{2, Location{71.0, 73.0}, Location{}, 1, 1.0, 1.0, 0.8}
    };
    GridPrivacy gridPrivacy;
    PrivacyContext privacyContext;
    privacyContext.gridSize = 10.0;
    const PrivacyResult privacyResult = gridPrivacy.apply(workers, privacyContext);
    require(near(privacyResult.workers[0].exposedLocation.x, 15.0),
            "GridPrivacy should expose the grid center x coordinate.");
    require(near(privacyResult.workers[0].exposedLocation.y, 15.0),
            "GridPrivacy should expose the grid center y coordinate.");
    require(privacyResult.averagePrivacyLoss > 0.0,
            "GridPrivacy should report an average obfuscation distance.");

    std::vector<Worker> anonymityWorkers{
        Worker{1, Location{0.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.9},
        Worker{2, Location{2.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.8},
        Worker{3, Location{10.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.7}
    };
    KAnonymityPrivacy kAnonymity;
    PrivacyContext anonymityContext;
    anonymityContext.k = 2;
    const PrivacyResult anonymityResult =
        kAnonymity.apply(anonymityWorkers, anonymityContext);
    require(anonymityResult.workers.size() == 3,
            "KAnonymityPrivacy should keep the worker count.");
    require(near(anonymityResult.workers[0].exposedLocation.x, 1.0) &&
                near(anonymityResult.workers[0].exposedLocation.y, 0.0),
            "KAnonymityPrivacy should expose the centroid of the nearest k workers.");
    require(anonymityResult.averagePrivacyLoss > 0.0,
            "KAnonymityPrivacy should report an average obfuscation distance.");

    LaplaceNoisePrivacy laplacePrivacy;
    PrivacyContext strongPrivacy;
    strongPrivacy.epsilon = 0.1;
    PrivacyContext weakPrivacy;
    weakPrivacy.epsilon = 10.0;
    const PrivacyResult strongNoise =
        laplacePrivacy.apply(anonymityWorkers, strongPrivacy);
    const PrivacyResult weakNoise =
        laplacePrivacy.apply(anonymityWorkers, weakPrivacy);
    require(strongNoise.workers.size() == anonymityWorkers.size(),
            "LaplaceNoisePrivacy should keep the worker count.");
    require(strongNoise.averagePrivacyLoss > weakNoise.averagePrivacyLoss,
            "LaplaceNoisePrivacy should add more noise for smaller epsilon.");
    require(weakNoise.averagePrivacyLoss > 0.0,
            "LaplaceNoisePrivacy should perturb exposed locations.");

    std::vector<Task> tasks{
        Task{10, Location{16.0, 16.0}, 20.0, 100.0, 1, 1, TaskType::Traffic},
        Task{11, Location{76.0, 72.0}, 30.0, 100.0, 1, 1, TaskType::Noise}
    };
    NearestGreedyAlgorithm algorithm;
    const AssignmentResult assignmentResult =
        algorithm.assign(tasks, privacyResult.workers, AssignmentContext{});
    require(assignmentResult.assignments.size() == 2,
            "NearestGreedyAlgorithm should assign both feasible tasks.");
    require(assignmentResult.assignments[0].taskId == 10 &&
                assignmentResult.assignments[0].workerId == 1,
            "NearestGreedyAlgorithm should choose the closest exposed worker.");
    require(assignmentResult.algorithmRuntimeMs >= 0.0,
            "NearestGreedyAlgorithm should record runtime.");

    std::vector<Worker> scoreWorkers{
        Worker{1, Location{0.0, 0.0}, Location{0.0, 0.0}, 1, 1.0, 0.0, 0.4},
        Worker{2, Location{10.0, 0.0}, Location{10.0, 0.0}, 1, 1.0, 0.0, 1.0}
    };
    std::vector<Task> scoreTasks{
        Task{20, Location{0.0, 0.0}, 0.0, 100.0, 1, 1, TaskType::Traffic}
    };
    AssignmentContext scoreContext;
    scoreContext.alpha = 100.0;
    scoreContext.beta = 0.0;
    scoreContext.gamma = 1.0;
    scoreContext.delta = 0.0;
    ScoreGreedyAlgorithm scoreAlgorithm;
    const AssignmentResult scoreResult =
        scoreAlgorithm.assign(scoreTasks, scoreWorkers, scoreContext);
    require(scoreResult.assignments.size() == 1,
            "ScoreGreedyAlgorithm should assign the feasible task.");
    require(scoreResult.assignments[0].workerId == 2,
            "ScoreGreedyAlgorithm should choose the highest scoring worker.");

    std::vector<Task> matchingTasks{
        Task{30, Location{0.0, 0.0}, 10.0, 100.0, 1, 1, TaskType::Traffic},
        Task{31, Location{10.0, 0.0}, 10.0, 100.0, 1, 1, TaskType::Noise}
    };
    std::vector<Worker> matchingWorkers{
        Worker{7, Location{1.0, 0.0}, Location{1.0, 0.0}, 1, 1.0, 0.0, 0.9},
        Worker{8, Location{9.0, 0.0}, Location{9.0, 0.0}, 1, 1.0, 0.0, 0.9}
    };
    HungarianAlgorithm hungarianAlgorithm;
    const AssignmentResult hungarianResult =
        hungarianAlgorithm.assign(matchingTasks, matchingWorkers, AssignmentContext{});
    require(hungarianResult.assignments.size() == 2,
            "HungarianAlgorithm should assign both tasks when workers are available.");
    require(hungarianResult.assignments[0].taskId == 30 &&
                hungarianResult.assignments[0].workerId == 7,
            "HungarianAlgorithm should minimize cost for the first task.");
    require(hungarianResult.assignments[1].taskId == 31 &&
                hungarianResult.assignments[1].workerId == 8,
            "HungarianAlgorithm should minimize cost for the second task.");

    const auto privacyFromFactory = PrivacyFactory::create("laplace");
    require(privacyFromFactory != nullptr,
            "PrivacyFactory should create known privacy mechanisms.");
    require(privacyFromFactory->name() == "Laplace Noise Privacy",
            "PrivacyFactory should create the requested privacy mechanism.");
    require(PrivacyFactory::create("unknown") == nullptr,
            "PrivacyFactory should return nullptr for unknown privacy mechanisms.");

    const auto algorithmFromFactory = AssignmentAlgorithmFactory::create("hungarian");
    require(algorithmFromFactory != nullptr,
            "AssignmentAlgorithmFactory should create known algorithms.");
    require(algorithmFromFactory->name() == "Hungarian",
            "AssignmentAlgorithmFactory should create the requested algorithm.");
    require(AssignmentAlgorithmFactory::create("unknown") == nullptr,
            "AssignmentAlgorithmFactory should return nullptr for unknown algorithms.");

    const EvaluationMetrics metrics = MetricsCalculator::calculate(
        tasks, workers, assignmentResult, privacyResult.averagePrivacyLoss);
    const double expectedAverageDistance =
        (Location{12.0, 18.0}.distanceTo(Location{16.0, 16.0}) +
         Location{71.0, 73.0}.distanceTo(Location{76.0, 72.0})) /
        2.0;
    require(near(metrics.completionRate, 1.0),
            "MetricsCalculator should count all assigned tasks as completed.");
    require(near(metrics.averageMovingDistance, expectedAverageDistance),
            "MetricsCalculator should use real worker locations for movement distance.");
    require(near(metrics.totalReward, 50.0),
            "MetricsCalculator should sum rewards of completed tasks.");
    require(near(metrics.averagePrivacyLoss, privacyResult.averagePrivacyLoss),
            "MetricsCalculator should carry the average privacy loss.");
    require(metrics.algorithmRuntimeMs >= 0.0,
            "MetricsCalculator should carry algorithm runtime.");

    const std::string csv = CsvExporter::toCsv({
        ExperimentSummaryRow{"Grid Privacy", "Nearest Greedy", metrics}
    });
    require(csv.find("privacy,algorithm,completed_tasks,total_tasks") != std::string::npos,
            "CsvExporter should emit a summary header.");
    require(csv.find("Grid Privacy,Nearest Greedy,2,2") != std::string::npos,
            "CsvExporter should emit experiment metric rows.");

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
