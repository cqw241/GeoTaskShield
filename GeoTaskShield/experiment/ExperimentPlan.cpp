#include "experiment/ExperimentPlan.h"

#include "assignment/AssignmentAlgorithmFactory.h"
#include "experiment/BatchExperimentExporter.h"
#include "privacy/PrivacyFactory.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gts {

namespace {

struct JsonValue {
    enum class Type {
        Object,
        Array,
        String,
        Number
    };

    Type type{Type::Object};
    std::map<std::string, JsonValue> objectValue;
    std::vector<JsonValue> arrayValue;
    std::string stringValue;
    double numberValue{};
};

class JsonParser {
public:
    explicit JsonParser(std::string text)
        : text_(std::move(text))
    {
    }

    JsonValue parse()
    {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();
        if (position_ != text_.size()) {
            throw std::runtime_error("Unexpected trailing JSON content.");
        }
        return value;
    }

private:
    JsonValue parseValue()
    {
        skipWhitespace();
        if (position_ >= text_.size()) {
            throw std::runtime_error("Unexpected end of JSON.");
        }

        const char ch = text_[position_];
        if (ch == '{') {
            return parseObject();
        }
        if (ch == '[') {
            return parseArray();
        }
        if (ch == '"') {
            JsonValue value;
            value.type = JsonValue::Type::String;
            value.stringValue = parseString();
            return value;
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)) != 0) {
            JsonValue value;
            value.type = JsonValue::Type::Number;
            value.numberValue = parseNumber();
            return value;
        }

        throw std::runtime_error("Unsupported JSON value.");
    }

    JsonValue parseObject()
    {
        consume('{');
        JsonValue value;
        value.type = JsonValue::Type::Object;
        skipWhitespace();
        if (tryConsume('}')) {
            return value;
        }

        while (true) {
            skipWhitespace();
            if (peek() != '"') {
                throw std::runtime_error("Expected JSON object key string.");
            }
            const std::string key = parseString();
            skipWhitespace();
            consume(':');
            value.objectValue.emplace(key, parseValue());
            skipWhitespace();
            if (tryConsume('}')) {
                break;
            }
            consume(',');
        }
        return value;
    }

    JsonValue parseArray()
    {
        consume('[');
        JsonValue value;
        value.type = JsonValue::Type::Array;
        skipWhitespace();
        if (tryConsume(']')) {
            return value;
        }

        while (true) {
            value.arrayValue.push_back(parseValue());
            skipWhitespace();
            if (tryConsume(']')) {
                break;
            }
            consume(',');
        }
        return value;
    }

    std::string parseString()
    {
        consume('"');
        std::string value;
        while (position_ < text_.size()) {
            const char ch = text_[position_++];
            if (ch == '"') {
                return value;
            }
            if (ch == '\\') {
                if (position_ >= text_.size()) {
                    throw std::runtime_error("Invalid JSON string escape.");
                }
                const char escaped = text_[position_++];
                if (escaped == '"' || escaped == '\\' || escaped == '/') {
                    value += escaped;
                } else if (escaped == 'n') {
                    value += '\n';
                } else if (escaped == 'r') {
                    value += '\r';
                } else if (escaped == 't') {
                    value += '\t';
                } else {
                    throw std::runtime_error("Unsupported JSON string escape.");
                }
            } else {
                value += ch;
            }
        }
        throw std::runtime_error("Unterminated JSON string.");
    }

    double parseNumber()
    {
        const std::size_t start = position_;
        if (peek() == '-') {
            ++position_;
        }
        readDigits();
        if (peek() == '.') {
            ++position_;
            readDigits();
        }
        if (peek() == 'e' || peek() == 'E') {
            ++position_;
            if (peek() == '+' || peek() == '-') {
                ++position_;
            }
            readDigits();
        }

        return std::stod(text_.substr(start, position_ - start));
    }

    void readDigits()
    {
        const std::size_t start = position_;
        while (position_ < text_.size() &&
               std::isdigit(static_cast<unsigned char>(text_[position_])) != 0) {
            ++position_;
        }
        if (start == position_) {
            throw std::runtime_error("Expected digit in JSON number.");
        }
    }

    void skipWhitespace()
    {
        while (position_ < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[position_])) != 0) {
            ++position_;
        }
    }

    char peek() const
    {
        if (position_ >= text_.size()) {
            return '\0';
        }
        return text_[position_];
    }

    bool tryConsume(char expected)
    {
        if (peek() == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void consume(char expected)
    {
        if (!tryConsume(expected)) {
            std::ostringstream message;
            message << "Expected '" << expected << "' in JSON.";
            throw std::runtime_error(message.str());
        }
    }

    std::string text_;
    std::size_t position_{};
};

std::string escapeJson(const std::string& value)
{
    std::string escaped;
    for (const char ch : value) {
        if (ch == '"' || ch == '\\') {
            escaped += '\\';
            escaped += ch;
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else if (ch == '\t') {
            escaped += "\\t";
        } else {
            escaped += ch;
        }
    }
    return escaped;
}

std::string compactDouble(double value)
{
    std::ostringstream output;
    output << std::setprecision(12) << value;
    return output.str();
}

std::string invalidValueMessage(const std::string& fieldName, const std::string& value)
{
    return "Field '" + fieldName + "' contains invalid value: " + value;
}

template <typename T>
std::string numberArrayJson(const std::vector<T>& values)
{
    std::ostringstream output;
    output << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << values[i];
    }
    output << ']';
    return output.str();
}

std::string doubleArrayJson(const std::vector<double>& values)
{
    std::ostringstream output;
    output << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << compactDouble(values[i]);
    }
    output << ']';
    return output.str();
}

std::string stringArrayJson(const std::vector<std::string>& values)
{
    std::ostringstream output;
    output << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << '"' << escapeJson(values[i]) << '"';
    }
    output << ']';
    return output.str();
}

std::string serializePlan(const ExperimentPlan& plan)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"name\": \"" << escapeJson(plan.name) << "\",\n"
           << "  \"run_label\": \"" << escapeJson(plan.runLabel) << "\",\n"
           << "  \"workers\": " << numberArrayJson(plan.workers) << ",\n"
           << "  \"tasks\": " << numberArrayJson(plan.tasks) << ",\n"
           << "  \"seeds\": " << numberArrayJson(plan.seeds) << ",\n"
           << "  \"privacy\": " << stringArrayJson(plan.privacyMechanisms) << ",\n"
           << "  \"algorithms\": " << stringArrayJson(plan.assignmentAlgorithms) << ",\n"
           << "  \"grid_size\": " << doubleArrayJson(plan.gridSizes) << ",\n"
           << "  \"k\": " << numberArrayJson(plan.kValues) << ",\n"
           << "  \"epsilon\": " << doubleArrayJson(plan.epsilons);
    if (plan.hasAreaWidth) {
        output << ",\n  \"areaWidth\": " << compactDouble(plan.areaWidth);
    }
    if (plan.hasAreaHeight) {
        output << ",\n  \"areaHeight\": " << compactDouble(plan.areaHeight);
    }
    output << "\n}\n";
    return output.str();
}

const JsonValue* findField(const JsonValue& object, const std::string& name)
{
    const auto it = object.objectValue.find(name);
    if (it == object.objectValue.end()) {
        return nullptr;
    }
    return &it->second;
}

const JsonValue& requiredField(const JsonValue& object, const std::string& name)
{
    const JsonValue* field = findField(object, name);
    if (!field) {
        throw std::runtime_error("Missing required field: " + name);
    }
    return *field;
}

const JsonValue& firstRequiredField(const JsonValue& object,
                                    const std::vector<std::string>& names)
{
    for (const std::string& name : names) {
        const JsonValue* field = findField(object, name);
        if (field) {
            return *field;
        }
    }
    return requiredField(object, names.front());
}

std::string requireString(const JsonValue& value, const std::string& fieldName)
{
    if (value.type != JsonValue::Type::String) {
        throw std::runtime_error("Field '" + fieldName + "' must be a string.");
    }
    if (value.stringValue.empty()) {
        throw std::runtime_error("Field '" + fieldName + "' must not be empty.");
    }
    return value.stringValue;
}

double requireNumber(const JsonValue& value, const std::string& fieldName)
{
    if (value.type != JsonValue::Type::Number) {
        throw std::runtime_error("Field '" + fieldName + "' must be a number.");
    }
    return value.numberValue;
}

int toInt(double value, const std::string& fieldName)
{
    const int converted = static_cast<int>(value);
    if (static_cast<double>(converted) != value) {
        throw std::runtime_error("Field '" + fieldName + "' must contain integer values.");
    }
    return converted;
}

std::vector<double> expandNumberRange(const JsonValue& value, const std::string& fieldName)
{
    const JsonValue& startField = requiredField(value, "start");
    const JsonValue& endField = requiredField(value, "end");
    const JsonValue& stepField = requiredField(value, "step");
    const double start = requireNumber(startField, fieldName + ".start");
    const double end = requireNumber(endField, fieldName + ".end");
    const double step = requireNumber(stepField, fieldName + ".step");
    if (step <= 0.0) {
        throw std::runtime_error("Field '" + fieldName + ".step' must be greater than zero.");
    }
    if (end < start) {
        throw std::runtime_error("Field '" + fieldName + ".end' must be greater than or equal to start.");
    }

    std::vector<double> values;
    for (double current = start; current <= end + (step / 1000000.0); current += step) {
        values.push_back(current);
    }
    return values;
}

std::vector<double> readNumberList(const JsonValue& value, const std::string& fieldName)
{
    std::vector<double> values;
    if (value.type == JsonValue::Type::Array) {
        for (const JsonValue& item : value.arrayValue) {
            values.push_back(requireNumber(item, fieldName));
        }
    } else if (value.type == JsonValue::Type::Object) {
        values = expandNumberRange(value, fieldName);
    } else {
        throw std::runtime_error("Field '" + fieldName + "' must be an array or range object.");
    }

    if (values.empty()) {
        throw std::runtime_error("Field '" + fieldName + "' must not be empty.");
    }
    return values;
}

std::vector<int> readIntList(const JsonValue& value, const std::string& fieldName)
{
    const std::vector<double> numbers = readNumberList(value, fieldName);
    std::vector<int> values;
    values.reserve(numbers.size());
    for (const double number : numbers) {
        values.push_back(toInt(number, fieldName));
    }
    return values;
}

std::vector<unsigned int> readSeedList(const JsonValue& value, const std::string& fieldName)
{
    const std::vector<int> numbers = readIntList(value, fieldName);
    std::vector<unsigned int> values;
    values.reserve(numbers.size());
    for (const int number : numbers) {
        if (number < 0) {
            throw std::runtime_error(invalidValueMessage(fieldName, std::to_string(number)));
        }
        values.push_back(static_cast<unsigned int>(number));
    }
    return values;
}

std::vector<std::string> readStringList(const JsonValue& value, const std::string& fieldName)
{
    if (value.type != JsonValue::Type::Array) {
        throw std::runtime_error("Field '" + fieldName + "' must be an array.");
    }
    if (value.arrayValue.empty()) {
        throw std::runtime_error("Field '" + fieldName + "' must not be empty.");
    }

    std::vector<std::string> values;
    values.reserve(value.arrayValue.size());
    for (const JsonValue& item : value.arrayValue) {
        values.push_back(requireString(item, fieldName));
    }
    return values;
}

void validateStrategies(const ExperimentPlan& plan)
{
    for (const std::string& privacy : plan.privacyMechanisms) {
        if (!PrivacyFactory::create(privacy)) {
            throw std::runtime_error("Unknown privacy mechanism: " + privacy);
        }
    }

    for (const std::string& algorithm : plan.assignmentAlgorithms) {
        if (!AssignmentAlgorithmFactory::create(algorithm)) {
            throw std::runtime_error("Unknown assignment algorithm: " + algorithm);
        }
    }
}

void validatePositiveInts(const std::vector<int>& values, const std::string& fieldName)
{
    for (const int value : values) {
        if (value <= 0) {
            throw std::runtime_error(invalidValueMessage(fieldName, std::to_string(value)));
        }
    }
}

void validatePositiveDoubles(const std::vector<double>& values, const std::string& fieldName)
{
    for (const double value : values) {
        if (value <= 0.0) {
            throw std::runtime_error(invalidValueMessage(fieldName, compactDouble(value)));
        }
    }
}

void validatePlanParameters(const ExperimentPlan& plan)
{
    validatePositiveInts(plan.workers, "workers");
    validatePositiveInts(plan.tasks, "tasks");
    validatePositiveInts(plan.kValues, "k");
    validatePositiveDoubles(plan.gridSizes, "grid_size");
    validatePositiveDoubles(plan.epsilons, "epsilon");

    if (plan.hasAreaWidth && plan.areaWidth <= 0.0) {
        throw std::runtime_error(invalidValueMessage("areaWidth", compactDouble(plan.areaWidth)));
    }
    if (plan.hasAreaHeight && plan.areaHeight <= 0.0) {
        throw std::runtime_error(invalidValueMessage("areaHeight", compactDouble(plan.areaHeight)));
    }
}

std::string timestampUtc()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &nowTime);
#else
    gmtime_r(&nowTime, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

std::string metadataJson(const ExperimentPlan& plan,
                         std::size_t scenarioCount,
                         const std::string& projectVersion)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"generated_at\": \"" << timestampUtc() << "\",\n"
           << "  \"plan_name\": \"" << escapeJson(plan.name) << "\",\n"
           << "  \"run_label\": \"" << escapeJson(plan.runLabel) << "\",\n"
           << "  \"scenario_count\": " << scenarioCount << ",\n"
           << "  \"seeds\": " << numberArrayJson(plan.seeds) << ",\n"
           << "  \"privacy\": " << stringArrayJson(plan.privacyMechanisms) << ",\n"
           << "  \"algorithms\": " << stringArrayJson(plan.assignmentAlgorithms) << ",\n"
           << "  \"project_version\": \"" << escapeJson(projectVersion) << "\"\n"
           << "}\n";
    return output.str();
}

bool writeTextFile(const std::filesystem::path& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    file << content;
    return static_cast<bool>(file);
}

} // namespace

std::vector<ExperimentScenario> ExperimentPlan::expandToScenarios() const
{
    std::vector<ExperimentScenario> scenarios;
    scenarios.reserve(workers.size() * tasks.size() * seeds.size() *
                      privacyMechanisms.size() * assignmentAlgorithms.size() *
                      gridSizes.size() * kValues.size() * epsilons.size());

    for (const int workerCount : workers) {
        for (const int taskCount : tasks) {
            for (const unsigned int seed : seeds) {
                for (const std::string& privacy : privacyMechanisms) {
                    for (const std::string& algorithm : assignmentAlgorithms) {
                        for (const double gridSize : gridSizes) {
                            for (const int k : kValues) {
                                for (const double epsilon : epsilons) {
                                    ExperimentScenario scenario;
                                    std::ostringstream nameStream;
                                    nameStream << name
                                               << "-w" << workerCount
                                               << "-t" << taskCount
                                               << "-seed" << seed
                                               << '-' << privacy
                                               << '-' << algorithm
                                               << "-grid" << compactDouble(gridSize)
                                               << "-k" << k
                                               << "-eps" << compactDouble(epsilon);
                                    scenario.name = nameStream.str();
                                    scenario.config.workerCount = workerCount;
                                    scenario.config.taskCount = taskCount;
                                    scenario.config.randomSeed = seed;
                                    scenario.config.privacy.gridSize = gridSize;
                                    scenario.config.privacy.k = k;
                                    scenario.config.privacy.epsilon = epsilon;
                                    if (hasAreaWidth) {
                                        scenario.config.areaWidth = areaWidth;
                                    }
                                    if (hasAreaHeight) {
                                        scenario.config.areaHeight = areaHeight;
                                    }
                                    scenario.privacyType = privacy;
                                    scenario.algorithmType = algorithm;
                                    scenarios.push_back(scenario);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return scenarios;
}

ExperimentPlanLoadResult ExperimentPlanLoader::loadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return ExperimentPlanLoadResult{
            false,
            ExperimentPlan{},
            "Experiment plan file does not exist or cannot be opened: " + filePath
        };
    }

    const std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    return loadFromString(content);
}

ExperimentPlanLoadResult ExperimentPlanLoader::loadFromString(const std::string& json)
{
    try {
        JsonParser parser(json);
        const JsonValue root = parser.parse();
        if (root.type != JsonValue::Type::Object) {
            throw std::runtime_error("Experiment plan root must be a JSON object.");
        }

        ExperimentPlan plan;
        plan.name = requireString(requiredField(root, "name"), "name");
        if (const JsonValue* runLabelSnake = findField(root, "run_label")) {
            plan.runLabel = requireString(*runLabelSnake, "run_label");
        } else if (const JsonValue* runLabelCamel = findField(root, "runLabel")) {
            plan.runLabel = requireString(*runLabelCamel, "runLabel");
        } else {
            plan.runLabel = plan.name;
        }
        plan.workers = readIntList(requiredField(root, "workers"), "workers");
        plan.tasks = readIntList(requiredField(root, "tasks"), "tasks");
        plan.seeds = readSeedList(requiredField(root, "seeds"), "seeds");
        plan.privacyMechanisms = readStringList(
            firstRequiredField(root, {"privacy", "privacy_mechanisms"}),
            "privacy");
        plan.assignmentAlgorithms = readStringList(
            firstRequiredField(root, {"algorithms", "assignment_algorithms"}),
            "algorithms");
        plan.gridSizes = readNumberList(
            firstRequiredField(root, {"grid_size", "grid_sizes"}),
            "grid_size");
        plan.kValues = readIntList(requiredField(root, "k"), "k");
        plan.epsilons = readNumberList(
            firstRequiredField(root, {"epsilon", "epsilons"}),
            "epsilon");

        if (const JsonValue* areaWidth = findField(root, "areaWidth")) {
            plan.hasAreaWidth = true;
            plan.areaWidth = requireNumber(*areaWidth, "areaWidth");
        }
        if (const JsonValue* areaHeight = findField(root, "areaHeight")) {
            plan.hasAreaHeight = true;
            plan.areaHeight = requireNumber(*areaHeight, "areaHeight");
        }

        validatePlanParameters(plan);
        validateStrategies(plan);
        plan.sourceJson = json;
        return ExperimentPlanLoadResult{true, plan, {}};
    } catch (const std::exception& ex) {
        return ExperimentPlanLoadResult{false, ExperimentPlan{}, ex.what()};
    }
}

ExperimentPlanRunResult ExperimentPlanRunner::runToDirectory(
    const ExperimentPlan& plan,
    const std::string& outputDirectory,
    const std::string& projectVersion)
{
    try {
        const std::vector<ExperimentScenario> scenarios = plan.expandToScenarios();
        if (scenarios.empty()) {
            return ExperimentPlanRunResult{
                false,
                "Experiment plan produced no scenarios.",
                outputDirectory,
                0
            };
        }

        const std::filesystem::path runDirectory(outputDirectory);
        std::filesystem::create_directories(runDirectory);

        const BatchExperimentRunner runner;
        const BatchExperimentResult result = runner.run(scenarios);
        const std::filesystem::path csvPath = runDirectory / "results.csv";
        const std::filesystem::path reportPath = runDirectory / "report.md";
        const std::filesystem::path snapshotPath = runDirectory / "plan_snapshot.json";
        const std::filesystem::path metadataPath = runDirectory / "metadata.json";

        if (!BatchExperimentExporter::writeCsvToFile(csvPath.string(), result)) {
            return ExperimentPlanRunResult{false, "Could not write results.csv.", outputDirectory, scenarios.size()};
        }
        if (!BatchExperimentExporter::writeMarkdownToFile(reportPath.string(), result)) {
            return ExperimentPlanRunResult{false, "Could not write report.md.", outputDirectory, scenarios.size()};
        }
        const std::string snapshot = plan.sourceJson.empty() ? serializePlan(plan) : plan.sourceJson;
        if (!writeTextFile(snapshotPath, snapshot)) {
            return ExperimentPlanRunResult{false, "Could not write plan_snapshot.json.", outputDirectory, scenarios.size()};
        }
        if (!writeTextFile(metadataPath, metadataJson(plan, scenarios.size(), projectVersion))) {
            return ExperimentPlanRunResult{false, "Could not write metadata.json.", outputDirectory, scenarios.size()};
        }

        return ExperimentPlanRunResult{true, {}, outputDirectory, scenarios.size()};
    } catch (const std::exception& ex) {
        return ExperimentPlanRunResult{false, ex.what(), outputDirectory, 0};
    }
}

std::string ExperimentPlanRunner::defaultOutputDirectory(const ExperimentPlan& plan)
{
    const std::string label = !plan.runLabel.empty() ? plan.runLabel : plan.name;
    return (std::filesystem::path("runs") / sanitizeRunLabel(label)).string();
}

std::string ExperimentPlanRunner::sanitizeRunLabel(const std::string& value)
{
    std::string sanitized;
    for (const unsigned char ch : value) {
        if (std::isalnum(ch) != 0 || ch == '-' || ch == '_') {
            sanitized += static_cast<char>(std::tolower(ch));
        } else if (std::isspace(ch) != 0 || ch == '.') {
            sanitized += '-';
        }
    }

    sanitized.erase(std::unique(sanitized.begin(), sanitized.end(), [](char lhs, char rhs) {
        return lhs == '-' && rhs == '-';
    }), sanitized.end());

    while (!sanitized.empty() && sanitized.front() == '-') {
        sanitized.erase(sanitized.begin());
    }
    while (!sanitized.empty() && sanitized.back() == '-') {
        sanitized.pop_back();
    }
    if (sanitized.empty()) {
        return "experiment-run";
    }
    return sanitized;
}

} // namespace gts
