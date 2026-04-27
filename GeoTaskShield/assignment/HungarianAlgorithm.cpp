#include "assignment/HungarianAlgorithm.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

namespace gts {

namespace {

struct WorkerSlot {
    int workerId{};
    Location exposedLocation;
};

std::vector<int> minimizeCost(const std::vector<std::vector<double>>& cost)
{
    const int n = static_cast<int>(cost.size());
    const int m = n == 0 ? 0 : static_cast<int>(cost.front().size());

    std::vector<double> u(static_cast<std::size_t>(n + 1), 0.0);
    std::vector<double> v(static_cast<std::size_t>(m + 1), 0.0);
    std::vector<int> p(static_cast<std::size_t>(m + 1), 0);
    std::vector<int> way(static_cast<std::size_t>(m + 1), 0);

    for (int i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;
        std::vector<double> minv(static_cast<std::size_t>(m + 1),
                                 std::numeric_limits<double>::infinity());
        std::vector<bool> used(static_cast<std::size_t>(m + 1), false);

        do {
            used[static_cast<std::size_t>(j0)] = true;
            const int i0 = p[static_cast<std::size_t>(j0)];
            double delta = std::numeric_limits<double>::infinity();
            int j1 = 0;

            for (int j = 1; j <= m; ++j) {
                if (used[static_cast<std::size_t>(j)]) {
                    continue;
                }

                const double current =
                    cost[static_cast<std::size_t>(i0 - 1)][static_cast<std::size_t>(j - 1)] -
                    u[static_cast<std::size_t>(i0)] - v[static_cast<std::size_t>(j)];
                if (current < minv[static_cast<std::size_t>(j)]) {
                    minv[static_cast<std::size_t>(j)] = current;
                    way[static_cast<std::size_t>(j)] = j0;
                }
                if (minv[static_cast<std::size_t>(j)] < delta) {
                    delta = minv[static_cast<std::size_t>(j)];
                    j1 = j;
                }
            }

            for (int j = 0; j <= m; ++j) {
                if (used[static_cast<std::size_t>(j)]) {
                    u[static_cast<std::size_t>(p[static_cast<std::size_t>(j)])] += delta;
                    v[static_cast<std::size_t>(j)] -= delta;
                } else {
                    minv[static_cast<std::size_t>(j)] -= delta;
                }
            }

            j0 = j1;
        } while (p[static_cast<std::size_t>(j0)] != 0);

        do {
            const int j1 = way[static_cast<std::size_t>(j0)];
            p[static_cast<std::size_t>(j0)] = p[static_cast<std::size_t>(j1)];
            j0 = j1;
        } while (j0 != 0);
    }

    std::vector<int> assignment(static_cast<std::size_t>(n), -1);
    for (int j = 1; j <= m; ++j) {
        if (p[static_cast<std::size_t>(j)] > 0) {
            assignment[static_cast<std::size_t>(p[static_cast<std::size_t>(j)] - 1)] = j - 1;
        }
    }
    return assignment;
}

} // namespace

std::string HungarianAlgorithm::name() const
{
    return "Hungarian";
}

AssignmentResult HungarianAlgorithm::assign(const std::vector<Task>& tasks,
                                            const std::vector<Worker>& workers,
                                            const AssignmentContext&) const
{
    const auto start = std::chrono::steady_clock::now();

    AssignmentResult result;
    result.assignments.reserve(tasks.size());
    if (tasks.empty()) {
        return result;
    }

    std::vector<WorkerSlot> slots;
    for (const Worker& worker : workers) {
        for (int i = 0; i < worker.maxTasks; ++i) {
            slots.push_back(WorkerSlot{worker.id, worker.exposedLocation});
        }
    }

    if (slots.empty()) {
        const auto end = std::chrono::steady_clock::now();
        result.algorithmRuntimeMs =
            std::chrono::duration<double, std::milli>(end - start).count();
        return result;
    }

    const std::size_t columnCount = std::max(slots.size(), tasks.size());
    constexpr double dummyCost = 1.0e9;
    std::vector<std::vector<double>> cost(
        tasks.size(), std::vector<double>(columnCount, dummyCost));

    for (std::size_t i = 0; i < tasks.size(); ++i) {
        for (std::size_t j = 0; j < slots.size(); ++j) {
            cost[i][j] = tasks[i].location.distanceTo(slots[j].exposedLocation);
        }
    }

    const std::vector<int> assignment = minimizeCost(cost);
    for (std::size_t taskIndex = 0; taskIndex < assignment.size(); ++taskIndex) {
        const int slotIndex = assignment[taskIndex];
        if (slotIndex < 0 || static_cast<std::size_t>(slotIndex) >= slots.size()) {
            continue;
        }

        result.assignments.push_back(Assignment{
            tasks[taskIndex].id,
            slots[static_cast<std::size_t>(slotIndex)].workerId,
            cost[taskIndex][static_cast<std::size_t>(slotIndex)],
            -cost[taskIndex][static_cast<std::size_t>(slotIndex)]
        });
    }

    const auto end = std::chrono::steady_clock::now();
    result.algorithmRuntimeMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

} // namespace gts
