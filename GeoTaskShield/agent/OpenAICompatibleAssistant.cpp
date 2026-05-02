#include "agent/OpenAICompatibleAssistant.h"

#include "agent/HttpClient.h"
#include "agent/RuleBasedAssistant.h"
#include "agent/WinHttpClient.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

namespace gts {

namespace {

std::string envValue(const std::string& name)
{
    if (name.empty()) {
        return {};
    }
#ifdef _WIN32
    char* rawValue{};
    std::size_t valueSize{};
    if (_dupenv_s(&rawValue, &valueSize, name.c_str()) != 0 ||
        rawValue == nullptr) {
        return {};
    }
    std::string value(rawValue);
    std::free(rawValue);
    return value;
#else
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return {};
    }
    return value;
#endif
}

std::string configuredEnvValue(const std::string& envName,
                               const std::string& fallbackEnvName)
{
    const std::string value = envValue(envName);
    if (!value.empty()) {
        return value;
    }
    return envValue(fallbackEnvName);
}

std::string effectiveValue(const std::string& envName,
                           const std::string& fallbackEnvName,
                           const std::string& fallback)
{
    const std::string value = configuredEnvValue(envName, fallbackEnvName);
    if (!value.empty()) {
        return value;
    }
    return fallback;
}

int effectivePositiveInt(const std::string& envName,
                         const std::string& fallbackEnvName,
                         int fallback)
{
    const std::string value = configuredEnvValue(envName, fallbackEnvName);
    if (value.empty()) {
        return fallback;
    }

    int parsed{};
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto [position, error] = std::from_chars(begin, end, parsed);
    if (error == std::errc() && position == end && parsed > 0) {
        return parsed;
    }
    return fallback;
}

std::string chatCompletionsUrl(std::string baseUrl)
{
    while (!baseUrl.empty() && baseUrl.back() == '/') {
        baseUrl.pop_back();
    }
    return baseUrl + "/chat/completions";
}

std::string jsonEscape(std::string_view value)
{
    std::ostringstream output;
    for (const unsigned char ch : value) {
        switch (ch) {
        case '\\':
            output << "\\\\";
            break;
        case '"':
            output << "\\\"";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            if (ch < 0x20) {
                output << "\\u" << std::hex << std::setw(4)
                       << std::setfill('0') << static_cast<int>(ch)
                       << std::dec << std::setfill(' ');
            } else {
                output << static_cast<char>(ch);
            }
            break;
        }
    }
    return output.str();
}

int hexValue(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

void appendUtf8(std::string& output, unsigned int codepoint)
{
    if (codepoint <= 0x7F) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

std::string jsonUnescape(std::string_view value)
{
    std::string output;
    output.reserve(value.size());

    for (std::size_t index = 0; index < value.size(); ++index) {
        const char ch = value[index];
        if (ch != '\\' || index + 1 >= value.size()) {
            output.push_back(ch);
            continue;
        }

        const char escaped = value[++index];
        switch (escaped) {
        case '"':
        case '\\':
        case '/':
            output.push_back(escaped);
            break;
        case 'b':
            output.push_back('\b');
            break;
        case 'f':
            output.push_back('\f');
            break;
        case 'n':
            output.push_back('\n');
            break;
        case 'r':
            output.push_back('\r');
            break;
        case 't':
            output.push_back('\t');
            break;
        case 'u': {
            if (index + 4 >= value.size()) {
                output += "\\u";
                break;
            }
            unsigned int codepoint = 0;
            bool valid = true;
            for (int offset = 0; offset < 4; ++offset) {
                const int digit = hexValue(value[index + 1 + offset]);
                if (digit < 0) {
                    valid = false;
                    break;
                }
                codepoint = (codepoint << 4) | static_cast<unsigned int>(digit);
            }
            if (valid) {
                appendUtf8(output, codepoint);
                index += 4;
            } else {
                output += "\\u";
            }
            break;
        }
        default:
            output.push_back(escaped);
            break;
        }
    }

    return output;
}

std::string findJsonStringValue(const std::string& json, const std::string& key)
{
    const std::string token = "\"" + key + "\"";
    std::size_t position = json.find(token);
    while (position != std::string::npos) {
        const std::size_t colon = json.find(':', position + token.size());
        if (colon == std::string::npos) {
            return {};
        }

        std::size_t quote = colon + 1;
        while (quote < json.size() &&
               (json[quote] == ' ' || json[quote] == '\n' ||
                json[quote] == '\r' || json[quote] == '\t')) {
            ++quote;
        }
        if (quote >= json.size() || json[quote] != '"') {
            position = json.find(token, position + token.size());
            continue;
        }

        std::size_t end = quote + 1;
        bool escaped = false;
        for (; end < json.size(); ++end) {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (json[end] == '\\') {
                escaped = true;
                continue;
            }
            if (json[end] == '"') {
                return jsonUnescape(
                    std::string_view(json).substr(quote + 1, end - quote - 1));
            }
        }
        return {};
    }

    return {};
}

std::string recordLine(const BatchResultRecord& record)
{
    std::ostringstream output;
    output << "- " << record.scenario << " | workers=" << record.workerCount
           << " tasks=" << record.taskCount << " | privacy=" << record.privacy
           << " | algorithm=" << record.algorithm
           << " | completionRate=" << record.completionRate
           << " | privacyUtilityRatio=" << record.privacyUtilityRatio
           << " | averagePrivacyLoss=" << record.averagePrivacyLoss
           << " | fairnessIndex=" << record.fairnessIndex << "\n";
    return output.str();
}

std::string buildProviderPrompt(const AssistantRequest& request,
                                const AssistantResponse& localResponse)
{
    std::ostringstream prompt;
    prompt << "User request:\n" << request.prompt << "\n\n";
    prompt << "Source label: "
           << (request.sourceLabel.empty() ? "current Batch Results filter"
                                           : request.sourceLabel)
           << "\n\n";
    prompt << "Local parsed intent:\n"
           << localResponse.intentPreviewMarkdown << "\n";
    prompt << "Batch result rows (" << request.batchResults.size()
           << " total, first 25 shown):\n";

    const std::size_t limit = std::min<std::size_t>(25, request.batchResults.size());
    for (std::size_t index = 0; index < limit; ++index) {
        prompt << recordLine(request.batchResults[index]);
    }
    if (request.batchResults.size() > limit) {
        prompt << "- ... " << (request.batchResults.size() - limit)
               << " more rows omitted.\n";
    }

    prompt << "\nReturn a concise Markdown analysis. Include conclusions for "
              "best completionRate, best privacyUtilityRatio, lowest "
              "averagePrivacyLoss, best fairnessIndex, and next experiment "
              "suggestions. Do not ask for or reveal API keys.";
    return prompt.str();
}

std::string requestBody(const std::string& model,
                        const AssistantRequest& request,
                        const AssistantResponse& localResponse)
{
    const std::string systemPrompt =
        "You are the GeoTaskShield experiment assistant. Analyze the supplied "
        "batch-result metrics without changing simulation, privacy, assignment, "
        "or batch experiment semantics. Return Markdown only.";

    std::ostringstream body;
    body << "{\"model\":\"" << jsonEscape(model) << "\",\"messages\":[";
    body << "{\"role\":\"system\",\"content\":\"" << jsonEscape(systemPrompt)
         << "\"},";
    body << "{\"role\":\"user\",\"content\":\""
         << jsonEscape(buildProviderPrompt(request, localResponse)) << "\"}";
    body << "]}";
    return body.str();
}

AssistantResponse unavailableResponse(AssistantResponse response,
                                      const std::string& warning)
{
    response.success = false;
    response.warnings.push_back(warning);
    response.analysisMarkdown =
        "# LLM Provider Unavailable\n\n" + warning +
        "\n\nNo network request was sent unless noted above.\n\n"
        "## Local Analysis Fallback\n\n" +
        response.analysisMarkdown;
    return response;
}

} // namespace

OpenAICompatibleAssistant::OpenAICompatibleAssistant(LLMProviderConfig config)
    : OpenAICompatibleAssistant(std::move(config),
                                std::make_shared<WinHttpClient>())
{
}

OpenAICompatibleAssistant::OpenAICompatibleAssistant(
    LLMProviderConfig config,
    std::shared_ptr<IHttpClient> httpClient)
    : config_(std::move(config))
    , httpClient_(std::move(httpClient))
{
}

AssistantResponse OpenAICompatibleAssistant::analyze(
    const AssistantRequest& request) const
{
    RuleBasedAssistant localAssistant;
    AssistantResponse response = localAssistant.analyze(request);

    const std::string apiKey =
        configuredEnvValue(config_.apiKeyEnvName, config_.fallbackApiKeyEnvName);
    if (apiKey.empty()) {
        std::string warning =
            "Environment variable `" + config_.apiKeyEnvName +
            "` is not set.";
        if (!config_.fallbackApiKeyEnvName.empty()) {
            warning += " Compatibility fallback `" +
                       config_.fallbackApiKeyEnvName + "` is also empty.";
        }
        warning += " Configure it to enable the real LLM provider.";
        return unavailableResponse(
            std::move(response),
            warning);
    }
    if (!httpClient_) {
        return unavailableResponse(std::move(response),
                                   "No HTTP client is configured for the LLM provider.");
    }

    const std::string model =
        effectiveValue(config_.modelEnvName, config_.fallbackModelEnvName,
                       config_.defaultModel);
    const std::string baseUrl =
        effectiveValue(config_.baseUrlEnvName, config_.fallbackBaseUrlEnvName,
                       config_.defaultBaseUrl);
    const int timeoutMs =
        effectivePositiveInt(config_.timeoutMsEnvName,
                             config_.fallbackTimeoutMsEnvName,
                             config_.requestTimeoutMs);

    HttpRequest httpRequest;
    httpRequest.url = chatCompletionsUrl(baseUrl);
    httpRequest.headers = {
        {"Authorization", "Bearer " + apiKey},
        {"Content-Type", "application/json"}
    };
    httpRequest.body = requestBody(model, request, response);
    httpRequest.timeoutMs = timeoutMs;

    const HttpResponse httpResponse = httpClient_->postJson(httpRequest);
    if (!httpResponse.success) {
        std::string warning = "LLM provider request failed.";
        if (!httpResponse.errorMessage.empty()) {
            warning += " " + httpResponse.errorMessage;
        }
        return unavailableResponse(std::move(response), warning);
    }

    const std::string markdown =
        findJsonStringValue(httpResponse.body, "content");
    if (markdown.empty()) {
        return unavailableResponse(
            std::move(response),
            "LLM provider response did not contain assistant Markdown content.");
    }

    response.success = true;
    response.analysisMarkdown = markdown;
    return response;
}

} // namespace gts
