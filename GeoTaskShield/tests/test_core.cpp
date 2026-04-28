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
#include "agent/ExperimentAgent.h"
#include "agent/ReportGenerator.h"
#include "agent/RuleBasedConfigParser.h"
#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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

bool contains(const std::string& value, const std::string& expected)
{
    return value.find(expected) != std::string::npos;
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

    std::vector<Worker> loadWorkers{
        Worker{101, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0},
        Worker{102, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0},
        Worker{103, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0}
    };
    std::vector<Task> loadTasks{
        Task{1010, Location{3.0, 4.0}, 10.0, 4.0, 1, 1, TaskType::Traffic},
        Task{1011, Location{0.0, 0.0}, 20.0, 1.0, 1, 1, TaskType::Noise},
        Task{1012, Location{0.0, 0.0}, 30.0, 1.0, 1, 1, TaskType::AirQuality}
    };
    AssignmentResult loadAssignment;
    loadAssignment.assignments = {
        Assignment{1010, 101, 5.0, 0.0},
        Assignment{1011, 101, 0.0, 0.0},
        Assignment{1012, 102, 0.0, 0.0}
    };
    const EvaluationMetrics loadMetrics =
        MetricsCalculator::calculate(loadTasks, loadWorkers, loadAssignment, 2.0);
    require(near(loadMetrics.userLoadStdDev, std::sqrt(2.0 / 3.0)),
            "MetricsCalculator should report worker load standard deviation.");
    require(near(loadMetrics.fairnessIndex, 0.6),
            "MetricsCalculator should report Jain fairness over worker loads.");
    require(near(loadMetrics.privacyUtilityRatio, 1.0 / 3.0),
            "MetricsCalculator should report completion per privacy loss.");
    require(near(loadMetrics.timeoutRate, 1.0 / 3.0),
            "MetricsCalculator should report assigned task timeout rate.");

    const std::string csv = CsvExporter::toCsv({
        ExperimentSummaryRow{"Grid Privacy", "Nearest Greedy", metrics}
    });
    require(csv.find("privacy,algorithm,completed_tasks,total_tasks") != std::string::npos,
            "CsvExporter should emit a summary header.");
    require(csv.find("Grid Privacy,Nearest Greedy,2,2") != std::string::npos,
            "CsvExporter should emit experiment metric rows.");

    RuleBasedConfigParser parser;
    const ExperimentRequest chineseRequest =
        parser.parse("100 个用户，50 个任务，k=5，使用匈牙利算法");
    require(chineseRequest.config.workerCount == 100,
            "RuleBasedConfigParser should parse Chinese worker counts.");
    require(chineseRequest.config.taskCount == 50,
            "RuleBasedConfigParser should parse Chinese task counts.");
    require(chineseRequest.config.privacy.k == 5,
            "RuleBasedConfigParser should parse k-anonymity values.");
    require(chineseRequest.privacyType == "k-anonymity",
            "RuleBasedConfigParser should infer k-anonymity from k prompts.");
    require(chineseRequest.algorithmType == "hungarian",
            "RuleBasedConfigParser should parse Hungarian algorithm requests.");

    const ExperimentRequest englishRequest =
        parser.parse("80 workers, 40 tasks, epsilon=0.5, use laplace and score greedy");
    require(englishRequest.config.workerCount == 80,
            "RuleBasedConfigParser should parse English worker counts.");
    require(englishRequest.config.taskCount == 40,
            "RuleBasedConfigParser should parse English task counts.");
    require(near(englishRequest.config.privacy.epsilon, 0.5),
            "RuleBasedConfigParser should parse epsilon values.");
    require(englishRequest.privacyType == "laplace",
            "RuleBasedConfigParser should parse Laplace privacy requests.");
    require(englishRequest.algorithmType == "score",
            "RuleBasedConfigParser should parse score greedy requests.");

    const ExperimentRequest comparisonRequest =
        parser.parse("对比三种隐私机制，50个用户，20个任务，使用最近贪心");
    require(comparisonRequest.comparePrivacyMechanisms,
            "RuleBasedConfigParser should detect privacy comparison requests.");
    require(comparisonRequest.privacyTypes.size() == 3,
            "Privacy comparison requests should include three mechanisms.");
    require(comparisonRequest.algorithmType == "nearest",
            "RuleBasedConfigParser should parse nearest greedy requests.");

    ExperimentReport report;
    report.requestText = "100 workers, 50 tasks, use grid and nearest";
    report.rows.push_back(ExperimentReportRow{
        "Grid Privacy",
        "Nearest Greedy",
        EvaluationMetrics{4, 5, 0.8, 12.5, 120.0, 3.2, 0.7}
    });
    const std::string markdown = ReportGenerator::toMarkdown(report);
    require(contains(markdown, "# GeoTaskShield Experiment Report"),
            "ReportGenerator should emit a Markdown title.");
    require(contains(markdown, "| Grid Privacy | Nearest Greedy | 4 | 5 | 80.00% |"),
            "ReportGenerator should emit a Markdown metrics table row.");
    require(contains(markdown, "Best completion rate"),
            "ReportGenerator should emit a short summary.");

    ExperimentAgent agent;
    const ExperimentAgentResult agentResult =
        agent.run("对比三种隐私机制，30个用户，10个任务，使用匈牙利算法");
    require(agentResult.report.rows.size() == 3,
            "ExperimentAgent should run three rows for privacy comparison prompts.");
    require(contains(agentResult.markdown, "Hungarian"),
            "ExperimentAgent should include selected algorithm names in the report.");

    BatchExperimentRunner batchRunner;
    std::vector<ExperimentScenario> scenarios{
        ExperimentScenario{"grid-small", SimulationConfig{}, "grid", "nearest"},
        ExperimentScenario{"laplace-small", SimulationConfig{}, "laplace", "score"}
    };
    scenarios[0].config.workerCount = 8;
    scenarios[0].config.taskCount = 3;
    scenarios[0].config.randomSeed = 11;
    scenarios[1].config.workerCount = 8;
    scenarios[1].config.taskCount = 3;
    scenarios[1].config.randomSeed = 11;
    scenarios[1].config.privacy.epsilon = 0.5;
    const BatchExperimentResult batchResult = batchRunner.run(scenarios);
    require(batchResult.rows.size() == 2,
            "BatchExperimentRunner should run one result row per scenario.");
    require(batchResult.rows[0].scenarioName == "grid-small",
            "BatchExperimentRunner should preserve scenario names.");
    require(batchResult.rows[0].metrics.totalTasks == 3,
            "BatchExperimentRunner should run scenarios through SimulationEngine.");

    const std::string batchCsv = BatchExperimentExporter::toCsv(batchResult);
    require(contains(batchCsv, "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm"),
            "BatchExperimentExporter should emit scenario configuration columns.");
    require(contains(batchCsv, "grid-small,8,3"),
            "BatchExperimentExporter should emit batch scenario rows.");
    require(contains(batchCsv, "fairness_index"),
            "BatchExperimentExporter should include expanded metrics.");

    const std::string batchMarkdown = BatchExperimentExporter::toMarkdown(batchResult);
    require(contains(batchMarkdown, "# GeoTaskShield Batch Experiment Report"),
            "BatchExperimentExporter should emit a Markdown title.");
    require(contains(batchMarkdown, "grid-small"),
            "BatchExperimentExporter should emit scenario names in Markdown.");

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
