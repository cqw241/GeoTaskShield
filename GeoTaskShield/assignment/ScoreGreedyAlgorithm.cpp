#include "assignment/ScoreGreedyAlgorithm.h"

#include <chrono>
#include <limits>
#include <vector>

namespace gts {

std::string ScoreGreedyAlgorithm::name() const
{
    return "Score Greedy";
}

AssignmentResult ScoreGreedyAlgorithm::assign(const std::vector<Task>& tasks,
                                              const std::vector<Worker>& workers,
                                              const AssignmentContext& context) const
{
    const auto start = std::chrono::steady_clock::now();

    AssignmentResult result;
    result.assignments.reserve(tasks.size());
    std::vector<int> assignedCounts(workers.size(), 0);

    for (const Task& task : tasks) {
        int bestWorkerIndex = -1;
        double bestScore = -std::numeric_limits<double>::infinity();
        double bestDistance = 0.0;

        for (std::size_t i = 0; i < workers.size(); ++i) {
            if (assignedCounts[i] >= workers[i].maxTasks) {
                continue;
            }

            const double distance = task.location.distanceTo(workers[i].exposedLocation);
            const double score =
                context.alpha * workers[i].reliability +
                context.beta * task.reward -
                context.gamma * distance -
                context.delta * workers[i].privacyBudget;

            if (score > bestScore ||
                (score == bestScore && bestWorkerIndex >= 0 &&
                 workers[i].id < workers[static_cast<std::size_t>(bestWorkerIndex)].id)) {
                bestScore = score;
                bestDistance = distance;
                bestWorkerIndex = static_cast<int>(i);
            }
        }

        if (bestWorkerIndex >= 0) {
            ++assignedCounts[static_cast<std::size_t>(bestWorkerIndex)];
            result.assignments.push_back(Assignment{
                task.id,
                workers[static_cast<std::size_t>(bestWorkerIndex)].id,
                bestDistance,
                bestScore
            });
        }
    }

    const auto end = std::chrono::steady_clock::now();
    result.algorithmRuntimeMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

} // namespace gts
