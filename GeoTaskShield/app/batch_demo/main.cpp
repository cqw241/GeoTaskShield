#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"
#include "experiment/ExperimentPlan.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifndef GEOTASKSHIELD_PROJECT_VERSION
#define GEOTASKSHIELD_PROJECT_VERSION "unknown"
#endif

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

void printUsage()
{
    std::cout << "Usage:\n"
              << "  GeoTaskShieldBatchDemo\n"
              << "  GeoTaskShieldBatchDemo --plan path\\to\\plan.json [--output runs\\label]\n";
}

int runLegacyBatch()
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

} // namespace

int main(int argc, char* argv[])
{
    if (argc == 1) {
        return runLegacyBatch();
    }

    std::string planPath;
    std::string outputDirectory;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        if (arg == "--plan") {
            if (i + 1 >= argc) {
                std::cerr << "--plan requires a file path.\n";
                printUsage();
                return 2;
            }
            planPath = argv[++i];
        } else if (arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "--output requires a directory path.\n";
                printUsage();
                return 2;
            }
            outputDirectory = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            printUsage();
            return 2;
        }
    }

    if (planPath.empty()) {
        std::cerr << "--plan is required when arguments are provided.\n";
        printUsage();
        return 2;
    }

    const gts::ExperimentPlanLoadResult loadResult =
        gts::ExperimentPlanLoader::loadFromFile(planPath);
    if (!loadResult.success) {
        std::cerr << "Could not load experiment plan: " << loadResult.errorMessage << '\n';
        return 1;
    }

    const std::string runDirectory = outputDirectory.empty()
        ? gts::ExperimentPlanRunner::defaultOutputDirectory(loadResult.plan)
        : outputDirectory;
    const gts::ExperimentPlanRunResult runResult =
        gts::ExperimentPlanRunner::runToDirectory(
            loadResult.plan,
            runDirectory,
            GEOTASKSHIELD_PROJECT_VERSION);
    if (!runResult.success) {
        std::cerr << "Experiment plan run failed: " << runResult.errorMessage << '\n';
        return 1;
    }

    std::cout << "Experiment plan: " << loadResult.plan.name << '\n'
              << "Scenarios: " << runResult.scenarioCount << '\n'
              << "Output directory: " << std::filesystem::path(runDirectory).string() << '\n'
              << "Exported CSV: " << (std::filesystem::path(runDirectory) / "results.csv").string() << '\n'
              << "Exported Markdown: " << (std::filesystem::path(runDirectory) / "report.md").string() << '\n';
    return 0;
}
