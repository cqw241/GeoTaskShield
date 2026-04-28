#pragma once

#include "agent/ExperimentIntent.h"

#include <string>
#include <vector>

namespace gts {

struct AssistantResponse {
    ExperimentIntent intent;
    std::string intentPreviewMarkdown;
    std::string analysisMarkdown;
    std::vector<std::string> warnings;
    bool success{};
};

} // namespace gts
