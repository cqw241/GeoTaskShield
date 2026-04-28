#include "experiment/BatchResultCsvLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace gts {

namespace {

std::string trim(const std::string& value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
                         return std::isspace(ch) != 0;
                     }).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

std::string normalizedHeader(const std::string& value)
{
    std::string result = trim(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return result;
}

void stripUtf8Bom(std::string& content)
{
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content.erase(0, 3);
    }
}

std::vector<std::vector<std::string>> parseCsvRows(const std::string& content)
{
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool inQuotes = false;

    for (std::size_t i = 0; i < content.size(); ++i) {
        const char ch = content[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field.push_back(ch);
            }
            continue;
        }

        if (ch == '"') {
            inQuotes = true;
        } else if (ch == ',') {
            row.push_back(field);
            field.clear();
        } else if (ch == '\r' || ch == '\n') {
            row.push_back(field);
            field.clear();
            rows.push_back(row);
            row.clear();
            if (ch == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                ++i;
            }
        } else {
            field.push_back(ch);
        }
    }

    if (!field.empty() || !row.empty()) {
        row.push_back(field);
        rows.push_back(row);
    }

    return rows;
}

std::unordered_map<std::string, std::size_t> headerIndex(
    const std::vector<std::string>& headers)
{
    std::unordered_map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        index.emplace(normalizedHeader(headers[i]), i);
    }
    return index;
}

bool findColumn(const std::unordered_map<std::string, std::size_t>& headers,
                std::initializer_list<const char*> aliases,
                std::size_t& index)
{
    for (const char* alias : aliases) {
        const auto it = headers.find(normalizedHeader(alias));
        if (it != headers.end()) {
            index = it->second;
            return true;
        }
    }
    return false;
}

std::string valueAt(const std::vector<std::string>& row, std::size_t index)
{
    if (index >= row.size()) {
        return {};
    }
    return row[index];
}

bool parseInt(const std::string& raw, int& value)
{
    const std::string text = trim(raw);
    try {
        std::size_t parsed = 0;
        const int parsedValue = std::stoi(text, &parsed);
        if (parsed != text.size()) {
            return false;
        }
        value = parsedValue;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parseDouble(const std::string& raw, double& value)
{
    const std::string text = trim(raw);
    try {
        std::size_t parsed = 0;
        const double parsedValue = std::stod(text, &parsed);
        if (parsed != text.size()) {
            return false;
        }
        value = parsedValue;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string invalidNumericMessage(std::size_t rowNumber,
                                  const std::string& columnName,
                                  const std::string& raw)
{
    std::ostringstream message;
    message << "Invalid numeric cell at row " << rowNumber
            << ", column " << columnName << ": " << raw;
    return message.str();
}

struct ColumnMap {
    std::size_t scenario{};
    std::size_t workers{};
    std::size_t tasks{};
    std::size_t gridSize{};
    std::size_t k{};
    std::size_t epsilon{};
    std::size_t privacy{};
    std::size_t algorithm{};
    std::size_t completedTasks{};
    std::size_t totalTasks{};
    std::size_t completionRate{};
    std::size_t averageTrueDistance{};
    std::size_t totalReward{};
    std::size_t averagePrivacyLoss{};
    std::size_t runtimeMs{};
    std::size_t userLoadStdDev{};
    std::size_t fairnessIndex{};
    std::size_t privacyUtilityRatio{};
    std::size_t timeoutRate{};
};

bool requireColumn(const std::unordered_map<std::string, std::size_t>& headers,
                   std::initializer_list<const char*> aliases,
                   const std::string& displayName,
                   std::size_t& index,
                   BatchResultLoadResult& result)
{
    if (findColumn(headers, aliases, index)) {
        return true;
    }
    result.errorMessage = "Missing required column: " + displayName;
    return false;
}

bool buildColumnMap(const std::vector<std::string>& headers,
                    ColumnMap& columns,
                    BatchResultLoadResult& result)
{
    const auto index = headerIndex(headers);
    return requireColumn(index, {"scenario"}, "scenario", columns.scenario, result) &&
           requireColumn(index, {"workers"}, "workers", columns.workers, result) &&
           requireColumn(index, {"tasks"}, "tasks", columns.tasks, result) &&
           requireColumn(index, {"grid_size"}, "grid_size", columns.gridSize, result) &&
           requireColumn(index, {"k"}, "k", columns.k, result) &&
           requireColumn(index, {"epsilon"}, "epsilon", columns.epsilon, result) &&
           requireColumn(index, {"privacy"}, "privacy", columns.privacy, result) &&
           requireColumn(index, {"algorithm"}, "algorithm", columns.algorithm, result) &&
           requireColumn(index, {"completed_tasks"}, "completed_tasks",
                         columns.completedTasks, result) &&
           requireColumn(index, {"total_tasks"}, "total_tasks", columns.totalTasks, result) &&
           requireColumn(index, {"completion_rate"}, "completion_rate",
                         columns.completionRate, result) &&
           requireColumn(index,
                         {"average_moving_distance", "average_true_distance",
                          "averageTrueDistance"},
                         "average_moving_distance", columns.averageTrueDistance, result) &&
           requireColumn(index, {"total_reward"}, "total_reward", columns.totalReward, result) &&
           requireColumn(index, {"average_privacy_loss"}, "average_privacy_loss",
                         columns.averagePrivacyLoss, result) &&
           requireColumn(index, {"algorithm_runtime_ms", "runtime_ms", "runtimeMs"},
                         "algorithm_runtime_ms", columns.runtimeMs, result) &&
           requireColumn(index, {"user_load_stddev"}, "user_load_stddev",
                         columns.userLoadStdDev, result) &&
           requireColumn(index, {"fairness_index"}, "fairness_index",
                         columns.fairnessIndex, result) &&
           requireColumn(index, {"privacy_utility_ratio", "privacyUtilityRatio"},
                         "privacy_utility_ratio", columns.privacyUtilityRatio, result) &&
           requireColumn(index, {"timeout_rate"}, "timeout_rate", columns.timeoutRate, result);
}

bool parseIntCell(const std::vector<std::string>& row,
                  std::size_t rowNumber,
                  std::size_t column,
                  const std::string& columnName,
                  int& value,
                  BatchResultLoadResult& result)
{
    const std::string raw = valueAt(row, column);
    if (parseInt(raw, value)) {
        return true;
    }
    result.errorMessage = invalidNumericMessage(rowNumber, columnName, raw);
    return false;
}

bool parseDoubleCell(const std::vector<std::string>& row,
                     std::size_t rowNumber,
                     std::size_t column,
                     const std::string& columnName,
                     double& value,
                     BatchResultLoadResult& result)
{
    const std::string raw = valueAt(row, column);
    if (parseDouble(raw, value)) {
        return true;
    }
    result.errorMessage = invalidNumericMessage(rowNumber, columnName, raw);
    return false;
}

bool parseRecord(const std::vector<std::string>& row,
                 std::size_t rowNumber,
                 const ColumnMap& columns,
                 BatchResultRecord& record,
                 BatchResultLoadResult& result)
{
    record.scenario = valueAt(row, columns.scenario);
    record.privacy = valueAt(row, columns.privacy);
    record.algorithm = valueAt(row, columns.algorithm);

    return parseIntCell(row, rowNumber, columns.workers, "workers", record.workerCount, result) &&
           parseIntCell(row, rowNumber, columns.tasks, "tasks", record.taskCount, result) &&
           parseDoubleCell(row, rowNumber, columns.gridSize, "grid_size", record.gridSize, result) &&
           parseIntCell(row, rowNumber, columns.k, "k", record.k, result) &&
           parseDoubleCell(row, rowNumber, columns.epsilon, "epsilon", record.epsilon, result) &&
           parseIntCell(row, rowNumber, columns.completedTasks, "completed_tasks",
                        record.completedTasks, result) &&
           parseIntCell(row, rowNumber, columns.totalTasks, "total_tasks",
                        record.totalTasks, result) &&
           parseDoubleCell(row, rowNumber, columns.completionRate, "completion_rate",
                           record.completionRate, result) &&
           parseDoubleCell(row, rowNumber, columns.averageTrueDistance,
                           "average_moving_distance", record.averageTrueDistance, result) &&
           parseDoubleCell(row, rowNumber, columns.totalReward, "total_reward",
                           record.totalReward, result) &&
           parseDoubleCell(row, rowNumber, columns.averagePrivacyLoss,
                           "average_privacy_loss", record.averagePrivacyLoss, result) &&
           parseDoubleCell(row, rowNumber, columns.runtimeMs, "algorithm_runtime_ms",
                           record.runtimeMs, result) &&
           parseDoubleCell(row, rowNumber, columns.userLoadStdDev,
                           "user_load_stddev", record.userLoadStdDev, result) &&
           parseDoubleCell(row, rowNumber, columns.fairnessIndex,
                           "fairness_index", record.fairnessIndex, result) &&
           parseDoubleCell(row, rowNumber, columns.privacyUtilityRatio,
                           "privacy_utility_ratio", record.privacyUtilityRatio, result) &&
           parseDoubleCell(row, rowNumber, columns.timeoutRate, "timeout_rate",
                           record.timeoutRate, result);
}

} // namespace

BatchResultLoadResult BatchResultCsvLoader::loadFromFile(const std::string& filePath)
{
    BatchResultLoadResult result;

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        result.errorMessage = "Failed to open CSV file: " + filePath;
        return result;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    stripUtf8Bom(content);
    if (content.empty()) {
        result.success = true;
        return result;
    }

    const std::vector<std::vector<std::string>> rows = parseCsvRows(content);
    if (rows.empty()) {
        result.success = true;
        return result;
    }

    ColumnMap columns;
    if (!buildColumnMap(rows.front(), columns, result)) {
        return result;
    }

    for (std::size_t i = 1; i < rows.size(); ++i) {
        const std::vector<std::string>& row = rows[i];
        const bool emptyRow = std::all_of(row.begin(), row.end(), [](const std::string& value) {
            return trim(value).empty();
        });
        if (emptyRow) {
            continue;
        }

        BatchResultRecord record;
        if (!parseRecord(row, i + 1, columns, record, result)) {
            result.records.clear();
            return result;
        }
        result.records.push_back(record);
    }

    result.success = true;
    return result;
}

} // namespace gts
