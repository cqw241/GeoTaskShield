#pragma once

#include "model/Task.h"
#include "model/Worker.h"

#include <string>
#include <vector>

namespace gts {

struct PrivacyContext {
    double epsilon{1.0};
    int k{5};
    double gridSize{10.0};
};

struct AssignmentContext {
    double alpha{1.0};
    double beta{1.0};
    double gamma{1.0};
    double delta{1.0};
};

struct SimulationConfig {
    int workerCount{100};
    int taskCount{50};
    unsigned int randomSeed{42};
    double areaWidth{100.0};
    double areaHeight{100.0};
    std::string dataProfile{"default"};
    PrivacyContext privacy;
    AssignmentContext assignment;
};

struct SimulationData {
    std::vector<Worker> workers;
    std::vector<Task> tasks;
};

} // namespace gts
