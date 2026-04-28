#pragma once

#include "experiment/BatchResultRecord.h"

#include <optional>
#include <string>
#include <vector>

namespace gts {

class BatchResultModel {
public:
    void setRecords(std::vector<BatchResultRecord> records);
    const std::vector<BatchResultRecord>& records() const;

    void setPrivacyFilter(const std::string& privacy);
    void setAlgorithmFilter(const std::string& algorithm);
    void clearFilters();

    std::vector<BatchResultRecord> filteredRecords() const;
    std::vector<BatchResultRecord> sortedRecords(BatchResultSortField field,
                                                 bool ascending) const;

    std::optional<BatchResultSummary> bestCompletionRate() const;
    std::optional<BatchResultSummary> bestPrivacyUtilityRatio() const;
    std::optional<BatchResultSummary> bestFairnessIndex() const;
    std::optional<BatchResultSummary> lowestAveragePrivacyLoss() const;

    std::vector<ChartBar> chartBars(BatchResultMetric metric) const;

private:
    std::vector<BatchResultRecord> records_;
    std::string privacyFilter_;
    std::string algorithmFilter_;
};

} // namespace gts
