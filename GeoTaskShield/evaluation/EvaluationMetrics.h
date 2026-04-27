#pragma once

namespace gts {

struct EvaluationMetrics {
    int completedTasks{};
    int totalTasks{};
    double completionRate{};
    double averageMovingDistance{};
    double totalReward{};
    double averagePrivacyLoss{};
    double algorithmRuntimeMs{};
    double userLoadStdDev{};
    double fairnessIndex{};
    double privacyUtilityRatio{};
    double timeoutRate{};
};

} // namespace gts
