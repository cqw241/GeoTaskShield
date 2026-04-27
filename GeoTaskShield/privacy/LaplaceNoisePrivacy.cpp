#include "privacy/LaplaceNoisePrivacy.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace gts {

namespace {

double sampleLaplace(std::mt19937& rng, double scale)
{
    std::uniform_real_distribution<double> distribution(-0.5, 0.5);
    const double u = distribution(rng);
    const double sign = u < 0.0 ? -1.0 : 1.0;
    return -scale * sign * std::log(1.0 - 2.0 * std::abs(u));
}

} // namespace

std::string LaplaceNoisePrivacy::name() const
{
    return "Laplace Noise Privacy";
}

PrivacyResult LaplaceNoisePrivacy::apply(const std::vector<Worker>& workers,
                                         const PrivacyContext& context) const
{
    const double epsilon = std::max(context.epsilon, 1e-6);
    const double scale = 1.0 / epsilon;
    std::mt19937 rng(20260427U);

    PrivacyResult result;
    result.workers.reserve(workers.size());

    double totalLoss = 0.0;
    for (Worker worker : workers) {
        worker.exposedLocation = Location{
            worker.realLocation.x + sampleLaplace(rng, scale),
            worker.realLocation.y + sampleLaplace(rng, scale)
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
