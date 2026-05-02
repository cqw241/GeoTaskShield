#pragma once

#include "agent/IExperimentAssistant.h"

#include <memory>
#include <string>

namespace gts {

class IHttpClient;

struct LLMProviderConfig {
    std::string apiKeyEnvName{"GTS_LLM_API_KEY"};
    std::string modelEnvName{"GTS_LLM_MODEL"};
    std::string baseUrlEnvName{"GTS_LLM_BASE_URL"};
    std::string timeoutMsEnvName{"GTS_LLM_TIMEOUT_MS"};
    std::string fallbackApiKeyEnvName{"DASHSCOPE_API_KEY"};
    std::string fallbackModelEnvName{"DASHSCOPE_MODEL"};
    std::string fallbackBaseUrlEnvName{"DASHSCOPE_BASE_URL"};
    std::string fallbackTimeoutMsEnvName{"DASHSCOPE_TIMEOUT_MS"};
    std::string defaultModel{"deepseek-v4-flash"};
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
