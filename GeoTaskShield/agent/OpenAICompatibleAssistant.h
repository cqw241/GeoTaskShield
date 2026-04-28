#pragma once

#include "agent/IExperimentAssistant.h"

#include <memory>
#include <string>

namespace gts {

class IHttpClient;

struct LLMProviderConfig {
    std::string apiKeyEnvName{"DASHSCOPE_API_KEY"};
    std::string modelEnvName{"DASHSCOPE_MODEL"};
    std::string baseUrlEnvName{"DASHSCOPE_BASE_URL"};
    std::string timeoutMsEnvName{"DASHSCOPE_TIMEOUT_MS"};
    std::string defaultModel{"kimi-k2.5"};
    std::string defaultBaseUrl{
        "https://dashscope.aliyuncs.com/compatible-mode/v1"};
    int requestTimeoutMs{15000};
};

class OpenAICompatibleAssistant : public IExperimentAssistant {
public:
    explicit OpenAICompatibleAssistant(LLMProviderConfig config = {});
    OpenAICompatibleAssistant(LLMProviderConfig config,
                              std::shared_ptr<IHttpClient> httpClient);

    [[nodiscard]] AssistantResponse analyze(
        const AssistantRequest& request) const override;

private:
    LLMProviderConfig config_;
    std::shared_ptr<IHttpClient> httpClient_;
};

} // namespace gts
