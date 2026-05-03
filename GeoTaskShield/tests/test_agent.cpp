#include "agent/ExperimentAgent.h"
#include "agent/MockLLMAssistant.h"
#include "agent/OpenAICompatibleAssistant.h"
#include "agent/ReportGenerator.h"
#include "agent/RuleBasedAssistant.h"
#include "agent/RuleBasedConfigParser.h"
#include "tests/TestSupport.h"

#include <memory>
#include <string>

int main()
{
    using namespace gts;
    using namespace gts_test;

    RuleBasedConfigParser parser;
    const ExperimentRequest chineseRequest =
        parser.parse("100 个用户，50 个任务，k=5，使用匈牙利算法");
    require(chineseRequest.config.workerCount == 100,
            "RuleBasedConfigParser should parse Chinese worker counts.");
    require(chineseRequest.config.taskCount == 50,
            "RuleBasedConfigParser should parse Chinese task counts.");
    require(chineseRequest.config.privacy.k == 5,
            "RuleBasedConfigParser should parse k-anonymity values.");
    require(chineseRequest.privacyType == "k-anonymity",
            "RuleBasedConfigParser should infer k-anonymity from k prompts.");
    require(chineseRequest.algorithmType == "hungarian",
            "RuleBasedConfigParser should parse Hungarian algorithm requests.");

    const ExperimentRequest englishRequest =
        parser.parse("80 workers, 40 tasks, epsilon=0.5, use laplace and score greedy");
    require(englishRequest.config.workerCount == 80,
            "RuleBasedConfigParser should parse English worker counts.");
    require(englishRequest.config.taskCount == 40,
            "RuleBasedConfigParser should parse English task counts.");
    require(near(englishRequest.config.privacy.epsilon, 0.5),
            "RuleBasedConfigParser should parse epsilon values.");
    require(englishRequest.privacyType == "laplace",
            "RuleBasedConfigParser should parse Laplace privacy requests.");
    require(englishRequest.algorithmType == "score",
            "RuleBasedConfigParser should parse score greedy requests.");

    const ExperimentRequest comparisonRequest =
        parser.parse("对比三种隐私机制，50个用户，20个任务，使用最近贪心");
    require(comparisonRequest.comparePrivacyMechanisms,
            "RuleBasedConfigParser should detect privacy comparison requests.");
    require(comparisonRequest.privacyTypes.size() == 3,
            "Privacy comparison requests should include three mechanisms.");
    require(comparisonRequest.algorithmType == "nearest",
            "RuleBasedConfigParser should parse nearest greedy requests.");

    ExperimentReport report;
    report.requestText = "100 workers, 50 tasks, use grid and nearest";
    report.rows.push_back(ExperimentReportRow{
        "Grid Privacy",
        "Nearest Greedy",
        EvaluationMetrics{4, 5, 0.8, 12.5, 120.0, 3.2, 0.7}
    });
    const std::string markdown = ReportGenerator::toMarkdown(report);
    require(contains(markdown, "# GeoTaskShield Experiment Report"),
            "ReportGenerator should emit a Markdown title.");
    require(contains(markdown, "| Grid Privacy | Nearest Greedy | 4 | 5 | 80.00% |"),
            "ReportGenerator should emit a Markdown metrics table row.");
    require(contains(markdown, "Best completion rate"),
            "ReportGenerator should emit a short summary.");

    ExperimentAgent agent;
    const ExperimentAgentResult agentResult =
        agent.run("对比三种隐私机制，30个用户，10个任务，使用匈牙利算法");
    require(agentResult.report.rows.size() == 3,
            "ExperimentAgent should run three rows for privacy comparison prompts.");
    require(contains(agentResult.markdown, "Hungarian"),
            "ExperimentAgent should include selected algorithm names in the report.");

    RuleBasedAssistant assistant;
    AssistantRequest assistantRequest;
    assistantRequest.prompt =
        "Compare grid and laplace for 100 workers, 50 tasks. Use hungarian and "
        "score greedy. Focus on completion rate, privacy utility, privacy loss, "
        "and fairness.";
    assistantRequest.sourceLabel = "test filtered rows";
    assistantRequest.batchResults = {
        BatchResultRecord{"completion-best", 100, 50, 10.0, 3, 1.0, "Grid Privacy",
                          "Hungarian", 49, 50, 0.98, 12.0, 200.0, 3.0, 0.8, 0.4,
                          0.72, 0.245, 0.02},
        BatchResultRecord{"utility-best", 100, 50, 10.0, 3, 0.5,
                          "Laplace Noise Privacy", "Score Greedy", 45, 50, 0.90,
                          9.0, 180.0, 0.5, 0.5, 0.2, 0.93, 0.60, 0.01},
        BatchResultRecord{"privacy-best", 100, 50, 10.0, 3, 1.0,
                          "K-Anonymity Privacy", "Nearest Greedy", 40, 50, 0.80,
                          7.0, 150.0, 0.2, 0.4, 0.1, 0.88, 0.30, 0.03}
    };
    const AssistantResponse assistantResponse = assistant.analyze(assistantRequest);
    require(assistantResponse.success,
            "RuleBasedAssistant should return a successful local analysis.");
    require(assistantResponse.intent.workerCount.has_value() &&
                assistantResponse.intent.workerCount.value() == 100,
            "RuleBasedAssistant should parse worker counts.");
    require(assistantResponse.intent.taskCount.has_value() &&
                assistantResponse.intent.taskCount.value() == 50,
            "RuleBasedAssistant should parse task counts.");
    require(assistantResponse.intent.compareRequested,
            "RuleBasedAssistant should detect comparison intent.");
    require(assistantResponse.intent.privacyTypes.size() == 2,
            "RuleBasedAssistant should parse requested privacy mechanisms.");
    require(assistantResponse.intent.algorithmTypes.size() == 2,
            "RuleBasedAssistant should parse requested assignment algorithms.");
    require(assistantResponse.intent.metricNames.size() >= 4,
            "RuleBasedAssistant should parse metric terms.");
    require(contains(assistantResponse.intentPreviewMarkdown, "workers: 100"),
            "RuleBasedAssistant should generate a structured intent preview.");
    require(contains(assistantResponse.analysisMarkdown,
                     "# GeoTaskShield Agent Assistant Analysis"),
            "RuleBasedAssistant should emit a Markdown analysis title.");
    require(contains(assistantResponse.analysisMarkdown, "Best completion rate") &&
                contains(assistantResponse.analysisMarkdown, "completion-best"),
            "RuleBasedAssistant should report the best completionRate row.");
    require(contains(assistantResponse.analysisMarkdown,
                     "Best privacy-utility ratio") &&
                contains(assistantResponse.analysisMarkdown, "utility-best"),
            "RuleBasedAssistant should report the best privacyUtilityRatio row.");
    require(contains(assistantResponse.analysisMarkdown,
                     "Lowest average privacy loss") &&
                contains(assistantResponse.analysisMarkdown, "privacy-best"),
            "RuleBasedAssistant should report the lowest averagePrivacyLoss row.");
    require(contains(assistantResponse.analysisMarkdown, "Best fairness index") &&
                contains(assistantResponse.analysisMarkdown, "utility-best"),
            "RuleBasedAssistant should report the best fairnessIndex row.");
    require(contains(assistantResponse.analysisMarkdown,
                     "Next Experiment Suggestions"),
            "RuleBasedAssistant should provide next-experiment suggestions.");

    AssistantRequest chineseMetricRequest;
    chineseMetricRequest.prompt =
        "比较隐私机制，关注完成率、隐私效用比、隐私损失和公平性";
    chineseMetricRequest.batchResults = {
        BatchResultRecord{"same-privacy-a", 10, 5, 10.0, 3, 1.0, "Grid Privacy",
                          "Nearest Greedy", 4, 5, 0.8, 2.0, 20.0, 1.0, 0.1, 0.2,
                          0.7, 0.4, 0.0},
        BatchResultRecord{"same-privacy-b", 10, 5, 10.0, 3, 1.0, "Grid Privacy",
                          "Hungarian", 5, 5, 1.0, 3.0, 25.0, 2.0, 0.2, 0.3,
                          0.8, 0.33, 0.0}
    };
    const AssistantResponse chineseMetricResponse =
        assistant.analyze(chineseMetricRequest);
    require(chineseMetricResponse.intent.metricNames.size() >= 4,
            "RuleBasedAssistant should parse Chinese metric terms.");
    require(contains(chineseMetricResponse.analysisMarkdown,
                     "Clear filters or load a broader CSV"),
            "RuleBasedAssistant should suggest broader data when privacy comparison has one privacy type.");

    AssistantRequest emptyAssistantRequest;
    emptyAssistantRequest.prompt = "Analyze completion and privacy loss";
    const AssistantResponse emptyAssistantResponse =
        assistant.analyze(emptyAssistantRequest);
    require(contains(emptyAssistantResponse.analysisMarkdown,
                     "no batch rows are available"),
            "RuleBasedAssistant should explain when no batch rows are available.");

    MockLLMAssistant mockAssistant;
    const AssistantResponse mockResponse = mockAssistant.analyze(assistantRequest);
    require(mockResponse.success,
            "MockLLMAssistant should return a deterministic local response.");
    require(contains(mockResponse.analysisMarkdown, "Mock LLM Assistant"),
            "MockLLMAssistant output should be clearly labeled as local mock output.");

    LLMProviderConfig missingKeyConfig;
    missingKeyConfig.apiKeyEnvName = "GEOTASKSHIELD_TEST_MISSING_API_KEY";
    missingKeyConfig.modelEnvName = "GEOTASKSHIELD_TEST_MISSING_MODEL";
    missingKeyConfig.baseUrlEnvName = "GEOTASKSHIELD_TEST_MISSING_BASE_URL";
    missingKeyConfig.timeoutMsEnvName = "GEOTASKSHIELD_TEST_MISSING_TIMEOUT_MS";
    missingKeyConfig.fallbackApiKeyEnvName = "";
    missingKeyConfig.fallbackModelEnvName = "";
    missingKeyConfig.fallbackBaseUrlEnvName = "";
    missingKeyConfig.fallbackTimeoutMsEnvName = "";
    clearEnvValue(missingKeyConfig.apiKeyEnvName);
    clearEnvValue(missingKeyConfig.modelEnvName);
    clearEnvValue(missingKeyConfig.baseUrlEnvName);
    clearEnvValue(missingKeyConfig.timeoutMsEnvName);
    auto missingKeyHttp = std::make_shared<FakeHttpClient>();
    OpenAICompatibleAssistant missingKeyAssistant(missingKeyConfig, missingKeyHttp);
    const AssistantResponse missingKeyResponse =
        missingKeyAssistant.analyze(assistantRequest);
    require(!missingKeyResponse.success,
            "OpenAICompatibleAssistant should fail closed when the API key is missing.");
    require(missingKeyHttp->callCount == 0,
            "OpenAICompatibleAssistant should not call HTTP without an API key.");
    require(contains(missingKeyResponse.analysisMarkdown,
                     missingKeyConfig.apiKeyEnvName),
            "OpenAICompatibleAssistant should explain the missing API key environment variable.");

    LLMProviderConfig llmConfig;
    llmConfig.apiKeyEnvName = "GEOTASKSHIELD_TEST_API_KEY";
    llmConfig.modelEnvName = "GEOTASKSHIELD_TEST_MODEL";
    llmConfig.baseUrlEnvName = "GEOTASKSHIELD_TEST_BASE_URL";
    llmConfig.timeoutMsEnvName = "GEOTASKSHIELD_TEST_TIMEOUT_MS";
    llmConfig.fallbackApiKeyEnvName = "";
    llmConfig.fallbackModelEnvName = "";
    llmConfig.fallbackBaseUrlEnvName = "";
    llmConfig.fallbackTimeoutMsEnvName = "";
    llmConfig.defaultBaseUrl = "https://dashscope.example.test/compatible-mode/v1/";
    llmConfig.defaultModel = "fallback-model";
    llmConfig.requestTimeoutMs = 4321;
    setEnvValue(llmConfig.apiKeyEnvName, "test-key");
    setEnvValue(llmConfig.modelEnvName, "kimi-k2.5");
    setEnvValue(llmConfig.timeoutMsEnvName, "2468");
    clearEnvValue(llmConfig.baseUrlEnvName);

    auto fakeHttp = std::make_shared<FakeHttpClient>();
    fakeHttp->response.success = true;
    fakeHttp->response.statusCode = 200;
    fakeHttp->response.body =
        R"({"choices":[{"message":{"role":"assistant","content":"# LLM Markdown\n\nBest completion rate: remote result."}}]})";
    OpenAICompatibleAssistant llmAssistant(llmConfig, fakeHttp);
    AssistantRequest llmRequest = assistantRequest;
    llmRequest.prompt =
        "Analyze 12 workers and 6 tasks with laplace. Explain completion rate.";
    const AssistantResponse llmResponse = llmAssistant.analyze(llmRequest);
    require(llmResponse.success,
            "OpenAICompatibleAssistant should return successful Markdown from a valid provider response.");
    require(contains(llmResponse.analysisMarkdown, "# LLM Markdown"),
            "OpenAICompatibleAssistant should use assistant content from the provider response.");
    require(llmResponse.intent.workerCount.has_value() &&
                llmResponse.intent.workerCount.value() == 12,
            "OpenAICompatibleAssistant should preserve local parsed intent.");
    require(fakeHttp->callCount == 1,
            "OpenAICompatibleAssistant should make one HTTP request when configured.");
    require(fakeHttp->lastRequest.url ==
                "https://dashscope.example.test/compatible-mode/v1/chat/completions",
            "OpenAICompatibleAssistant should call the OpenAI-compatible chat completions endpoint.");
    require(hasHeader(fakeHttp->lastRequest.headers, "Authorization",
                      "Bearer test-key"),
            "OpenAICompatibleAssistant should send the API key as a bearer token.");
    require(hasHeader(fakeHttp->lastRequest.headers, "Content-Type",
                      "application/json"),
            "OpenAICompatibleAssistant should send JSON content.");
    require(contains(fakeHttp->lastRequest.body, R"("model":"kimi-k2.5")"),
            "OpenAICompatibleAssistant should read the model name from the environment.");
    require(contains(fakeHttp->lastRequest.body, llmRequest.prompt),
            "OpenAICompatibleAssistant should include the user prompt in the request body.");
    require(fakeHttp->lastRequest.timeoutMs == 2468,
            "OpenAICompatibleAssistant should read timeout milliseconds from the environment.");

    setEnvValue(llmConfig.timeoutMsEnvName, "not-a-number");
    fakeHttp->response.success = true;
    fakeHttp->response.statusCode = 200;
    fakeHttp->response.body =
        R"({"choices":[{"message":{"role":"assistant","content":"# LLM Markdown"}}]})";
    const AssistantResponse invalidTimeoutResponse =
        llmAssistant.analyze(llmRequest);
    require(invalidTimeoutResponse.success,
            "OpenAICompatibleAssistant should ignore invalid timeout environment values.");
    require(fakeHttp->lastRequest.timeoutMs == 4321,
            "OpenAICompatibleAssistant should fall back to configured timeout for invalid timeout values.");

    fakeHttp->response = HttpResponse{false, 504, {}, "request timed out"};
    const AssistantResponse timeoutResponse = llmAssistant.analyze(llmRequest);
    require(!timeoutResponse.success,
            "OpenAICompatibleAssistant should report HTTP transport failures.");
    require(contains(timeoutResponse.analysisMarkdown, "request timed out"),
            "Provider fallback should include the transport failure message.");
    require(contains(timeoutResponse.analysisMarkdown, "Local Analysis Fallback"),
            "Provider fallback should preserve local analysis after transport failure.");

    fakeHttp->response = HttpResponse{true, 200, R"({"choices":[]})", {}};
    const AssistantResponse emptyContentResponse =
        llmAssistant.analyze(llmRequest);
    require(!emptyContentResponse.success,
            "OpenAICompatibleAssistant should reject provider responses without Markdown content.");
    require(contains(emptyContentResponse.analysisMarkdown,
                     "Local Analysis Fallback"),
            "Provider fallback should preserve local analysis for empty provider content.");

    fakeHttp->response = HttpResponse{true, 200, R"({"unexpected":true})", {}};
    const AssistantResponse malformedResponse = llmAssistant.analyze(llmRequest);
    require(!malformedResponse.success,
            "OpenAICompatibleAssistant should reject malformed provider responses.");
    require(contains(malformedResponse.analysisMarkdown,
                     "did not contain assistant Markdown content"),
            "Malformed provider responses should explain the missing Markdown content.");
    clearEnvValue(llmConfig.apiKeyEnvName);
    clearEnvValue(llmConfig.modelEnvName);
    clearEnvValue(llmConfig.timeoutMsEnvName);

    LLMProviderConfig defaultProviderConfig;
    clearEnvValue("GTS_LLM_API_KEY");
    clearEnvValue("GTS_LLM_MODEL");
    clearEnvValue("GTS_LLM_BASE_URL");
    clearEnvValue("GTS_LLM_TIMEOUT_MS");
    clearEnvValue("DASHSCOPE_API_KEY");
    clearEnvValue("DASHSCOPE_MODEL");
    clearEnvValue("DASHSCOPE_BASE_URL");
    clearEnvValue("DASHSCOPE_TIMEOUT_MS");
    setEnvValue("GTS_LLM_API_KEY", "generic-key");
    auto defaultProviderHttp = std::make_shared<FakeHttpClient>();
    defaultProviderHttp->response =
        HttpResponse{true, 200,
                     R"({"choices":[{"message":{"content":"# Generic LLM"}}]})",
                     {}};
    OpenAICompatibleAssistant defaultProviderAssistant(defaultProviderConfig,
                                                       defaultProviderHttp);
    const AssistantResponse defaultProviderResponse =
        defaultProviderAssistant.analyze(llmRequest);
    require(defaultProviderResponse.success,
            "OpenAI-compatible provider should use the generic API key environment variable.");
    require(defaultProviderHttp->lastRequest.url ==
                "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
            "OpenAI-compatible provider should keep the DashScope-compatible endpoint as the default.");
    require(contains(defaultProviderHttp->lastRequest.body,
                     R"("model":"deepseek-v4-flash")"),
            "OpenAI-compatible provider should default to deepseek-v4-flash.");

    clearEnvValue("GTS_LLM_API_KEY");
    setEnvValue("DASHSCOPE_API_KEY", "legacy-key");
    setEnvValue("DASHSCOPE_MODEL", "legacy-model");
    setEnvValue("DASHSCOPE_BASE_URL", "https://legacy.example.test/v1");
    setEnvValue("DASHSCOPE_TIMEOUT_MS", "9876");
    defaultProviderHttp->response =
        HttpResponse{true, 200,
                     R"({"choices":[{"message":{"content":"# Legacy LLM"}}]})",
                     {}};
    const AssistantResponse legacyProviderResponse =
        defaultProviderAssistant.analyze(llmRequest);
    require(legacyProviderResponse.success,
            "OpenAI-compatible provider should keep DASHSCOPE_* as compatibility fallback.");
    require(hasHeader(defaultProviderHttp->lastRequest.headers, "Authorization",
                      "Bearer legacy-key"),
            "OpenAI-compatible provider should read legacy API keys when generic keys are absent.");
    require(defaultProviderHttp->lastRequest.url ==
                "https://legacy.example.test/v1/chat/completions",
            "OpenAI-compatible provider should read the legacy base URL fallback.");
    require(contains(defaultProviderHttp->lastRequest.body,
                     R"("model":"legacy-model")"),
            "OpenAI-compatible provider should read the legacy model fallback.");
    require(defaultProviderHttp->lastRequest.timeoutMs == 9876,
            "OpenAI-compatible provider should read the legacy timeout fallback.");

    setEnvValue("GTS_LLM_API_KEY", "generic-key");
    setEnvValue("GTS_LLM_MODEL", "generic-model");
    setEnvValue("GTS_LLM_BASE_URL", "https://generic.example.test/v1");
    setEnvValue("GTS_LLM_TIMEOUT_MS", "6789");
    defaultProviderHttp->response =
        HttpResponse{true, 200,
                     R"({"choices":[{"message":{"content":"# Generic LLM"}}]})",
                     {}};
    const AssistantResponse genericProviderResponse =
        defaultProviderAssistant.analyze(llmRequest);
    require(genericProviderResponse.success,
            "OpenAI-compatible provider should prefer GTS_LLM_* over DASHSCOPE_*.");
    require(hasHeader(defaultProviderHttp->lastRequest.headers, "Authorization",
                      "Bearer generic-key"),
            "OpenAI-compatible provider should prefer the generic API key.");
    require(defaultProviderHttp->lastRequest.url ==
                "https://generic.example.test/v1/chat/completions",
            "OpenAI-compatible provider should prefer the generic base URL.");
    require(contains(defaultProviderHttp->lastRequest.body,
                     R"("model":"generic-model")"),
            "OpenAI-compatible provider should prefer the generic model.");
    require(defaultProviderHttp->lastRequest.timeoutMs == 6789,
            "OpenAI-compatible provider should prefer the generic timeout.");
    clearEnvValue("GTS_LLM_API_KEY");
    clearEnvValue("GTS_LLM_MODEL");
    clearEnvValue("GTS_LLM_BASE_URL");
    clearEnvValue("GTS_LLM_TIMEOUT_MS");
    clearEnvValue("DASHSCOPE_API_KEY");
    clearEnvValue("DASHSCOPE_MODEL");
    clearEnvValue("DASHSCOPE_BASE_URL");
    clearEnvValue("DASHSCOPE_TIMEOUT_MS");

    return 0;
}
