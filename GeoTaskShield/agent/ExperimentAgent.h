#pragma once

#include "agent/ExperimentReport.h"
#include "agent/ExperimentRequest.h"

#include <string>

namespace gts {

struct ExperimentAgentResult {
    ExperimentRequest request;
    ExperimentReport report;
    std::string markdown;
};

class ExperimentAgent {
public:
    ExperimentAgentResult run(const std::string& text) const;
};

} // namespace gts
