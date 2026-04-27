#pragma once

#include "evaluation/EvaluationMetrics.h"

#include <string>
#include <vector>

namespace gts {

struct ExperimentSummaryRow {
    std::string privacyName;
    std::string algorithmName;
    EvaluationMetrics metrics;
};

class CsvExporter {
public:
    static std::string toCsv(const std::vector<ExperimentSummaryRow>& rows);
    static bool writeToFile(const std::string& filePath,
                            const std::vector<ExperimentSummaryRow>& rows);
};

} // namespace gts
