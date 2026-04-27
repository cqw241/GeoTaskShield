#include "assignment/AssignmentAlgorithmFactory.h"
#include "data/CsvExporter.h"
#include "privacy/PrivacyFactory.h"
#include "simulation/SimulationEngine.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main()
{
    using namespace gts;

    SimulationConfig config;
    config.workerCount = 100;
    config.taskCount = 50;
    config.randomSeed = 42;
    config.areaWidth = 100.0;
    config.areaHeight = 100.0;
    config.privacy.gridSize = 10.0;

    config.privacy.epsilon = 1.0;
    config.privacy.k = 5;

    const std::vector<std::string> privacyTypes{"grid", "k-anonymity", "laplace"};
    const std::vector<std::string> algorithmTypes{"nearest", "score", "hungarian"};
    std::vector<ExperimentSummaryRow> summaryRows;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "GeoTaskShield Console Phase 2 Comparison\n";
    std::cout << "Workers: " << config.workerCount << ", Tasks: " << config.taskCount << "\n\n";
    std::cout << std::left
              << std::setw(25) << "Privacy"
              << std::setw(18) << "Algorithm"
              << std::right
              << std::setw(12) << "Completed"
              << std::setw(14) << "Rate(%)"
              << std::setw(14) << "AvgDist"
              << std::setw(14) << "Reward"
              << std::setw(14) << "PrivLoss"
              << std::setw(14) << "RuntimeMs"
              << '\n';

    for (const std::string& privacyType : privacyTypes) {
        for (const std::string& algorithmType : algorithmTypes) {
            auto privacy = PrivacyFactory::create(privacyType);
            auto algorithm = AssignmentAlgorithmFactory::create(algorithmType);
            if (!privacy || !algorithm) {
                continue;
            }

            const std::string privacyName = privacy->name();
            const std::string algorithmName = algorithm->name();
            SimulationEngine engine(std::move(privacy), std::move(algorithm));
            const SimulationRunResult result = engine.run(config);
            summaryRows.push_back(ExperimentSummaryRow{
                privacyName,
                algorithmName,
                result.metrics
            });

            std::cout << std::left
                      << std::setw(25) << privacyName
                      << std::setw(18) << algorithmName
                      << std::right
                      << std::setw(12) << result.metrics.completedTasks
                      << std::setw(14) << result.metrics.completionRate * 100.0
                      << std::setw(14) << result.metrics.averageMovingDistance
                      << std::setw(14) << result.metrics.totalReward
                      << std::setw(14) << result.metrics.averagePrivacyLoss
                      << std::setw(14) << result.metrics.algorithmRuntimeMs
                      << '\n';
        }
    }

    const std::string exportPath = "phase2_results.csv";
    if (CsvExporter::writeToFile(exportPath, summaryRows)) {
        std::cout << "\nExported CSV: " << exportPath << '\n';
    }

    return 0;
}
