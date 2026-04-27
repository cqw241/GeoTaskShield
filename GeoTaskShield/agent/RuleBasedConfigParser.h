#pragma once

#include "agent/ExperimentRequest.h"

#include <string>

namespace gts {

class RuleBasedConfigParser {
public:
    ExperimentRequest parse(const std::string& text) const;
};

} // namespace gts
