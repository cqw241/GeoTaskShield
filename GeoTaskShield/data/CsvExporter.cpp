#include "data/CsvExporter.h"

#include <fstream>
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

} // namespace

std::string CsvExporter::toCsv(const std::vector<ExperimentSummaryRow>& rows)
{
    std::ostringstream output;
    output << "privacy,algorithm,completed_tasks,total_tasks,completion_rate,"
              "average_moving_distance,total_reward,average_privacy_loss,algorithm_runtime_ms\n";

    for (const ExperimentSummaryRow& row : rows) {
        output << escapeCsv(row.privacyName) << ','
               << escapeCsv(row.algorithmName) << ','
               << row.metrics.completedTasks << ','
               << row.metrics.totalTasks << ','
               << row.metrics.completionRate << ','
               << row.metrics.averageMovingDistance << ','
               << row.metrics.totalReward << ','
               << row.metrics.averagePrivacyLoss << ','
               << row.metrics.algorithmRuntimeMs << '\n';
    }

    return output.str();
}

bool CsvExporter::writeToFile(const std::string& filePath,
                              const std::vector<ExperimentSummaryRow>& rows)
{
    std::ofstream file(filePath);
    if (!file) {
        return false;
    }

    file << toCsv(rows);
    return static_cast<bool>(file);
}

} // namespace gts
