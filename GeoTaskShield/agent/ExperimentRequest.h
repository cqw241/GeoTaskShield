#pragma once

#include "model/ExperimentConfig.h"

#include <string>
#include <vector>

namespace gts {

struct ExperimentRequest {
    std::string originalText;
    SimulationConfig config;
    std::string privacyType{"grid"};
    std::string algorithmType{"nearest"};
    bool comparePrivacyMechanisms{};
    std::vector<std::string> privacyTypes;
};

} // namespace gts
