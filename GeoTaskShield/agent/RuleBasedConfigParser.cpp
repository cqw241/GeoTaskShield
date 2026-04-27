#include "agent/RuleBasedConfigParser.h"

#include <algorithm>
#include <cctype>
#include <regex>

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

bool extractInt(const std::string& text, const std::regex& pattern, int& output)
{
    std::smatch match;
    if (!std::regex_search(text, match, pattern) || match.size() < 2) {
        return false;
    }
    output = std::stoi(match[1].str());
    return true;
}

bool extractDouble(const std::string& text, const std::regex& pattern, double& output)
{
    std::smatch match;
    if (!std::regex_search(text, match, pattern) || match.size() < 2) {
        return false;
    }
    output = std::stod(match[1].str());
    return true;
}

} // namespace

ExperimentRequest RuleBasedConfigParser::parse(const std::string& text) const
{
    ExperimentRequest request;
    request.originalText = text;

    const std::string lower = asciiLower(text);
    int intValue = 0;
    double doubleValue = 0.0;

    if (extractInt(text, std::regex(R"((\d+)\s*(?:个)?\s*(?:用户|移动用户|workers?|worker))"),
                   intValue)) {
        request.config.workerCount = intValue;
    }
    if (extractInt(text, std::regex(R"((\d+)\s*(?:个)?\s*(?:任务|tasks?|task))"),
                   intValue)) {
        request.config.taskCount = intValue;
    }
    if (extractInt(lower, std::regex(R"(k\s*=?\s*(\d+))"), intValue)) {
        request.config.privacy.k = intValue;
        request.privacyType = "k-anonymity";
    }
    if (extractDouble(lower, std::regex(R"((?:epsilon|eps|ε)\s*=?\s*(\d+(?:\.\d+)?))"),
                      doubleValue)) {
        request.config.privacy.epsilon = doubleValue;
    }
    if (extractDouble(lower, std::regex(R"((?:grid\s*size|grid|网格)\s*=?\s*(\d+(?:\.\d+)?))"),
                      doubleValue)) {
        request.config.privacy.gridSize = doubleValue;
    }

    if (contains(lower, "laplace") || contains(text, "拉普拉斯")) {
        request.privacyType = "laplace";
    } else if (contains(lower, "k-anonymity") || contains(lower, "kanonymity") ||
               contains(text, "k匿名") || contains(text, "k-匿名") ||
               contains(text, "K匿名") || contains(text, "K-匿名")) {
        request.privacyType = "k-anonymity";
    } else if (contains(lower, "grid") || contains(text, "网格")) {
        request.privacyType = "grid";
    }

    if (contains(lower, "hungarian") || contains(text, "匈牙利")) {
        request.algorithmType = "hungarian";
    } else if (contains(lower, "score") || contains(text, "评分") ||
               contains(text, "综合")) {
        request.algorithmType = "score";
    } else if (contains(lower, "nearest") || contains(text, "最近") ||
               contains(text, "贪心")) {
        request.algorithmType = "nearest";
    }

    const bool asksComparison =
        contains(lower, "compare") || contains(text, "对比") || contains(text, "比较");
    const bool asksPrivacy =
        contains(lower, "privacy") || contains(text, "隐私") || contains(text, "机制");
    request.comparePrivacyMechanisms = asksComparison && asksPrivacy;
    if (request.comparePrivacyMechanisms) {
        request.privacyTypes = {"grid", "k-anonymity", "laplace"};
    }

    return request;
}

} // namespace gts
