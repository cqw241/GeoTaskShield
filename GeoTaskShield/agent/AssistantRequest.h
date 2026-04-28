#pragma once

#include "experiment/BatchResultRecord.h"

#include <string>
#include <vector>

namespace gts {

struct AssistantRequest {
    std::string prompt;
    std::vector<BatchResultRecord> batchResults;
    std::string sourceLabel;
};

} // namespace gts
