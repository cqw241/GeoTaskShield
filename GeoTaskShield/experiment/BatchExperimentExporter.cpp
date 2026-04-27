#include "experiment/BatchExperimentExporter.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace gts {

namespace {

std::string escapeCsv(const std::string& value)
{
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }
    escaped += '"';
    return escaped;
}

void writeMetricCsvColumns(std::ostringstream& output, const EvaluationMetrics& metrics)
{
    output << metrics.completedTasks << ','
           << metrics.totalTasks << ','
           << metrics.completionRate << ','
           << metrics.averageMovingDistance << ','
           << metrics.totalReward << ','
           << metrics.averagePrivacyLoss << ','
           << metrics.algorithmRuntimeMs << ','
           << metrics.userLoadStdDev << ','
           << metrics.fairnessIndex << ','
           << metrics.privacyUtilityRatio << ','
           << metrics.timeoutRate;
}

} // namespace

std::string BatchExperimentExporter::toCsv(const BatchExperimentResult& result)
{
    std::ostringstream output;
    output << "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
              "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
              "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
              "fairness_index,privacy_utility_ratio,timeout_rate\n";

    for (const BatchExperimentRow& row : result.rows) {
        output << escapeCsv(row.scenarioName) << ','
               << row.config.workerCount << ','
               << row.config.taskCount << ','
               << row.config.privacy.gridSize << ','
               << row.config.privacy.k << ','
               << row.config.privacy.epsilon << ','
               << escapeCsv(row.privacyName) << ','
               << escapeCsv(row.algorithmName) << ',';
        writeMetricCsvColumns(output, row.metrics);
        output << '\n';
    }

    return output.str();
}

std::string BatchExperimentExporter::toMarkdown(const BatchExperimentResult& result)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(2);
    output << "# GeoTaskShield Batch Experiment Report\n\n";
    output << "| Scenario | Workers | Tasks | Privacy | Algorithm | Completion | "
              "Avg Distance | Reward | Privacy Loss | Fairness | Timeout |\n";
    output << "|---|---:|---:|---|---|---:|---:|---:|---:|---:|---:|\n";

    const BatchExperimentRow* bestRow = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (!bestRow ||
            row.metrics.privacyUtilityRatio > bestRow->metrics.privacyUtilityRatio) {
            bestRow = &row;
        }

        output << "| " << row.scenarioName
               << " | " << row.config.workerCount
               << " | " << row.config.taskCount
               << " | " << row.privacyName
               << " | " << row.algorithmName
               << " | " << row.metrics.completionRate * 100.0 << "%"
               << " | " << row.metrics.averageMovingDistance
               << " | " << row.metrics.totalReward
               << " | " << row.metrics.averagePrivacyLoss
               << " | " << row.metrics.fairnessIndex
               << " | " << row.metrics.timeoutRate * 100.0 << "%"
               << " |\n";
    }

    output << "\n## Summary\n\n";
    if (bestRow) {
        output << "Best privacy-utility ratio: " << bestRow->scenarioName
               << " (" << bestRow->privacyName << " + "
               << bestRow->algorithmName << ") at "
               << bestRow->metrics.privacyUtilityRatio << ".\n";
    } else {
        output << "No batch experiment rows were generated.\n";
    }

    return output.str();
}

bool BatchExperimentExporter::writeCsvToFile(const std::string& filePath,
                                             const BatchExperimentResult& result)
{
    std::ofstream file(filePath);
    if (!file) {
        return false;
    }
    file << toCsv(result);
    return static_cast<bool>(file);
}

bool BatchExperimentExporter::writeMarkdownToFile(const std::string& filePath,
                                                  const BatchExperimentResult& result)
{
    std::ofstream file(filePath);
    if (!file) {
        return false;
    }
    file << toMarkdown(result);
    return static_cast<bool>(file);
}

} // namespace gts
