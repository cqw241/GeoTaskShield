#pragma once

#include <optional>
#include <string>
#include <vector>

namespace gts {

struct ExperimentIntent {
    std::string originalText;
    std::optional<int> workerCount;
    std::optional<int> taskCount;
    std::vector<std::string> privacyTypes;
    std::vector<std::string> algorithmTypes;
    std::vector<std::string> metricNames;
    bool compareRequested{};
    std::string compareTarget;
    std::vector<std::string> warnings;
};

} // namespace gts
