#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

gts::ExperimentScenario makeScenario(const std::string& name,
                                     int workerCount,
                                     int taskCount,
                                     const std::string& privacyType,
                                     const std::string& algorithmType,
                                     double gridSize,
                                     int k,
                                     double epsilon,
                                     unsigned int seed)
{
    gts::ExperimentScenario scenario;
    scenario.name = name;
    scenario.config.workerCount = workerCount;
    scenario.config.taskCount = taskCount;
    scenario.config.randomSeed = seed;
    scenario.config.privacy.gridSize = gridSize;
    scenario.config.privacy.k = k;
    scenario.config.privacy.epsilon = epsilon;
    scenario.privacyType = privacyType;
    scenario.algorithmType = algorithmType;
    return scenario;
}

} // namespace

int main()
{
    const std::vector<gts::ExperimentScenario> scenarios{
        makeScenario("workers-50-grid-nearest", 50, 20, "grid", "nearest", 10.0, 5, 1.0, 42),
        makeScenario("workers-100-grid-nearest", 100, 50, "grid", "nearest", 10.0, 5, 1.0, 42),
        makeScenario("epsilon-0.5-laplace-score", 100, 50, "laplace", "score", 10.0, 5, 0.5, 42),
        makeScenario("epsilon-2.0-laplace-score", 100, 50, "laplace", "score", 10.0, 5, 2.0, 42),
        makeScenario("k-3-hungarian", 100, 50, "k-anonymity", "hungarian", 10.0, 3, 1.0, 42),
        makeScenario("k-8-hungarian", 100, 50, "k-anonymity", "hungarian", 10.0, 8, 1.0, 42),
        makeScenario("grid-5-score", 100, 50, "grid", "score", 5.0, 5, 1.0, 42),
        makeScenario("grid-20-score", 100, 50, "grid", "score", 20.0, 5, 1.0, 42)
    };

    const gts::BatchExperimentRunner runner;
    const gts::BatchExperimentResult result = runner.run(scenarios);
    const std::string csvPath = "phase5_batch_results.csv";
    const std::string markdownPath = "phase5_batch_report.md";

    const bool csvWritten = gts::BatchExperimentExporter::writeCsvToFile(csvPath, result);
    const bool markdownWritten =
        gts::BatchExperimentExporter::writeMarkdownToFile(markdownPath, result);

    std::cout << gts::BatchExperimentExporter::toMarkdown(result);
    if (csvWritten) {
        std::cout << "\nExported CSV: " << csvPath << '\n';
    }
    if (markdownWritten) {
        std::cout << "Exported Markdown: " << markdownPath << '\n';
    }

    return csvWritten && markdownWritten ? 0 : 1;
}
