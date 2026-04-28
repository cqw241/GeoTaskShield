#include "agent/RuleBasedAssistant.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>

namespace gts {

namespace {

std::string asciiLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool contains(const std::string& value, const std::string& needle)
{
    return value.find(needle) != std::string::npos;
}

void addUnique(std::vector<std::string>& values, const std::string& value)
{
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

bool extractInt(const std::string& text, const std::regex& pattern, int& output)
{
    std::smatch match;
    if (!std::regex_search(text, match, pattern) || match.size() < 2) {
        return false;
    }
    output = std::stoi(match[1].str());
    return true;
}

std::string joinOrNone(const std::vector<std::string>& values)
{
    if (values.empty()) {
        return "none detected";
    }

    std::ostringstream output;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            output << ", ";
        }
        output << values[index];
    }
    return output.str();
}

std::string recordIdentity(const BatchResultRecord& record)
{
    std::ostringstream output;
    output << record.scenario << " | " << record.privacy << " | " << record.algorithm;
    return output.str();
}

std::size_t uniquePrivacyCount(const std::vector<BatchResultRecord>& records)
{
    std::set<std::string> values;
    for (const BatchResultRecord& record : records) {
        values.insert(record.privacy);
    }
    return values.size();
}

std::size_t uniqueAlgorithmCount(const std::vector<BatchResultRecord>& records)
{
    std::set<std::string> values;
    for (const BatchResultRecord& record : records) {
        values.insert(record.algorithm);
    }
    return values.size();
}

std::string formatRate(double value)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(2) << value * 100.0 << "%";
    return output.str();
}

std::string formatNumber(double value)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(4) << value;
    return output.str();
}

const BatchResultRecord* maxByCompletionRate(
    const std::vector<BatchResultRecord>& records)
{
    if (records.empty()) {
        return nullptr;
    }
    return &*std::max_element(records.begin(), records.end(),
                              [](const BatchResultRecord& lhs,
                                 const BatchResultRecord& rhs) {
                                  return lhs.completionRate < rhs.completionRate;
                              });
}

const BatchResultRecord* maxByPrivacyUtilityRatio(
    const std::vector<BatchResultRecord>& records)
{
    if (records.empty()) {
        return nullptr;
    }
    return &*std::max_element(records.begin(), records.end(),
                              [](const BatchResultRecord& lhs,
                                 const BatchResultRecord& rhs) {
                                  return lhs.privacyUtilityRatio <
                                         rhs.privacyUtilityRatio;
                              });
}

const BatchResultRecord* minByAveragePrivacyLoss(
    const std::vector<BatchResultRecord>& records)
{
    if (records.empty()) {
        return nullptr;
    }
    return &*std::min_element(records.begin(), records.end(),
                              [](const BatchResultRecord& lhs,
                                 const BatchResultRecord& rhs) {
                                  return lhs.averagePrivacyLoss <
                                         rhs.averagePrivacyLoss;
                              });
}

const BatchResultRecord* maxByFairnessIndex(
    const std::vector<BatchResultRecord>& records)
{
    if (records.empty()) {
        return nullptr;
    }
    return &*std::max_element(records.begin(), records.end(),
                              [](const BatchResultRecord& lhs,
                                 const BatchResultRecord& rhs) {
                                  return lhs.fairnessIndex < rhs.fairnessIndex;
                              });
}

ExperimentIntent parseIntent(const std::string& prompt)
{
    ExperimentIntent intent;
    intent.originalText = prompt;

    const std::string lower = asciiLower(prompt);
    int intValue = 0;

    if (extractInt(prompt,
                   std::regex(R"((\d+)\s*(?:个)?\s*(?:用户|移动用户|workers?|worker|users?|user))"),
                   intValue)) {
        intent.workerCount = intValue;
    }
    if (extractInt(prompt,
                   std::regex(R"((\d+)\s*(?:个)?\s*(?:任务|tasks?|task))"),
                   intValue)) {
        intent.taskCount = intValue;
    }

    if (contains(lower, "grid") || contains(prompt, "网格")) {
        addUnique(intent.privacyTypes, "grid");
    }
    if (contains(lower, "laplace") || contains(prompt, "拉普拉斯")) {
        addUnique(intent.privacyTypes, "laplace");
    }
    if (contains(lower, "k-anonymity") || contains(lower, "k anonymity") ||
        contains(lower, "kanonymity") || contains(prompt, "k匿名") ||
        contains(prompt, "k-匿名") || std::regex_search(lower, std::regex(R"(k\s*=)"))) {
        addUnique(intent.privacyTypes, "k-anonymity");
    }

    if (contains(lower, "hungarian") || contains(prompt, "匈牙利")) {
        addUnique(intent.algorithmTypes, "hungarian");
    }
    if (contains(lower, "score") || contains(prompt, "评分") ||
        contains(prompt, "综合")) {
        addUnique(intent.algorithmTypes, "score");
    }
    if (contains(lower, "nearest") || contains(prompt, "最近") ||
        contains(prompt, "贪心")) {
        addUnique(intent.algorithmTypes, "nearest");
    }

    if (contains(lower, "completion") || contains(prompt, "完成率") ||
        contains(prompt, "完成")) {
        addUnique(intent.metricNames, "completionRate");
    }
    if (contains(lower, "privacy utility") ||
        contains(lower, "privacy-utility") || contains(prompt, "隐私效用")) {
        addUnique(intent.metricNames, "privacyUtilityRatio");
    }
    if (contains(lower, "privacy loss") || contains(prompt, "隐私损失")) {
        addUnique(intent.metricNames, "averagePrivacyLoss");
    }
    if (contains(lower, "fairness") || contains(prompt, "公平")) {
        addUnique(intent.metricNames, "fairnessIndex");
    }
    if (contains(lower, "runtime") || contains(lower, "time cost") ||
        contains(prompt, "运行时间") || contains(prompt, "耗时")) {
        addUnique(intent.metricNames, "runtimeMs");
    }
    if (contains(lower, "timeout") || contains(prompt, "超时")) {
        addUnique(intent.metricNames, "timeoutRate");
    }
    if (contains(lower, "reward") || contains(prompt, "收益") ||
        contains(prompt, "奖励")) {
        addUnique(intent.metricNames, "totalReward");
    }
    if (contains(lower, "distance") || contains(prompt, "距离")) {
        addUnique(intent.metricNames, "averageTrueDistance");
    }

    intent.compareRequested = contains(lower, "compare") ||
                              contains(lower, "versus") ||
                              contains(lower, " vs ") ||
                              contains(prompt, "对比") ||
                              contains(prompt, "比较");
    if (intent.compareRequested) {
        if (intent.privacyTypes.size() > 1 || contains(lower, "privacy") ||
            contains(prompt, "隐私")) {
            intent.compareTarget = "privacy";
        } else if (intent.algorithmTypes.size() > 1 ||
                   contains(lower, "algorithm") || contains(prompt, "算法")) {
            intent.compareTarget = "algorithm";
        } else {
            intent.compareTarget = "metrics";
        }
    }

    if (prompt.empty()) {
        intent.warnings.push_back("Prompt is empty.");
    }

    return intent;
}

std::string intentPreview(const ExperimentIntent& intent)
{
    std::ostringstream output;
    output << "## Parsed Intent\n\n";
    output << "- originalText: " << intent.originalText << "\n";
    output << "- workers: ";
    if (intent.workerCount) {
        output << *intent.workerCount;
    } else {
        output << "not specified";
    }
    output << "\n";
    output << "- tasks: ";
    if (intent.taskCount) {
        output << *intent.taskCount;
    } else {
        output << "not specified";
    }
    output << "\n";
    output << "- privacy: " << joinOrNone(intent.privacyTypes) << "\n";
    output << "- algorithms: " << joinOrNone(intent.algorithmTypes) << "\n";
    output << "- metrics: " << joinOrNone(intent.metricNames) << "\n";
    output << "- compareRequested: "
           << (intent.compareRequested ? "true" : "false") << "\n";
    if (!intent.compareTarget.empty()) {
        output << "- compareTarget: " << intent.compareTarget << "\n";
    }
    return output.str();
}

std::string analysisMarkdown(const AssistantRequest& request,
                             const ExperimentIntent& intent,
                             const std::string& preview)
{
    std::ostringstream output;
    output << "# GeoTaskShield Agent Assistant Analysis\n\n";
    output << preview << "\n";

    output << "## Data Scope\n\n";
    output << "- Source: "
           << (request.sourceLabel.empty() ? "current Batch Results filter"
                                           : request.sourceLabel)
           << "\n";
    output << "- Rows analyzed: " << request.batchResults.size() << "\n\n";

    output << "## Key Conclusions\n\n";
    if (request.batchResults.empty()) {
        output << "no batch rows are available for metric analysis. Load a Batch "
                  "Results CSV or adjust filters before rerunning analysis.\n\n";
    } else {
        const BatchResultRecord* completion =
            maxByCompletionRate(request.batchResults);
        const BatchResultRecord* utility =
            maxByPrivacyUtilityRatio(request.batchResults);
        const BatchResultRecord* privacyLoss =
            minByAveragePrivacyLoss(request.batchResults);
        const BatchResultRecord* fairness =
            maxByFairnessIndex(request.batchResults);

        output << "- Best completion rate: "
               << formatRate(completion->completionRate) << " from "
               << recordIdentity(*completion) << ".\n";
        output << "- Best privacy-utility ratio: "
               << formatNumber(utility->privacyUtilityRatio) << " from "
               << recordIdentity(*utility) << ".\n";
        output << "- Lowest average privacy loss: "
               << formatNumber(privacyLoss->averagePrivacyLoss) << " from "
               << recordIdentity(*privacyLoss) << ".\n";
        output << "- Best fairness index: "
               << formatNumber(fairness->fairnessIndex) << " from "
               << recordIdentity(*fairness) << ".\n\n";
    }

    output << "## Next Experiment Suggestions\n\n";
    if (request.batchResults.empty()) {
        output << "- Load `phase5_batch_results.csv` in `Batch Results`, then run "
                  "the assistant again.\n";
    } else {
        const BatchResultRecord* completion =
            maxByCompletionRate(request.batchResults);
        const BatchResultRecord* privacyLoss =
            minByAveragePrivacyLoss(request.batchResults);
        const BatchResultRecord* fairness =
            maxByFairnessIndex(request.batchResults);
        if (completion && privacyLoss &&
            recordIdentity(*completion) != recordIdentity(*privacyLoss)) {
            output << "- Sweep nearby privacy parameters around "
                   << completion->privacy << " and " << privacyLoss->privacy
                   << " to inspect the utility/privacy tradeoff.\n";
        }
        if (completion && fairness &&
            completion->fairnessIndex + 0.05 < fairness->fairnessIndex) {
            output << "- Compare assignment algorithms under "
                   << completion->privacy
                   << " to improve fairness without losing too much completion.\n";
        }
        if (intent.compareRequested && request.batchResults.size() < 2) {
            output << "- Clear filters or load a broader CSV before comparing "
                      "privacy mechanisms or algorithms.\n";
        }
        if (intent.compareTarget == "privacy" &&
            uniquePrivacyCount(request.batchResults) < 2) {
            output << "- Clear filters or load a broader CSV before comparing "
                      "privacy mechanisms; the current rows contain only one "
                      "privacy type.\n";
        }
        if (intent.compareTarget == "algorithm" &&
            uniqueAlgorithmCount(request.batchResults) < 2) {
            output << "- Clear filters or load a broader CSV before comparing "
                      "assignment algorithms; the current rows contain only one "
                      "algorithm type.\n";
        }
        output << "- Re-run analysis after changing Batch Results filters to check "
                  "whether conclusions are filter-sensitive.\n";
    }

    return output.str();
}

} // namespace

AssistantResponse RuleBasedAssistant::analyze(const AssistantRequest& request) const
{
    AssistantResponse response;
    response.success = true;
    response.intent = parseIntent(request.prompt);
    response.intentPreviewMarkdown = intentPreview(response.intent);
    response.warnings = response.intent.warnings;
    response.analysisMarkdown =
        analysisMarkdown(request, response.intent, response.intentPreviewMarkdown);
    return response;
}

} // namespace gts
