#pragma once

#include "model/ExperimentConfig.h"

#include <string>
#include <vector>

namespace gts {

struct PrivacyResult {
    std::vector<Worker> workers;
    double averagePrivacyLoss{};
};

class IPrivacyMechanism {
public:
    virtual ~IPrivacyMechanism() = default;

    virtual std::string name() const = 0;
    virtual PrivacyResult apply(const std::vector<Worker>& workers,
                                const PrivacyContext& context) const = 0;
};

} // namespace gts
