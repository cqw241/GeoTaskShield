#include "privacy/GridPrivacy.h"

#include <cmath>

namespace gts {

std::string GridPrivacy::name() const
{
    return "Grid Privacy";
}

PrivacyResult GridPrivacy::apply(const std::vector<Worker>& workers,
                                 const PrivacyContext& context) const
{
    const double gridSize = context.gridSize > 0.0 ? context.gridSize : 1.0;

    PrivacyResult result;
    result.workers.reserve(workers.size());

    double totalLoss = 0.0;
    for (Worker worker : workers) {
        const double gridX = std::floor(worker.realLocation.x / gridSize);
        const double gridY = std::floor(worker.realLocation.y / gridSize);
        worker.exposedLocation = Location{
            gridX * gridSize + gridSize / 2.0,
            gridY * gridSize + gridSize / 2.0
        };
        totalLoss += worker.realLocation.distanceTo(worker.exposedLocation);
        result.workers.push_back(worker);
    }

    if (!workers.empty()) {
        result.averagePrivacyLoss = totalLoss / static_cast<double>(workers.size());
    }

    return result;
}

} // namespace gts
