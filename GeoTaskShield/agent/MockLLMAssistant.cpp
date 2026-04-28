#include "agent/MockLLMAssistant.h"

#include "agent/RuleBasedAssistant.h"

#include <sstream>

namespace gts {

AssistantResponse MockLLMAssistant::analyze(const AssistantRequest& request) const
{
    RuleBasedAssistant localAssistant;
    AssistantResponse response = localAssistant.analyze(request);

    std::ostringstream markdown;
    markdown << "# Mock LLM Assistant\n\n";
    markdown << "This is a deterministic local mock response. It does not call "
                "a network API and does not read API keys.\n\n";
    markdown << response.analysisMarkdown;
    response.analysisMarkdown = markdown.str();
    return response;
}

} // namespace gts
