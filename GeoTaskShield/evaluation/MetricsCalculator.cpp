#include "evaluation/MetricsCalculator.h"

#include <cmath>
#include <unordered_map>
#include <vector>

namespace gts {

EvaluationMetrics MetricsCalculator::calculate(const std::vector<Task>& tasks,
                                               const std::vector<Worker>& workers,
                                               const AssignmentResult& assignment,
                                               double averagePrivacyLoss)
{
    std::unordered_map<int, const Task*> tasksById;
    tasksById.reserve(tasks.size());
    for (const Task& task : tasks) {
        tasksById.emplace(task.id, &task);
    }

    std::unordered_map<int, const Worker*> workersById;
    workersById.reserve(workers.size());
    for (const Worker& worker : workers) {
        workersById.emplace(worker.id, &worker);
    }

    EvaluationMetrics metrics;
    metrics.totalTasks = static_cast<int>(tasks.size());
    metrics.completedTasks = static_cast<int>(assignment.assignments.size());
    metrics.averagePrivacyLoss = averagePrivacyLoss;
    metrics.algorithmRuntimeMs = assignment.algorithmRuntimeMs;

    double totalDistance = 0.0;
    int timedOutTasks = 0;
    std::unordered_map<int, int> assignmentCountsByWorkerId;
    assignmentCountsByWorkerId.reserve(workers.size());
    for (const Worker& worker : workers) {
        assignmentCountsByWorkerId.emplace(worker.id, 0);
    }

    for (const Assignment& item : assignment.assignments) {
        const auto taskIt = tasksById.find(item.taskId);
        const auto workerIt = workersById.find(item.workerId);
        if (taskIt == tasksById.end() || workerIt == workersById.end()) {
            continue;
        }

        const double distance =
            workerIt->second->realLocation.distanceTo(taskIt->second->location);
        totalDistance += distance;
        metrics.totalReward += taskIt->second->reward;

        const double speed = workerIt->second->speed > 0.0 ? workerIt->second->speed : 1.0;
        if (distance / speed > taskIt->second->deadline) {
            ++timedOutTasks;
        }

        auto loadIt = assignmentCountsByWorkerId.find(item.workerId);
        if (loadIt != assignmentCountsByWorkerId.end()) {
            ++loadIt->second;
        }
    }

    if (metrics.totalTasks > 0) {
        metrics.completionRate =
            static_cast<double>(metrics.completedTasks) / static_cast<double>(metrics.totalTasks);
    }
    if (!assignment.assignments.empty()) {
        metrics.averageMovingDistance =
            totalDistance / static_cast<double>(assignment.assignments.size());
        metrics.timeoutRate =
            static_cast<double>(timedOutTasks) /
            static_cast<double>(assignment.assignments.size());
    }

    if (!workers.empty()) {
        const double meanLoad =
            static_cast<double>(assignment.assignments.size()) /
            static_cast<double>(workers.size());
        double variance = 0.0;
        double loadSum = 0.0;
        double loadSquareSum = 0.0;

        for (const Worker& worker : workers) {
            const int load = assignmentCountsByWorkerId[worker.id];
            const double loadAsDouble = static_cast<double>(load);
            const double delta = loadAsDouble - meanLoad;
            variance += delta * delta;
            loadSum += loadAsDouble;
            loadSquareSum += loadAsDouble * loadAsDouble;
        }

        variance /= static_cast<double>(workers.size());
        metrics.userLoadStdDev = std::sqrt(variance);
        if (loadSquareSum > 0.0) {
            metrics.fairnessIndex =
                (loadSum * loadSum) /
                (static_cast<double>(workers.size()) * loadSquareSum);
        }
    }

    metrics.privacyUtilityRatio =
        metrics.completionRate / (1.0 + metrics.averagePrivacyLoss);

    return metrics;
}

} // namespace gts
