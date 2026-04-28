#pragma once

#include "experiment/BatchResultRecord.h"

#include <string>
#include <vector>

namespace gts {

struct BatchResultLoadResult {
    bool success{};
    std::vector<BatchResultRecord> records;
    std::string errorMessage;
};

class BatchResultCsvLoader {
public:
    static BatchResultLoadResult loadFromFile(const std::string& filePath);
};

} // namespace gts
