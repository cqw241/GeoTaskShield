#include "model/Location.h"
#include "model/Task.h"
#include "model/Worker.h"
#include "model/ExperimentConfig.h"
#include "simulation/DataGenerator.h"
#include "privacy/GridPrivacy.h"
#include "privacy/KAnonymityPrivacy.h"
#include "privacy/LaplaceNoisePrivacy.h"
#include "privacy/PrivacyFactory.h"
#include "assignment/NearestGreedyAlgorithm.h"
#include "assignment/ScoreGreedyAlgorithm.h"
#include "assignment/HungarianAlgorithm.h"
#include "assignment/AssignmentAlgorithmFactory.h"
#include "evaluation/MetricsCalculator.h"
#include "simulation/SimulationEngine.h"
#include "data/CsvExporter.h"
#include "agent/ExperimentAgent.h"
#include "agent/HttpClient.h"
#include "agent/OpenAICompatibleAssistant.h"
#include "agent/ReportGenerator.h"
#include "agent/RuleBasedConfigParser.h"
#include "agent/MockLLMAssistant.h"
#include "agent/RuleBasedAssistant.h"
#include "experiment/BatchExperiment.h"
#include "experiment/BatchExperimentExporter.h"
#include "experiment/BatchResultCsvLoader.h"
#include "experiment/BatchResultModel.h"
#include "experiment/BatchResultRecord.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void require(bool condition, const std::string& message)
{
    if (!condition) {
        std::cerr << "Test failed: " << message << '\n';
        std::exit(1);
    }
}

bool near(double lhs, double rhs, double tolerance = 1e-9)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

bool contains(const std::string& value, const std::string& expected)
{
    return value.find(expected) != std::string::npos;
}

class FakeHttpClient : public gts::IHttpClient {
public:
    mutable int callCount{};
    mutable gts::HttpRequest lastRequest;
    gts::HttpResponse response;

    [[nodiscard]] gts::HttpResponse postJson(
        const gts::HttpRequest& request) const override
    {
        ++callCount;
        lastRequest = request;
        return response;
    }
};

void setEnvValue(const std::string& name, const std::string& value)
{
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

void clearEnvValue(const std::string& name)
{
#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif
}

bool hasHeader(const std::vector<std::pair<std::string, std::string>>& headers,
               const std::string& name,
               const std::string& value)
{
    for (const auto& [headerName, headerValue] : headers) {
        if (headerName == name && headerValue == value) {
            return true;
        }
    }
    return false;
}

std::string writeTempCsv(const std::string& name, const std::string& content)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::ofstream file(path, std::ios::binary);
    file << content;
    return path.string();
}

} // namespace

int main()
{
    using namespace gts;

    require(near(Location{0.0, 0.0}.distanceTo(Location{3.0, 4.0}), 5.0),
            "Location distance should use Euclidean distance.");

    SimulationConfig config;
    config.workerCount = 3;
    config.taskCount = 2;
    config.randomSeed = 7;
    config.areaWidth = 100.0;
    config.areaHeight = 100.0;

    DataGenerator generator;
    const SimulationData data = generator.generate(config);
    require(data.workers.size() == 3, "DataGenerator should create the requested workers.");
    require(data.tasks.size() == 2, "DataGenerator should create the requested tasks.");

    std::vector<Worker> workers{
        Worker{1, Location{12.0, 18.0}, Location{}, 2, 1.0, 1.0, 0.9},
        Worker{2, Location{71.0, 73.0}, Location{}, 1, 1.0, 1.0, 0.8}
    };
    GridPrivacy gridPrivacy;
    PrivacyContext privacyContext;
    privacyContext.gridSize = 10.0;
    const PrivacyResult privacyResult = gridPrivacy.apply(workers, privacyContext);
    require(near(privacyResult.workers[0].exposedLocation.x, 15.0),
            "GridPrivacy should expose the grid center x coordinate.");
    require(near(privacyResult.workers[0].exposedLocation.y, 15.0),
            "GridPrivacy should expose the grid center y coordinate.");
    require(privacyResult.averagePrivacyLoss > 0.0,
            "GridPrivacy should report an average obfuscation distance.");

    std::vector<Worker> anonymityWorkers{
        Worker{1, Location{0.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.9},
        Worker{2, Location{2.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.8},
        Worker{3, Location{10.0, 0.0}, Location{}, 1, 1.0, 1.0, 0.7}
    };
    KAnonymityPrivacy kAnonymity;
    PrivacyContext anonymityContext;
    anonymityContext.k = 2;
    const PrivacyResult anonymityResult =
        kAnonymity.apply(anonymityWorkers, anonymityContext);
    require(anonymityResult.workers.size() == 3,
            "KAnonymityPrivacy should keep the worker count.");
    require(near(anonymityResult.workers[0].exposedLocation.x, 1.0) &&
                near(anonymityResult.workers[0].exposedLocation.y, 0.0),
            "KAnonymityPrivacy should expose the centroid of the nearest k workers.");
    require(anonymityResult.averagePrivacyLoss > 0.0,
            "KAnonymityPrivacy should report an average obfuscation distance.");

    LaplaceNoisePrivacy laplacePrivacy;
    PrivacyContext strongPrivacy;
    strongPrivacy.epsilon = 0.1;
    PrivacyContext weakPrivacy;
    weakPrivacy.epsilon = 10.0;
    const PrivacyResult strongNoise =
        laplacePrivacy.apply(anonymityWorkers, strongPrivacy);
    const PrivacyResult weakNoise =
        laplacePrivacy.apply(anonymityWorkers, weakPrivacy);
    require(strongNoise.workers.size() == anonymityWorkers.size(),
            "LaplaceNoisePrivacy should keep the worker count.");
    require(strongNoise.averagePrivacyLoss > weakNoise.averagePrivacyLoss,
            "LaplaceNoisePrivacy should add more noise for smaller epsilon.");
    require(weakNoise.averagePrivacyLoss > 0.0,
            "LaplaceNoisePrivacy should perturb exposed locations.");

    std::vector<Task> tasks{
        Task{10, Location{16.0, 16.0}, 20.0, 100.0, 1, 1, TaskType::Traffic},
        Task{11, Location{76.0, 72.0}, 30.0, 100.0, 1, 1, TaskType::Noise}
    };
    NearestGreedyAlgorithm algorithm;
    const AssignmentResult assignmentResult =
        algorithm.assign(tasks, privacyResult.workers, AssignmentContext{});
    require(assignmentResult.assignments.size() == 2,
            "NearestGreedyAlgorithm should assign both feasible tasks.");
    require(assignmentResult.assignments[0].taskId == 10 &&
                assignmentResult.assignments[0].workerId == 1,
            "NearestGreedyAlgorithm should choose the closest exposed worker.");
    require(assignmentResult.algorithmRuntimeMs >= 0.0,
            "NearestGreedyAlgorithm should record runtime.");

    std::vector<Worker> scoreWorkers{
        Worker{1, Location{0.0, 0.0}, Location{0.0, 0.0}, 1, 1.0, 0.0, 0.4},
        Worker{2, Location{10.0, 0.0}, Location{10.0, 0.0}, 1, 1.0, 0.0, 1.0}
    };
    std::vector<Task> scoreTasks{
        Task{20, Location{0.0, 0.0}, 0.0, 100.0, 1, 1, TaskType::Traffic}
    };
    AssignmentContext scoreContext;
    scoreContext.alpha = 100.0;
    scoreContext.beta = 0.0;
    scoreContext.gamma = 1.0;
    scoreContext.delta = 0.0;
    ScoreGreedyAlgorithm scoreAlgorithm;
    const AssignmentResult scoreResult =
        scoreAlgorithm.assign(scoreTasks, scoreWorkers, scoreContext);
    require(scoreResult.assignments.size() == 1,
            "ScoreGreedyAlgorithm should assign the feasible task.");
    require(scoreResult.assignments[0].workerId == 2,
            "ScoreGreedyAlgorithm should choose the highest scoring worker.");

    std::vector<Task> matchingTasks{
        Task{30, Location{0.0, 0.0}, 10.0, 100.0, 1, 1, TaskType::Traffic},
        Task{31, Location{10.0, 0.0}, 10.0, 100.0, 1, 1, TaskType::Noise}
    };
    std::vector<Worker> matchingWorkers{
        Worker{7, Location{1.0, 0.0}, Location{1.0, 0.0}, 1, 1.0, 0.0, 0.9},
        Worker{8, Location{9.0, 0.0}, Location{9.0, 0.0}, 1, 1.0, 0.0, 0.9}
    };
    HungarianAlgorithm hungarianAlgorithm;
    const AssignmentResult hungarianResult =
        hungarianAlgorithm.assign(matchingTasks, matchingWorkers, AssignmentContext{});
    require(hungarianResult.assignments.size() == 2,
            "HungarianAlgorithm should assign both tasks when workers are available.");
    require(hungarianResult.assignments[0].taskId == 30 &&
                hungarianResult.assignments[0].workerId == 7,
            "HungarianAlgorithm should minimize cost for the first task.");
    require(hungarianResult.assignments[1].taskId == 31 &&
                hungarianResult.assignments[1].workerId == 8,
            "HungarianAlgorithm should minimize cost for the second task.");

    const auto privacyFromFactory = PrivacyFactory::create("laplace");
    require(privacyFromFactory != nullptr,
            "PrivacyFactory should create known privacy mechanisms.");
    require(privacyFromFactory->name() == "Laplace Noise Privacy",
            "PrivacyFactory should create the requested privacy mechanism.");
    require(PrivacyFactory::create("unknown") == nullptr,
            "PrivacyFactory should return nullptr for unknown privacy mechanisms.");

    const auto algorithmFromFactory = AssignmentAlgorithmFactory::create("hungarian");
    require(algorithmFromFactory != nullptr,
            "AssignmentAlgorithmFactory should create known algorithms.");
    require(algorithmFromFactory->name() == "Hungarian",
            "AssignmentAlgorithmFactory should create the requested algorithm.");
    require(AssignmentAlgorithmFactory::create("unknown") == nullptr,
            "AssignmentAlgorithmFactory should return nullptr for unknown algorithms.");

    const EvaluationMetrics metrics = MetricsCalculator::calculate(
        tasks, workers, assignmentResult, privacyResult.averagePrivacyLoss);
    const double expectedAverageDistance =
        (Location{12.0, 18.0}.distanceTo(Location{16.0, 16.0}) +
         Location{71.0, 73.0}.distanceTo(Location{76.0, 72.0})) /
        2.0;
    require(near(metrics.completionRate, 1.0),
            "MetricsCalculator should count all assigned tasks as completed.");
    require(near(metrics.averageMovingDistance, expectedAverageDistance),
            "MetricsCalculator should use real worker locations for movement distance.");
    require(near(metrics.totalReward, 50.0),
            "MetricsCalculator should sum rewards of completed tasks.");
    require(near(metrics.averagePrivacyLoss, privacyResult.averagePrivacyLoss),
            "MetricsCalculator should carry the average privacy loss.");
    require(metrics.algorithmRuntimeMs >= 0.0,
            "MetricsCalculator should carry algorithm runtime.");

    std::vector<Worker> loadWorkers{
        Worker{101, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0},
        Worker{102, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0},
        Worker{103, Location{0.0, 0.0}, Location{0.0, 0.0}, 3, 1.0, 1.0, 1.0}
    };
    std::vector<Task> loadTasks{
        Task{1010, Location{3.0, 4.0}, 10.0, 4.0, 1, 1, TaskType::Traffic},
        Task{1011, Location{0.0, 0.0}, 20.0, 1.0, 1, 1, TaskType::Noise},
        Task{1012, Location{0.0, 0.0}, 30.0, 1.0, 1, 1, TaskType::AirQuality}
    };
    AssignmentResult loadAssignment;
    loadAssignment.assignments = {
        Assignment{1010, 101, 5.0, 0.0},
        Assignment{1011, 101, 0.0, 0.0},
        Assignment{1012, 102, 0.0, 0.0}
    };
    const EvaluationMetrics loadMetrics =
        MetricsCalculator::calculate(loadTasks, loadWorkers, loadAssignment, 2.0);
    require(near(loadMetrics.userLoadStdDev, std::sqrt(2.0 / 3.0)),
            "MetricsCalculator should report worker load standard deviation.");
    require(near(loadMetrics.fairnessIndex, 0.6),
            "MetricsCalculator should report Jain fairness over worker loads.");
    require(near(loadMetrics.privacyUtilityRatio, 1.0 / 3.0),
            "MetricsCalculator should report completion per privacy loss.");
    require(near(loadMetrics.timeoutRate, 1.0 / 3.0),
            "MetricsCalculator should report assigned task timeout rate.");

    const std::string csv = CsvExporter::toCsv({
        ExperimentSummaryRow{"Grid Privacy", "Nearest Greedy", metrics}
    });
    require(csv.find("privacy,algorithm,completed_tasks,total_tasks") != std::string::npos,
            "CsvExporter should emit a summary header.");
    require(csv.find("Grid Privacy,Nearest Greedy,2,2") != std::string::npos,
            "CsvExporter should emit experiment metric rows.");

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
    clearEnvValue(missingKeyConfig.apiKeyEnvName);
    clearEnvValue(missingKeyConfig.modelEnvName);
    clearEnvValue(missingKeyConfig.baseUrlEnvName);
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

    BatchExperimentRunner batchRunner;
    std::vector<ExperimentScenario> scenarios{
        ExperimentScenario{"grid-small", SimulationConfig{}, "grid", "nearest"},
        ExperimentScenario{"laplace-small", SimulationConfig{}, "laplace", "score"}
    };
    scenarios[0].config.workerCount = 8;
    scenarios[0].config.taskCount = 3;
    scenarios[0].config.randomSeed = 11;
    scenarios[1].config.workerCount = 8;
    scenarios[1].config.taskCount = 3;
    scenarios[1].config.randomSeed = 11;
    scenarios[1].config.privacy.epsilon = 0.5;
    const BatchExperimentResult batchResult = batchRunner.run(scenarios);
    require(batchResult.rows.size() == 2,
            "BatchExperimentRunner should run one result row per scenario.");
    require(batchResult.rows[0].scenarioName == "grid-small",
            "BatchExperimentRunner should preserve scenario names.");
    require(batchResult.rows[0].metrics.totalTasks == 3,
            "BatchExperimentRunner should run scenarios through SimulationEngine.");

    const std::string batchCsv = BatchExperimentExporter::toCsv(batchResult);
    require(contains(batchCsv, "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm"),
            "BatchExperimentExporter should emit scenario configuration columns.");
    require(contains(batchCsv, "grid-small,8,3"),
            "BatchExperimentExporter should emit batch scenario rows.");
    require(contains(batchCsv, "fairness_index"),
            "BatchExperimentExporter should include expanded metrics.");

    const std::string batchMarkdown = BatchExperimentExporter::toMarkdown(batchResult);
    require(contains(batchMarkdown, "# GeoTaskShield Batch Experiment Report"),
            "BatchExperimentExporter should emit a Markdown title.");
    require(contains(batchMarkdown, "grid-small"),
            "BatchExperimentExporter should emit scenario names in Markdown.");

    const std::string validBatchCsv =
        "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
        "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
        "fairness_index,privacy_utility_ratio,timeout_rate\r\n"
        "\"scenario,quoted\",10,5,10,3,1,Grid Privacy,Nearest Greedy,"
        "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\r\n";
    const BatchResultLoadResult validLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv("gts_phase7_valid.csv", validBatchCsv));
    require(validLoad.success, validLoad.errorMessage);
    require(validLoad.records.size() == 1,
            "BatchResultCsvLoader should load one valid row.");
    require(validLoad.records[0].scenario == "scenario,quoted",
            "BatchResultCsvLoader should support quoted fields.");
    require(near(validLoad.records[0].averageTrueDistance, 12.5),
            "BatchResultCsvLoader should map average_moving_distance.");
    require(near(validLoad.records[0].runtimeMs, 0.4),
            "BatchResultCsvLoader should map algorithm_runtime_ms.");

    const std::string aliasCsv =
        "\xEF\xBB\xBFscenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_true_distance,"
        "total_reward,average_privacy_loss,runtimeMs,user_load_stddev,"
        "fairness_index,privacyUtilityRatio,timeout_rate\n"
        "alias,8,3,5,2,0.5,Laplace Noise Privacy,Score Greedy,"
        "3,3,1,4.5,50,1.25,0.7,0.1,0.8,0.44,0.2\n";
    const BatchResultLoadResult aliasLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv("gts_phase7_alias.csv", aliasCsv));
    require(aliasLoad.success, aliasLoad.errorMessage);
    require(near(aliasLoad.records[0].averageTrueDistance, 4.5),
            "BatchResultCsvLoader should accept average_true_distance alias.");
    require(near(aliasLoad.records[0].runtimeMs, 0.7),
            "BatchResultCsvLoader should accept runtimeMs alias.");
    require(near(aliasLoad.records[0].privacyUtilityRatio, 0.44),
            "BatchResultCsvLoader should accept privacyUtilityRatio alias.");

    const BatchResultLoadResult missingColumnLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv(
            "gts_phase7_missing.csv",
            "scenario,workers\nonly,1\n"));
    require(!missingColumnLoad.success,
            "BatchResultCsvLoader should reject missing required columns.");
    require(contains(missingColumnLoad.errorMessage, "tasks"),
            "Missing-column errors should include the missing column name.");

    const BatchResultLoadResult invalidNumberLoad =
        BatchResultCsvLoader::loadFromFile(writeTempCsv(
            "gts_phase7_invalid.csv",
            "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
            "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
            "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
            "fairness_index,privacy_utility_ratio,timeout_rate\n"
            "bad,not-a-number,5,10,3,1,Grid Privacy,Nearest Greedy,"
            "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\n"));
    require(!invalidNumberLoad.success,
            "BatchResultCsvLoader should reject invalid numeric cells.");
    require(contains(invalidNumberLoad.errorMessage, "workers") &&
                contains(invalidNumberLoad.errorMessage, "not-a-number"),
            "Invalid numeric errors should include row, column, and raw value.");

    BatchResultModel resultModel;
    resultModel.setRecords({
        BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Grid Privacy", "Nearest Greedy",
                          5, 5, 1.0, 5.0, 100.0, 2.0, 0.3, 0.4, 0.7, 0.33, 0.0},
        BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Laplace Noise Privacy", "Score Greedy",
                          4, 5, 0.8, 3.0, 80.0, 1.0, 0.2, 0.2, 0.9, 0.40, 0.1},
        BatchResultRecord{"s2", 20, 8, 5.0, 5, 0.5, "Grid Privacy", "Hungarian",
                          6, 8, 0.75, 8.0, 120.0, 4.0, 0.1, 0.8, 0.6, 0.15, 0.2}
    });
    resultModel.setPrivacyFilter("Grid Privacy");
    require(resultModel.filteredRecords().size() == 2,
            "BatchResultModel should filter by privacy.");
    const std::string filteredCsv = resultModel.csvReport();
    require(contains(filteredCsv,
                     "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm"),
            "BatchResultModel should export filtered results with the batch CSV header.");
    require(contains(filteredCsv, "s1,10,5,10,3,1,Grid Privacy,Nearest Greedy"),
            "BatchResultModel should export matching filtered rows.");
    require(!contains(filteredCsv, "Laplace Noise Privacy"),
            "BatchResultModel should exclude rows outside the active filter.");
    resultModel.setAlgorithmFilter("Hungarian");
    require(resultModel.filteredRecords().size() == 1 &&
                resultModel.filteredRecords()[0].scenario == "s2",
            "BatchResultModel should filter by privacy and algorithm.");
    resultModel.clearFilters();

    const auto completion = resultModel.bestCompletionRate();
    require(completion.has_value() &&
                completion->record.scenario == "s1" &&
                near(completion->value, 1.0),
            "BatchResultModel should report best completion rate with source record.");
    const auto utility = resultModel.bestPrivacyUtilityRatio();
    require(utility.has_value() &&
                utility->record.privacy == "Laplace Noise Privacy",
            "BatchResultModel should report best privacy-utility source record.");
    const auto fairness = resultModel.bestFairnessIndex();
    require(fairness.has_value() && near(fairness->value, 0.9),
            "BatchResultModel should report best fairness.");
    const auto privacyLoss = resultModel.lowestAveragePrivacyLoss();
    require(privacyLoss.has_value() && near(privacyLoss->value, 1.0),
            "BatchResultModel should report lowest average privacy loss.");

    const std::vector<BatchResultRecord> sortedBatchResults =
        resultModel.sortedRecords(BatchResultSortField::AveragePrivacyLoss, true);
    require(sortedBatchResults.front().privacy == "Laplace Noise Privacy",
            "BatchResultModel should sort numeric metrics numerically.");
    const std::vector<ChartBar> bars =
        resultModel.chartBars(BatchResultMetric::PrivacyUtilityRatio);
    require(bars.size() == 3 &&
                contains(bars[0].label, "s1") &&
                contains(bars[0].label, "Grid Privacy") &&
                contains(bars[0].label, "Nearest Greedy"),
            "BatchResultModel should build chart labels from scenario, privacy, and algorithm.");

    SimulationEngine engine(
        std::make_unique<GridPrivacy>(),
        std::make_unique<NearestGreedyAlgorithm>());
    SimulationConfig runConfig;
    runConfig.workerCount = 10;
    runConfig.taskCount = 4;
    runConfig.randomSeed = 42;
    runConfig.privacy.gridSize = 10.0;
    const SimulationRunResult runResult = engine.run(runConfig);
    require(runResult.data.workers.size() == 10,
            "SimulationEngine should generate the configured number of workers.");
    require(runResult.data.tasks.size() == 4,
            "SimulationEngine should generate the configured number of tasks.");
    require(runResult.privacy.workers.size() == 10,
            "SimulationEngine should apply privacy to generated workers.");
    require(runResult.assignment.assignments.size() <= 4,
            "SimulationEngine should not assign more tasks than generated.");
    require(runResult.metrics.completionRate >= 0.0 &&
                runResult.metrics.completionRate <= 1.0,
            "SimulationEngine should produce a bounded completion rate.");

    return 0;
}
