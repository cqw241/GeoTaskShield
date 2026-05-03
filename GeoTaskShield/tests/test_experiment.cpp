#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"
#include "experiment/BatchResultCsvLoader.h"
#include "experiment/BatchResultModel.h"
#include "tests/TestSupport.h"

#include <string>
#include <vector>

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

    return 0;
}
