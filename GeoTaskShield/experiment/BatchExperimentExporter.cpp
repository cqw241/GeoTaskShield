#include "experiment/BatchExperimentExporter.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
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

const BatchExperimentRow* maxByCompletionGap(const BatchExperimentResult& result)
{
    const BatchExperimentRow* best = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (row.metrics.completionRate >= 1.0) {
            continue;
        }
        if (!best || row.metrics.completionRate < best->metrics.completionRate) {
            best = &row;
        }
    }
    return best;
}

const BatchExperimentRow* maxByTimeout(const BatchExperimentResult& result)
{
    const BatchExperimentRow* best = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (row.metrics.timeoutRate <= 0.0) {
            continue;
        }
        if (!best || row.metrics.timeoutRate > best->metrics.timeoutRate) {
            best = &row;
        }
    }
    return best;
}

const BatchExperimentRow* maxByReward(const BatchExperimentResult& result)
{
    const BatchExperimentRow* best = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (!best || row.metrics.totalReward > best->metrics.totalReward) {
            best = &row;
        }
    }
    return best;
}

const BatchExperimentRow* maxByFairness(const BatchExperimentResult& result)
{
    const BatchExperimentRow* best = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (!best || row.metrics.fairnessIndex > best->metrics.fairnessIndex) {
            best = &row;
        }
    }
    return best;
}

const BatchExperimentRow* maxByPrivacyLoss(const BatchExperimentResult& result)
{
    const BatchExperimentRow* best = nullptr;
    for (const BatchExperimentRow& row : result.rows) {
        if (!best || row.metrics.averagePrivacyLoss > best->metrics.averagePrivacyLoss) {
            best = &row;
        }
    }
    return best;
}

std::size_t countNonFullCompletionRows(const BatchExperimentResult& result)
{
    std::size_t count = 0;
    for (const BatchExperimentRow& row : result.rows) {
        if (row.metrics.completionRate < 1.0) {
            ++count;
        }
    }
    return count;
}

std::size_t countTimeoutRows(const BatchExperimentResult& result)
{
    std::size_t count = 0;
    for (const BatchExperimentRow& row : result.rows) {
        if (row.metrics.timeoutRate > 0.0) {
            ++count;
        }
    }
    return count;
}

void writeRowReference(std::ostringstream& output, const BatchExperimentRow& row)
{
    output << row.scenarioName << " (" << row.privacyName << " + "
           << row.algorithmName << ")";
}

struct ProfileSummary {
    std::size_t rows{};
    std::size_t nonFullCompletionRows{};
    std::size_t timeoutRows{};
    double minCompletion{std::numeric_limits<double>::max()};
    double maxCompletion{std::numeric_limits<double>::lowest()};
    double maxTimeout{};
    double minDistance{std::numeric_limits<double>::max()};
    double maxDistance{std::numeric_limits<double>::lowest()};
    double minPrivacyLoss{std::numeric_limits<double>::max()};
    double maxPrivacyLoss{std::numeric_limits<double>::lowest()};
    double minPrivacyUtility{std::numeric_limits<double>::max()};
    double maxPrivacyUtility{std::numeric_limits<double>::lowest()};
    double minReward{std::numeric_limits<double>::max()};
    double maxReward{std::numeric_limits<double>::lowest()};
    double minFairness{std::numeric_limits<double>::max()};
    double maxFairness{std::numeric_limits<double>::lowest()};
};

void addToProfileSummary(ProfileSummary& summary, const BatchExperimentRow& row)
{
    ++summary.rows;
    if (row.metrics.completionRate < 1.0) {
        ++summary.nonFullCompletionRows;
    }
    if (row.metrics.timeoutRate > 0.0) {
        ++summary.timeoutRows;
    }

    summary.minCompletion = std::min(summary.minCompletion, row.metrics.completionRate);
    summary.maxCompletion = std::max(summary.maxCompletion, row.metrics.completionRate);
    summary.maxTimeout = std::max(summary.maxTimeout, row.metrics.timeoutRate);
    summary.minDistance = std::min(summary.minDistance, row.metrics.averageMovingDistance);
    summary.maxDistance = std::max(summary.maxDistance, row.metrics.averageMovingDistance);
    summary.minPrivacyLoss = std::min(summary.minPrivacyLoss, row.metrics.averagePrivacyLoss);
    summary.maxPrivacyLoss = std::max(summary.maxPrivacyLoss, row.metrics.averagePrivacyLoss);
    summary.minPrivacyUtility = std::min(summary.minPrivacyUtility, row.metrics.privacyUtilityRatio);
    summary.maxPrivacyUtility = std::max(summary.maxPrivacyUtility, row.metrics.privacyUtilityRatio);
    summary.minReward = std::min(summary.minReward, row.metrics.totalReward);
    summary.maxReward = std::max(summary.maxReward, row.metrics.totalReward);
    summary.minFairness = std::min(summary.minFairness, row.metrics.fairnessIndex);
    summary.maxFairness = std::max(summary.maxFairness, row.metrics.fairnessIndex);
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

    output << "\n## Stress Scenario Summary\n\n";
    const std::size_t nonFullRows = countNonFullCompletionRows(result);
    const std::size_t timeoutRows = countTimeoutRows(result);
    output << "- Non-full completion rows: " << nonFullRows << " of "
           << result.rows.size() << ".";
    if (const BatchExperimentRow* row = maxByCompletionGap(result)) {
        output << " Lowest completion: ";
        writeRowReference(output, *row);
        output << " at " << row->metrics.completionRate * 100.0 << "%.";
    }
    output << '\n';

    output << "- Timeout pressure rows: " << timeoutRows << " of "
           << result.rows.size() << ".";
    if (const BatchExperimentRow* row = maxByTimeout(result)) {
        output << " Highest timeout: ";
        writeRowReference(output, *row);
        output << " at " << row->metrics.timeoutRate * 100.0 << "%.";
    }
    output << '\n';

    if (const BatchExperimentRow* row = maxByPrivacyLoss(result)) {
        output << "- Privacy-utility tradeoff: strongest privacy perturbation observed in ";
        writeRowReference(output, *row);
        output << " with average_moving_distance " << row->metrics.averageMovingDistance
               << ", average_privacy_loss " << row->metrics.averagePrivacyLoss
               << ", and privacy_utility_ratio "
               << row->metrics.privacyUtilityRatio << ".\n";
    }

    if (const BatchExperimentRow* row = maxByReward(result)) {
        output << "- Best totalReward: ";
        writeRowReference(output, *row);
        output << " at " << row->metrics.totalReward << ".\n";
    }
    if (const BatchExperimentRow* row = maxByFairness(result)) {
        output << "- Best fairnessIndex: ";
        writeRowReference(output, *row);
        output << " at " << row->metrics.fairnessIndex << ".\n";
    }

    std::map<std::string, ProfileSummary> summariesByProfile;
    for (const BatchExperimentRow& row : result.rows) {
        const std::string profile =
            row.config.dataProfile.empty() ? "default" : row.config.dataProfile;
        addToProfileSummary(summariesByProfile[profile], row);
    }

    if (!summariesByProfile.empty()) {
        output << "\n## Stress Profiles\n\n";
        output << "| Data profile | Rows | Non-full | Timeout rows | Completion range | "
                  "Max timeout | Distance range | Privacy loss range | Utility range | "
                  "Reward range | Fairness range |\n";
        output << "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n";
        for (const auto& [profile, summary] : summariesByProfile) {
            output << "| " << profile
                   << " | " << summary.rows
                   << " | " << summary.nonFullCompletionRows
                   << " | " << summary.timeoutRows
                   << " | " << summary.minCompletion * 100.0 << "%-"
                   << summary.maxCompletion * 100.0 << "%"
                   << " | " << summary.maxTimeout * 100.0 << "%"
                   << " | " << summary.minDistance << "-"
                   << summary.maxDistance
                   << " | " << summary.minPrivacyLoss << "-"
                   << summary.maxPrivacyLoss
                   << " | " << summary.minPrivacyUtility << "-"
                   << summary.maxPrivacyUtility
                   << " | " << summary.minReward << "-"
                   << summary.maxReward
                   << " | " << summary.minFairness << "-"
                   << summary.maxFairness
                   << " |\n";
        }
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
