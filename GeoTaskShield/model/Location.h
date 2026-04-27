#pragma once

#include <cmath>

namespace gts {

struct Location {
    double x{};
    double y{};

    double distanceTo(const Location& other) const
    {
        const double dx = x - other.x;
        const double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

} // namespace gts
