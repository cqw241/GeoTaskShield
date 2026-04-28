#include "experiment/BatchResultModel.h"

#include <algorithm>
#include <sstream>

namespace gts {

namespace {

double metricValue(const BatchResultRecord& record, BatchResultMetric metric)
{
    switch (metric) {
    case BatchResultMetric::CompletionRate:
        return record.completionRate;
    case BatchResultMetric::AverageTrueDistance:
        return record.averageTrueDistance;
    case BatchResultMetric::TotalReward:
        return record.totalReward;
    case BatchResultMetric::AveragePrivacyLoss:
        return record.averagePrivacyLoss;
    case BatchResultMetric::RuntimeMs:
        return record.runtimeMs;
    case BatchResultMetric::UserLoadStdDev:
        return record.userLoadStdDev;
    case BatchResultMetric::FairnessIndex:
        return record.fairnessIndex;
    case BatchResultMetric::PrivacyUtilityRatio:
        return record.privacyUtilityRatio;
    case BatchResultMetric::TimeoutRate:
        return record.timeoutRate;
    }
    return 0.0;
}

double numericSortValue(const BatchResultRecord& record, BatchResultSortField field)
{
    switch (field) {
    case BatchResultSortField::WorkerCount:
        return static_cast<double>(record.workerCount);
    case BatchResultSortField::TaskCount:
        return static_cast<double>(record.taskCount);
    case BatchResultSortField::CompletionRate:
        return record.completionRate;
    case BatchResultSortField::AverageTrueDistance:
        return record.averageTrueDistance;
    case BatchResultSortField::TotalReward:
        return record.totalReward;
    case BatchResultSortField::AveragePrivacyLoss:
        return record.averagePrivacyLoss;
    case BatchResultSortField::RuntimeMs:
        return record.runtimeMs;
    case BatchResultSortField::UserLoadStdDev:
        return record.userLoadStdDev;
    case BatchResultSortField::FairnessIndex:
        return record.fairnessIndex;
    case BatchResultSortField::PrivacyUtilityRatio:
        return record.privacyUtilityRatio;
    case BatchResultSortField::TimeoutRate:
        return record.timeoutRate;
    case BatchResultSortField::Scenario:
    case BatchResultSortField::Privacy:
    case BatchResultSortField::Algorithm:
        break;
    }
    return 0.0;
}

std::string textSortValue(const BatchResultRecord& record, BatchResultSortField field)
{
    switch (field) {
    case BatchResultSortField::Scenario:
        return record.scenario;
    case BatchResultSortField::Privacy:
        return record.privacy;
    case BatchResultSortField::Algorithm:
        return record.algorithm;
    default:
        return {};
    }
}

bool isTextField(BatchResultSortField field)
{
    return field == BatchResultSortField::Scenario ||
           field == BatchResultSortField::Privacy ||
           field == BatchResultSortField::Algorithm;
}

std::optional<BatchResultSummary> bestBy(const std::vector<BatchResultRecord>& records,
                                         BatchResultMetric metric,
                                         bool lowest)
{
    if (records.empty()) {
        return std::nullopt;
    }

    const auto bestIt = std::minmax_element(
        records.begin(), records.end(), [&](const BatchResultRecord& lhs,
                                            const BatchResultRecord& rhs) {
            return metricValue(lhs, metric) < metricValue(rhs, metric);
        });
    const BatchResultRecord& record = lowest ? *bestIt.first : *bestIt.second;
    return BatchResultSummary{record, metricValue(record, metric)};
}

std::string chartLabel(const BatchResultRecord& record)
{
    std::ostringstream label;
    label << record.scenario << " | " << record.privacy << " | " << record.algorithm;
    return label.str();
}

} // namespace

void BatchResultModel::setRecords(std::vector<BatchResultRecord> records)
{
    records_ = std::move(records);
}

const std::vector<BatchResultRecord>& BatchResultModel::records() const
{
    return records_;
}

void BatchResultModel::setPrivacyFilter(const std::string& privacy)
{
    privacyFilter_ = privacy;
}

void BatchResultModel::setAlgorithmFilter(const std::string& algorithm)
{
    algorithmFilter_ = algorithm;
}

void BatchResultModel::clearFilters()
{
    privacyFilter_.clear();
    algorithmFilter_.clear();
}

std::vector<BatchResultRecord> BatchResultModel::filteredRecords() const
{
    std::vector<BatchResultRecord> filtered;
    for (const BatchResultRecord& record : records_) {
        if (!privacyFilter_.empty() && record.privacy != privacyFilter_) {
            continue;
        }
        if (!algorithmFilter_.empty() && record.algorithm != algorithmFilter_) {
            continue;
        }
        filtered.push_back(record);
    }
    return filtered;
}

std::vector<BatchResultRecord> BatchResultModel::sortedRecords(BatchResultSortField field,
                                                               bool ascending) const
{
    std::vector<BatchResultRecord> sorted = filteredRecords();
    std::sort(sorted.begin(), sorted.end(), [&](const BatchResultRecord& lhs,
                                                const BatchResultRecord& rhs) {
        if (isTextField(field)) {
            if (ascending) {
                return textSortValue(lhs, field) < textSortValue(rhs, field);
            }
            return textSortValue(rhs, field) < textSortValue(lhs, field);
        }

        if (ascending) {
            return numericSortValue(lhs, field) < numericSortValue(rhs, field);
        }
        return numericSortValue(rhs, field) < numericSortValue(lhs, field);
    });
    return sorted;
}

std::optional<BatchResultSummary> BatchResultModel::bestCompletionRate() const
{
    return bestBy(filteredRecords(), BatchResultMetric::CompletionRate, false);
}

std::optional<BatchResultSummary> BatchResultModel::bestPrivacyUtilityRatio() const
{
    return bestBy(filteredRecords(), BatchResultMetric::PrivacyUtilityRatio, false);
}

std::optional<BatchResultSummary> BatchResultModel::bestFairnessIndex() const
{
    return bestBy(filteredRecords(), BatchResultMetric::FairnessIndex, false);
}

std::optional<BatchResultSummary> BatchResultModel::lowestAveragePrivacyLoss() const
{
    return bestBy(filteredRecords(), BatchResultMetric::AveragePrivacyLoss, true);
}

std::vector<ChartBar> BatchResultModel::chartBars(BatchResultMetric metric) const
{
    std::vector<ChartBar> bars;
    const std::vector<BatchResultRecord> records = filteredRecords();
    bars.reserve(records.size());
    for (const BatchResultRecord& record : records) {
        bars.push_back(ChartBar{chartLabel(record), metricValue(record, metric)});
    }
    return bars;
}

} // namespace gts
