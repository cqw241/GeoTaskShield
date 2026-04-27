#include "evaluation/MetricsCalculator.h"

#include <unordered_map>

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
    for (const Assignment& item : assignment.assignments) {
        const auto taskIt = tasksById.find(item.taskId);
        const auto workerIt = workersById.find(item.workerId);
        if (taskIt == tasksById.end() || workerIt == workersById.end()) {
            continue;
        }

        totalDistance += workerIt->second->realLocation.distanceTo(taskIt->second->location);
        metrics.totalReward += taskIt->second->reward;
    }

    if (metrics.totalTasks > 0) {
        metrics.completionRate =
            static_cast<double>(metrics.completedTasks) / static_cast<double>(metrics.totalTasks);
    }
    if (!assignment.assignments.empty()) {
        metrics.averageMovingDistance =
            totalDistance / static_cast<double>(assignment.assignments.size());
    }

    return metrics;
}

} // namespace gts
