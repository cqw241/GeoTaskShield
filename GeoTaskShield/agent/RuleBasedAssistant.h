#pragma once

#include "agent/IExperimentAssistant.h"

namespace gts {

class RuleBasedAssistant : public IExperimentAssistant {
public:
    [[nodiscard]] AssistantResponse analyze(
        const AssistantRequest& request) const override;
};

} // namespace gts
