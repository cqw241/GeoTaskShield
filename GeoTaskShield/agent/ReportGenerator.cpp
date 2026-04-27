#include "agent/ReportGenerator.h"

#include <iomanip>
#include <sstream>

namespace gts {

std::string ReportGenerator::toMarkdown(const ExperimentReport& report)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(2);
    output << "# GeoTaskShield Experiment Report\n\n";
    output << "## Request\n\n";
    output << report.requestText << "\n\n";
    output << "## Configuration\n\n";
    output << "- Workers: " << report.config.workerCount << "\n";
    output << "- Tasks: " << report.config.taskCount << "\n";
    output << "- Grid size: " << report.config.privacy.gridSize << "\n";
    output << "- k: " << report.config.privacy.k << "\n";
    output << "- Epsilon: " << report.config.privacy.epsilon << "\n\n";

    output << "## Results\n\n";
    output << "| Privacy | Algorithm | Completed | Total | Completion | Avg Distance | "
              "Reward | Privacy Loss | Runtime ms |\n";
    output << "|---|---:|---:|---:|---:|---:|---:|---:|---:|\n";

    const ExperimentReportRow* bestRow = nullptr;
    for (const ExperimentReportRow& row : report.rows) {
        if (!bestRow || row.metrics.completionRate > bestRow->metrics.completionRate) {
            bestRow = &row;
        }
        output << "| " << row.privacyName
               << " | " << row.algorithmName
               << " | " << row.metrics.completedTasks
               << " | " << row.metrics.totalTasks
               << " | " << row.metrics.completionRate * 100.0 << "%"
               << " | " << row.metrics.averageMovingDistance
               << " | " << row.metrics.totalReward
               << " | " << row.metrics.averagePrivacyLoss
               << " | " << row.metrics.algorithmRuntimeMs
               << " |\n";
    }

    output << "\n## Summary\n\n";
    if (bestRow) {
        output << "Best completion rate: " << bestRow->privacyName << " + "
               << bestRow->algorithmName << " at "
               << bestRow->metrics.completionRate * 100.0 << "%.\n";
    } else {
        output << "No experiment rows were generated.\n";
    }

    return output.str();
}

} // namespace gts
