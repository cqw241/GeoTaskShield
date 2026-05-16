#pragma once

#include "experiment/BatchExperiment.h"

#include <string>
#include <vector>

namespace gts {

struct ExperimentPlan {
    std::string name;
    std::string runLabel;
    std::vector<int> workers;
    std::vector<int> tasks;
    std::vector<unsigned int> seeds;
    std::vector<std::string> privacyMechanisms;
    std::vector<std::string> assignmentAlgorithms;
    std::vector<double> gridSizes;
    std::vector<int> kValues;
    std::vector<double> epsilons;
    bool hasAreaWidth{false};
    bool hasAreaHeight{false};
    double areaWidth{};
    double areaHeight{};
    std::string sourceJson;

    [[nodiscard]] std::vector<ExperimentScenario> expandToScenarios() const;
};

struct ExperimentPlanLoadResult {
    bool success{false};
    ExperimentPlan plan;
    std::string errorMessage;
};

class ExperimentPlanLoader {
public:
    static ExperimentPlanLoadResult loadFromFile(const std::string& filePath);
    static ExperimentPlanLoadResult loadFromString(const std::string& json);
};

struct ExperimentPlanRunResult {
    bool success{false};
    std::string errorMessage;
    std::string outputDirectory;
    std::size_t scenarioCount{};
};

class ExperimentPlanRunner {
public:
    static ExperimentPlanRunResult runToDirectory(
        const ExperimentPlan& plan,
        const std::string& outputDirectory,
        const std::string& projectVersion = "unknown");

    static std::string defaultOutputDirectory(const ExperimentPlan& plan);
    static std::string sanitizeRunLabel(const std::string& value);
};

} // namespace gts
