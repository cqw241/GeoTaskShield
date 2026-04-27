#pragma once

#include "model/Location.h"

namespace gts {

struct Worker {
    int id{};
    Location realLocation;
    Location exposedLocation;
    int maxTasks{1};
    double speed{1.0};
    double privacyBudget{1.0};
    double reliability{1.0};
};

} // namespace gts
