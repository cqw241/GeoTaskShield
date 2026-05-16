#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"
#include "experiment/BatchResultCsvLoader.h"
#include "experiment/BatchResultModel.h"
#include "experiment/ExperimentPlan.h"
#include "tests/TestSupport.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

namespace {

std::filesystem::path findProjectRoot()
{
    std::filesystem::path current = std::filesystem::current_path();
    while (!current.empty()) {
        if (std::filesystem::exists(current / "docs" / "examples" / "experiment_plan_basic.json")) {
            return current;
        }
        current = current.parent_path();
    }
    return {};
}

} // namespace

int main()
{
    using namespace gts;
    using namespace gts_test;

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

    const std::string validBatchCsv =
        "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
        "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
        "fairness_index,privacy_utility_ratio,timeout_rate\r\n"
        "\"scenario,quoted\",10,5,10,3,1,Grid Privacy,Nearest Greedy,"
        "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\r\n";
    const BatchResultLoadResult validLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv("gts_phase7_valid.csv", validBatchCsv));
    require(validLoad.success, validLoad.errorMessage);
    require(validLoad.records.size() == 1,
            "BatchResultCsvLoader should load one valid row.");
    require(validLoad.records[0].scenario == "scenario,quoted",
            "BatchResultCsvLoader should support quoted fields.");
    require(near(validLoad.records[0].averageTrueDistance, 12.5),
            "BatchResultCsvLoader should map average_moving_distance.");
    require(near(validLoad.records[0].runtimeMs, 0.4),
            "BatchResultCsvLoader should map algorithm_runtime_ms.");

    const std::string aliasCsv =
        "\xEF\xBB\xBFscenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_true_distance,"
        "total_reward,average_privacy_loss,runtimeMs,user_load_stddev,"
        "fairness_index,privacyUtilityRatio,timeout_rate\n"
        "alias,8,3,5,2,0.5,Laplace Noise Privacy,Score Greedy,"
        "3,3,1,4.5,50,1.25,0.7,0.1,0.8,0.44,0.2\n";
    const BatchResultLoadResult aliasLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv("gts_phase7_alias.csv", aliasCsv));
    require(aliasLoad.success, aliasLoad.errorMessage);
    require(near(aliasLoad.records[0].averageTrueDistance, 4.5),
            "BatchResultCsvLoader should accept average_true_distance alias.");
    require(near(aliasLoad.records[0].runtimeMs, 0.7),
            "BatchResultCsvLoader should accept runtimeMs alias.");
    require(near(aliasLoad.records[0].privacyUtilityRatio, 0.44),
            "BatchResultCsvLoader should accept privacyUtilityRatio alias.");

    const BatchResultLoadResult missingColumnLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv(
            "gts_phase7_missing.csv",
            "scenario,workers\nonly,1\n"));
    require(!missingColumnLoad.success,
            "BatchResultCsvLoader should reject missing required columns.");
    require(contains(missingColumnLoad.errorMessage, "tasks"),
            "Missing-column errors should include the missing column name.");

    const BatchResultLoadResult invalidNumberLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv(
            "gts_phase7_invalid.csv",
            "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
            "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
            "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
            "fairness_index,privacy_utility_ratio,timeout_rate\n"
            "bad,not-a-number,5,10,3,1,Grid Privacy,Nearest Greedy,"
            "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\n"));
    require(!invalidNumberLoad.success,
            "BatchResultCsvLoader should reject invalid numeric cells.");
    require(contains(invalidNumberLoad.errorMessage, "workers") &&
                contains(invalidNumberLoad.errorMessage, "not-a-number"),
            "Invalid numeric errors should include row, column, and raw value.");

    BatchResultModel resultModel;
    resultModel.setRecords({
        BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Grid Privacy", "Nearest Greedy",
                          5, 5, 1.0, 5.0, 100.0, 2.0, 0.3, 0.4, 0.7, 0.33, 0.0},
        BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Laplace Noise Privacy", "Score Greedy",
                          4, 5, 0.8, 3.0, 80.0, 1.0, 0.2, 0.2, 0.9, 0.40, 0.1},
        BatchResultRecord{"s2", 20, 8, 5.0, 5, 0.5, "Grid Privacy", "Hungarian",
                          6, 8, 0.75, 8.0, 120.0, 4.0, 0.1, 0.8, 0.6, 0.15, 0.2}
    });
    resultModel.setPrivacyFilter("Grid Privacy");
    require(resultModel.filteredRecords().size() == 2,
            "BatchResultModel should filter by privacy.");
    const std::string filteredCsv = resultModel.csvReport();
    require(contains(filteredCsv,
                     "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm"),
            "BatchResultModel should export filtered results with the batch CSV header.");
    require(contains(filteredCsv, "s1,10,5,10,3,1,Grid Privacy,Nearest Greedy"),
            "BatchResultModel should export matching filtered rows.");
    require(!contains(filteredCsv, "Laplace Noise Privacy"),
            "BatchResultModel should exclude rows outside the active filter.");
    resultModel.setAlgorithmFilter("Hungarian");
    require(resultModel.filteredRecords().size() == 1 &&
                resultModel.filteredRecords()[0].scenario == "s2",
            "BatchResultModel should filter by privacy and algorithm.");
    resultModel.clearFilters();

    const auto completion = resultModel.bestCompletionRate();
    require(completion.has_value() &&
                completion->record.scenario == "s1" &&
                near(completion->value, 1.0),
            "BatchResultModel should report best completion rate with source record.");
    const auto utility = resultModel.bestPrivacyUtilityRatio();
    require(utility.has_value() &&
                utility->record.privacy == "Laplace Noise Privacy",
            "BatchResultModel should report best privacy-utility source record.");
    const auto fairness = resultModel.bestFairnessIndex();
    require(fairness.has_value() && near(fairness->value, 0.9),
            "BatchResultModel should report best fairness.");
    const auto privacyLoss = resultModel.lowestAveragePrivacyLoss();
    require(privacyLoss.has_value() && near(privacyLoss->value, 1.0),
            "BatchResultModel should report lowest average privacy loss.");

    const std::vector<BatchResultRecord> sortedBatchResults =
        resultModel.sortedRecords(BatchResultSortField::AveragePrivacyLoss, true);
    require(sortedBatchResults.front().privacy == "Laplace Noise Privacy",
            "BatchResultModel should sort numeric metrics numerically.");
    const std::vector<ChartBar> bars =
        resultModel.chartBars(BatchResultMetric::PrivacyUtilityRatio);
    require(bars.size() == 3 &&
                contains(bars[0].label, "s1") &&
                contains(bars[0].label, "Grid Privacy") &&
                contains(bars[0].label, "Nearest Greedy"),
            "BatchResultModel should build chart labels from scenario, privacy, and algorithm.");

    const std::string validPlanJson = R"({
  "name": "core plan",
  "run_label": "core-run",
  "workers": [8, 12],
  "tasks": [3],
  "seeds": [11, 12],
  "privacy": ["grid", "laplace"],
  "algorithms": ["nearest", "score"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [0.5, 1.0],
  "areaWidth": 80.0,
  "areaHeight": 90.0
})";
    const ExperimentPlanLoadResult validPlanLoad =
        ExperimentPlanLoader::loadFromString(validPlanJson);
    require(validPlanLoad.success, validPlanLoad.errorMessage);
    require(validPlanLoad.plan.name == "core plan",
            "ExperimentPlanLoader should parse plan name.");
    require(validPlanLoad.plan.runLabel == "core-run",
            "ExperimentPlanLoader should parse run label.");
    const std::vector<ExperimentScenario> expanded =
        validPlanLoad.plan.expandToScenarios();
    require(expanded.size() == 32,
            "ExperimentPlan should expand workers, tasks, seeds, privacy, algorithms, grid_size, k, and epsilon as a Cartesian product.");
    require(expanded.front().config.workerCount == 8 &&
                expanded.front().config.taskCount == 3 &&
                expanded.front().config.randomSeed == 11 &&
                expanded.front().privacyType == "grid" &&
                expanded.front().algorithmType == "nearest" &&
                near(expanded.front().config.privacy.gridSize, 5.0) &&
                expanded.front().config.privacy.k == 3 &&
                near(expanded.front().config.privacy.epsilon, 0.5) &&
                near(expanded.front().config.areaWidth, 80.0) &&
                near(expanded.front().config.areaHeight, 90.0),
            "ExperimentPlan should apply parsed values to expanded scenarios.");
    require(expanded.back().config.workerCount == 12 &&
                expanded.back().config.randomSeed == 12 &&
                expanded.back().privacyType == "laplace" &&
                expanded.back().algorithmType == "score" &&
                near(expanded.back().config.privacy.epsilon, 1.0),
            "ExperimentPlan should include the last Cartesian product combination.");
    const std::vector<ExperimentScenario> expandedAgain =
        validPlanLoad.plan.expandToScenarios();
    require(expandedAgain.size() == expanded.size() &&
                expandedAgain.front().name == expanded.front().name &&
                expandedAgain.back().name == expanded.back().name,
            "ExperimentPlan expansion should be stable across repeated calls.");

    const std::filesystem::path projectRoot = findProjectRoot();
    require(!projectRoot.empty(),
            "Experiment tests should locate the project root from the build directory.");
    const ExperimentPlanLoadResult stressPlanLoad =
        ExperimentPlanLoader::loadFromFile(
            (projectRoot / "docs" / "examples" / "experiment_plan_stress.json").string());
    require(stressPlanLoad.success, stressPlanLoad.errorMessage);
    require(stressPlanLoad.plan.dataProfiles.size() == 5,
            "Stress plan should parse all configured data profiles.");
    const std::vector<ExperimentScenario> stressScenarios =
        stressPlanLoad.plan.expandToScenarios();
    require(stressScenarios.size() == 180,
            "Stress plan should expand profiles, privacy mechanisms, algorithms, grid sizes, and epsilons.");
    require(stressScenarios.front().config.dataProfile == "worker-shortage",
            "Stress scenario expansion should apply the first data profile.");
    require(contains(stressScenarios.front().name, "profile-worker-shortage"),
            "Stress scenario names should include the data profile.");

    const BatchExperimentResult stressResult = batchRunner.run(stressScenarios);
    int nonFullCompletionRows = 0;
    int fullCompletionRows = 0;
    int timeoutRows = 0;
    double minFairness = std::numeric_limits<double>::max();
    double maxFairness = std::numeric_limits<double>::lowest();
    double minRewardSkewReward = std::numeric_limits<double>::max();
    double maxRewardSkewReward = std::numeric_limits<double>::lowest();
    double workerShortageCompletionSum = 0.0;
    int workerShortageRows = 0;
    double rewardSkewCompletionSum = 0.0;
    int rewardSkewRows = 0;
    double rewardSkewNearestRewardSum = 0.0;
    int rewardSkewNearestRows = 0;
    double rewardSkewScoreRewardSum = 0.0;
    int rewardSkewScoreRows = 0;
    double rewardSkewHungarianRewardSum = 0.0;
    int rewardSkewHungarianRows = 0;
    double minHighPrivacyLoss = std::numeric_limits<double>::max();
    double maxHighPrivacyLoss = std::numeric_limits<double>::lowest();
    double minHighPrivacyDistance = std::numeric_limits<double>::max();
    double maxHighPrivacyDistance = std::numeric_limits<double>::lowest();
    double minHighPrivacyUtility = std::numeric_limits<double>::max();
    double maxHighPrivacyUtility = std::numeric_limits<double>::lowest();
    int highPrivacyRows = 0;
    int deadlineTimeoutFullCompletionRows = 0;
    double maxDeadlineOrSpeedTimeout = 0.0;
    for (const BatchExperimentRow& row : stressResult.rows) {
        if (row.metrics.completionRate < 1.0) {
            ++nonFullCompletionRows;
        } else if (near(row.metrics.completionRate, 1.0)) {
            ++fullCompletionRows;
        }
        if (row.metrics.timeoutRate > 0.0) {
            ++timeoutRows;
        }
        minFairness = std::min(minFairness, row.metrics.fairnessIndex);
        maxFairness = std::max(maxFairness, row.metrics.fairnessIndex);
        if (row.config.dataProfile == "worker-shortage") {
            workerShortageCompletionSum += row.metrics.completionRate;
            ++workerShortageRows;
        }
        if (row.config.dataProfile == "reward-skew") {
            rewardSkewCompletionSum += row.metrics.completionRate;
            ++rewardSkewRows;
            minRewardSkewReward = std::min(minRewardSkewReward, row.metrics.totalReward);
            maxRewardSkewReward = std::max(maxRewardSkewReward, row.metrics.totalReward);
            if (row.algorithmName == "Nearest Greedy") {
                rewardSkewNearestRewardSum += row.metrics.totalReward;
                ++rewardSkewNearestRows;
            } else if (row.algorithmName == "Score Greedy") {
                rewardSkewScoreRewardSum += row.metrics.totalReward;
                ++rewardSkewScoreRows;
            } else if (row.algorithmName == "Hungarian") {
                rewardSkewHungarianRewardSum += row.metrics.totalReward;
                ++rewardSkewHungarianRows;
            }
        }
        if (row.config.dataProfile == "high-privacy-noise") {
            minHighPrivacyLoss = std::min(minHighPrivacyLoss, row.metrics.averagePrivacyLoss);
            maxHighPrivacyLoss = std::max(maxHighPrivacyLoss, row.metrics.averagePrivacyLoss);
            minHighPrivacyDistance =
                std::min(minHighPrivacyDistance, row.metrics.averageMovingDistance);
            maxHighPrivacyDistance =
                std::max(maxHighPrivacyDistance, row.metrics.averageMovingDistance);
            minHighPrivacyUtility =
                std::min(minHighPrivacyUtility, row.metrics.privacyUtilityRatio);
            maxHighPrivacyUtility =
                std::max(maxHighPrivacyUtility, row.metrics.privacyUtilityRatio);
            ++highPrivacyRows;
        }
        if (row.config.dataProfile == "deadline-tight" ||
            row.config.dataProfile == "heterogeneous-speed") {
            maxDeadlineOrSpeedTimeout =
                std::max(maxDeadlineOrSpeedTimeout, row.metrics.timeoutRate);
        }
        if (row.config.dataProfile == "deadline-tight" &&
            near(row.metrics.completionRate, 1.0) &&
            row.metrics.timeoutRate > 0.0) {
            ++deadlineTimeoutFullCompletionRows;
        }
    }
    require(nonFullCompletionRows >= 3,
            "Stress scenario suite should produce at least three non-full completion rows.");
    require(fullCompletionRows > 0,
            "Stress scenario suite should include full-completion rows so capacity pressure does not dominate every profile.");
    require(timeoutRows >= 1,
            "Stress scenario suite should produce at least one timeout row.");
    require(maxDeadlineOrSpeedTimeout > 0.0,
            "Deadline-tight or heterogeneous-speed stress profiles should produce timeout pressure.");
    require(deadlineTimeoutFullCompletionRows > 0,
            "Deadline-tight stress profile should show timeout pressure without completion pressure.");
    require(maxFairness - minFairness > 0.05,
            "Stress scenario suite should produce visible fairness variation.");
    require(workerShortageRows > 0 && rewardSkewRows > 0,
            "Stress scenario suite should include worker-shortage and reward-skew rows.");
    require((rewardSkewCompletionSum / static_cast<double>(rewardSkewRows)) >= 0.80,
            "Reward-skew stress profile should not be dominated by worker capacity shortage.");
    require((rewardSkewCompletionSum / static_cast<double>(rewardSkewRows)) >
                (workerShortageCompletionSum / static_cast<double>(workerShortageRows)),
            "Reward-skew stress profile should be less capacity constrained than worker-shortage.");
    require(maxRewardSkewReward - minRewardSkewReward > 100.0,
            "Reward-skew stress profile should produce visible totalReward differences across algorithms.");
    require(rewardSkewNearestRows > 0 && rewardSkewScoreRows > 0 &&
                rewardSkewHungarianRows > 0,
            "Reward-skew stress profile should include all configured algorithms.");
    require((rewardSkewScoreRewardSum / static_cast<double>(rewardSkewScoreRows)) >
                (rewardSkewNearestRewardSum / static_cast<double>(rewardSkewNearestRows)) + 100.0,
            "Reward-skew stress profile should show score beating nearest on totalReward.");
    require(highPrivacyRows > 0,
            "Stress scenario suite should include high-privacy-noise rows.");
    require(maxHighPrivacyLoss - minHighPrivacyLoss > 1.0 &&
                maxHighPrivacyDistance - minHighPrivacyDistance > 1.0 &&
                maxHighPrivacyUtility - minHighPrivacyUtility > 0.1,
            "High-privacy-noise stress profile should vary privacy loss, distance, and utility.");

    const std::string stressMarkdown = BatchExperimentExporter::toMarkdown(stressResult);
    require(contains(stressMarkdown, "## Stress Scenario Summary"),
            "Stress report should include a stress scenario summary.");
    require(contains(stressMarkdown, "Non-full completion rows"),
            "Stress report should summarize non-full completion rows.");
    require(contains(stressMarkdown, "Timeout pressure rows"),
            "Stress report should summarize timeout pressure rows.");
    require(contains(stressMarkdown, "Privacy-utility tradeoff"),
            "Stress report should explain privacy utility tradeoffs.");
    require(contains(stressMarkdown, "## Stress Profiles"),
            "Stress report should include per-profile summaries.");
    require(contains(stressMarkdown, "reward-skew"),
            "Stress report should identify reward-skew profile results.");

    SimulationConfig legacyDefaultConfig;
    require(legacyDefaultConfig.dataProfile == "default",
            "SimulationConfig should preserve the legacy default data profile.");

    const ExperimentPlanLoadResult missingFieldLoad =
        ExperimentPlanLoader::loadFromString(R"({
  "name": "missing",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})");
    require(!missingFieldLoad.success,
            "ExperimentPlanLoader should reject missing required fields.");
    require(contains(missingFieldLoad.errorMessage, "algorithms"),
            "Missing-field errors should name the missing field.");

    const ExperimentPlanLoadResult emptyListLoad =
        ExperimentPlanLoader::loadFromString(R"({
  "name": "empty-list",
  "workers": [],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})");
    require(!emptyListLoad.success,
            "ExperimentPlanLoader should reject empty parameter lists.");
    require(contains(emptyListLoad.errorMessage, "workers"),
            "Empty-list errors should name the empty field.");

    const ExperimentPlanLoadResult unknownPrivacyLoad =
        ExperimentPlanLoader::loadFromString(R"({
  "name": "bad-privacy",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["masked"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})");
    require(!unknownPrivacyLoad.success,
            "ExperimentPlanLoader should reject unknown privacy mechanisms.");
    require(contains(unknownPrivacyLoad.errorMessage, "masked"),
            "Unknown privacy errors should include the invalid value.");

    const ExperimentPlanLoadResult unknownAlgorithmLoad =
        ExperimentPlanLoader::loadFromString(R"({
  "name": "bad-algorithm",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["auction"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})");
    require(!unknownAlgorithmLoad.success,
            "ExperimentPlanLoader should reject unknown assignment algorithms.");
    require(contains(unknownAlgorithmLoad.errorMessage, "auction"),
            "Unknown algorithm errors should include the invalid value.");

    const auto requireInvalidPlanValue =
        [](const std::string& json,
           const std::string& fieldName,
           const std::string& invalidValue) {
            const ExperimentPlanLoadResult load =
                ExperimentPlanLoader::loadFromString(json);
            require(!load.success,
                    "ExperimentPlanLoader should reject invalid " + fieldName + " values.");
            require(contains(load.errorMessage, fieldName) &&
                        contains(load.errorMessage, invalidValue),
                    "Invalid " + fieldName + " errors should include the field name and illegal value.");
        };
    requireInvalidPlanValue(R"({
  "name": "invalid-workers",
  "workers": [0],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})", "workers", "0");
    requireInvalidPlanValue(R"({
  "name": "invalid-tasks",
  "workers": [8],
  "tasks": [-1],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0]
})", "tasks", "-1");
    requireInvalidPlanValue(R"({
  "name": "invalid-grid",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [0],
  "k": [3],
  "epsilon": [1.0]
})", "grid_size", "0");
    requireInvalidPlanValue(R"({
  "name": "invalid-epsilon",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [0]
})", "epsilon", "0");
    requireInvalidPlanValue(R"({
  "name": "invalid-area",
  "workers": [8],
  "tasks": [3],
  "seeds": [11],
  "privacy": ["grid"],
  "algorithms": ["nearest"],
  "grid_size": [5.0],
  "k": [3],
  "epsilon": [1.0],
  "areaWidth": 0
})", "areaWidth", "0");

    const std::filesystem::path runDir =
        std::filesystem::temp_directory_path() / "gts_experiment_plan_run";
    std::filesystem::remove_all(runDir);
    ExperimentPlan archivePlan;
    archivePlan.name = "Archive Plan";
    archivePlan.runLabel = "Archive Run";
    archivePlan.workers = {8};
    archivePlan.tasks = {3};
    archivePlan.seeds = {11};
    archivePlan.privacyMechanisms = {"grid"};
    archivePlan.assignmentAlgorithms = {"nearest"};
    archivePlan.gridSizes = {5.0};
    archivePlan.kValues = {3};
    archivePlan.epsilons = {1.0};
    archivePlan.sourceJson = validPlanJson;
    const ExperimentPlanRunResult runResult =
        ExperimentPlanRunner::runToDirectory(archivePlan, runDir.string(), "test-version");
    require(runResult.success, runResult.errorMessage);
    require(runResult.scenarioCount == 1,
            "ExperimentPlanRunner should report the expanded scenario count.");
    require(std::filesystem::exists(runDir / "results.csv"),
            "ExperimentPlanRunner should export results.csv.");
    require(std::filesystem::exists(runDir / "report.md"),
            "ExperimentPlanRunner should export report.md.");
    require(std::filesystem::exists(runDir / "plan_snapshot.json"),
            "ExperimentPlanRunner should export plan_snapshot.json.");
    require(std::filesystem::exists(runDir / "metadata.json"),
            "ExperimentPlanRunner should export metadata.json.");
    {
        std::ifstream metadataFile(runDir / "metadata.json");
        const std::string metadata((std::istreambuf_iterator<char>(metadataFile)),
                                   std::istreambuf_iterator<char>());
        require(contains(metadata, "\"scenario_count\": 1"),
                "ExperimentPlanRunner metadata should include scenario count.");
        require(contains(metadata, "\"project_version\": \"test-version\""),
                "ExperimentPlanRunner metadata should include project version.");
    }
    std::filesystem::remove_all(runDir);

    return 0;
}
