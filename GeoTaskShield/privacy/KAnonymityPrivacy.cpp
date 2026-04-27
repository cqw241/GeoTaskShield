#include "privacy/KAnonymityPrivacy.h"

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace gts {

std::string KAnonymityPrivacy::name() const
{
    return "K-Anonymity Privacy";
}

PrivacyResult KAnonymityPrivacy::apply(const std::vector<Worker>& workers,
                                       const PrivacyContext& context) const
{
    PrivacyResult result;
    result.workers = workers;
    if (workers.empty()) {
        return result;
    }

    const std::size_t groupSize = std::min<std::size_t>(
        workers.size(), static_cast<std::size_t>(std::max(1, context.k)));

    double totalLoss = 0.0;
    for (std::size_t i = 0; i < workers.size(); ++i) {
        std::vector<std::size_t> indices(workers.size());
        std::iota(indices.begin(), indices.end(), 0U);
        std::sort(indices.begin(), indices.end(), [&](std::size_t lhs, std::size_t rhs) {
            const double lhsDistance =
                workers[i].realLocation.distanceTo(workers[lhs].realLocation);
            const double rhsDistance =
                workers[i].realLocation.distanceTo(workers[rhs].realLocation);
            if (lhsDistance == rhsDistance) {
                return workers[lhs].id < workers[rhs].id;
            }
            return lhsDistance < rhsDistance;
        });

        Location centroid;
        for (std::size_t j = 0; j < groupSize; ++j) {
            centroid.x += workers[indices[j]].realLocation.x;
            centroid.y += workers[indices[j]].realLocation.y;
        }
        centroid.x /= static_cast<double>(groupSize);
        centroid.y /= static_cast<double>(groupSize);

        result.workers[i].exposedLocation = centroid;
        totalLoss += workers[i].realLocation.distanceTo(centroid);
    }

    result.averagePrivacyLoss = totalLoss / static_cast<double>(workers.size());
    return result;
}

} // namespace gts
