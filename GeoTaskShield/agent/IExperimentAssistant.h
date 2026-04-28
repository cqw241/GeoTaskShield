#pragma once

#include "agent/AssistantRequest.h"
#include "agent/AssistantResponse.h"

namespace gts {

class IExperimentAssistant {
public:
    virtual ~IExperimentAssistant() = default;

    [[nodiscard]] virtual AssistantResponse analyze(
        const AssistantRequest& request) const = 0;
};

} // namespace gts
