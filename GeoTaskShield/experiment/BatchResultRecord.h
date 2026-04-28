#pragma once

#include <string>

namespace gts {

enum class BatchResultMetric {
    CompletionRate,
    AverageTrueDistance,
    TotalReward,
    AveragePrivacyLoss,
    RuntimeMs,
    UserLoadStdDev,
    FairnessIndex,
    PrivacyUtilityRatio,
    TimeoutRate
};

enum class BatchResultSortField {
    Scenario,
    WorkerCount,
    TaskCount,
    Privacy,
    Algorithm,
    CompletionRate,
    AverageTrueDistance,
    TotalReward,
    AveragePrivacyLoss,
    RuntimeMs,
    UserLoadStdDev,
    FairnessIndex,
    PrivacyUtilityRatio,
    TimeoutRate
};

struct BatchResultRecord {
    std::string scenario;
    int workerCount{};
    int taskCount{};
    double gridSize{};
    int k{};
    double epsilon{};
    std::string privacy;
    std::string algorithm;
    int completedTasks{};
    int totalTasks{};
    double completionRate{};
    double averageTrueDistance{};
    double totalReward{};
    double averagePrivacyLoss{};
    double runtimeMs{};
    double userLoadStdDev{};
    double fairnessIndex{};
    double privacyUtilityRatio{};
    double timeoutRate{};
};

struct BatchResultSummary {
    BatchResultRecord record;
    double value{};
};

struct ChartBar {
    std::string label;
    double value{};
};

} // namespace gts
