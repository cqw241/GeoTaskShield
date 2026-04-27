#pragma once

#include "agent/ExperimentReport.h"

#include <string>

namespace gts {

class ReportGenerator {
public:
    static std::string toMarkdown(const ExperimentReport& report);
};

} // namespace gts
