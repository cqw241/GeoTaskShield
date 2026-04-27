#include "assignment/AssignmentAlgorithmFactory.h"

#include "assignment/HungarianAlgorithm.h"
#include "assignment/NearestGreedyAlgorithm.h"
#include "assignment/ScoreGreedyAlgorithm.h"

#include <algorithm>

namespace gts {

namespace {

std::string normalized(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

} // namespace

std::unique_ptr<IAssignmentAlgorithm> AssignmentAlgorithmFactory::create(const std::string& type)
{
    const std::string key = normalized(type);
    if (key == "nearest" || key == "nearest-greedy") {
        return std::make_unique<NearestGreedyAlgorithm>();
    }
    if (key == "score" || key == "score-greedy") {
        return std::make_unique<ScoreGreedyAlgorithm>();
    }
    if (key == "hungarian") {
        return std::make_unique<HungarianAlgorithm>();
    }
    return nullptr;
}

} // namespace gts
