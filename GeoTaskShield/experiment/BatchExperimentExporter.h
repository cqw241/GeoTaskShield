#pragma once

#include "experiment/BatchExperiment.h"

#include <string>

namespace gts {

class BatchExperimentExporter {
public:
    static std::string toCsv(const BatchExperimentResult& result);
    static std::string toMarkdown(const BatchExperimentResult& result);
    static bool writeCsvToFile(const std::string& filePath,
                               const BatchExperimentResult& result);
    static bool writeMarkdownToFile(const std::string& filePath,
                                    const BatchExperimentResult& result);
};

} // namespace gts
