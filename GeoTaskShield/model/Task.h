#pragma once

#include "model/Location.h"

namespace gts {

enum class TaskType {
    Traffic,
    AirQuality,
    BikeSharing,
    CrowdFlow,
    Noise
};

struct Task {
    int id{};
    Location location;
    double reward{};
    double deadline{};
    int requiredWorkers{1};
    int priority{1};
    TaskType type{TaskType::Traffic};
};

} // namespace gts
