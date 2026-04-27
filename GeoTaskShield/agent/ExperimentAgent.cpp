#include "agent/ExperimentAgent.h"

#include "agent/ReportGenerator.h"
#include "agent/RuleBasedConfigParser.h"
#include "assignment/AssignmentAlgorithmFactory.h"
#include "privacy/PrivacyFactory.h"
#include "simulation/SimulationEngine.h"

#include <stdexcept>
#include <utility>
#include <vector>

namespace gts {

ExperimentAgentResult ExperimentAgent::run(const std::string& text) const
{
    RuleBasedConfigParser parser;
    ExperimentAgentResult result;
    result.request = parser.parse(text);
    result.report.requestText = text;
    result.report.config = result.request.config;

    std::vector<std::string> privacyTypes;
    if (result.request.comparePrivacyMechanisms) {
        privacyTypes = result.request.privacyTypes;
    } else {
        privacyTypes.push_back(result.request.privacyType);
    }

    for (const std::string& privacyType : privacyTypes) {
        auto privacy = PrivacyFactory::create(privacyType);
        auto algorithm = AssignmentAlgorithmFactory::create(result.request.algorithmType);
        if (!privacy || !algorithm) {
            throw std::runtime_error("ExperimentAgent could not create requested strategies.");
        }

        const std::string privacyName = privacy->name();
        const std::string algorithmName = algorithm->name();
        SimulationEngine engine(std::move(privacy), std::move(algorithm));
        const SimulationRunResult runResult = engine.run(result.request.config);
        result.report.rows.push_back(ExperimentReportRow{
            privacyName,
            algorithmName,
            runResult.metrics
        });
    }

    result.markdown = ReportGenerator::toMarkdown(result.report);
    return result;
}

} // namespace gts
