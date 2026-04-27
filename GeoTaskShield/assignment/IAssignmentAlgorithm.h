#pragma once

#include "model/Assignment.h"
#include "model/ExperimentConfig.h"

#include <string>
#include <vector>

namespace gts {

struct AssignmentResult {
    std::vector<Assignment> assignments;
    double algorithmRuntimeMs{};
};

class IAssignmentAlgorithm {
public:
    virtual ~IAssignmentAlgorithm() = default;

    virtual std::string name() const = 0;
    virtual AssignmentResult assign(const std::vector<Task>& tasks,
                                    const std::vector<Worker>& workers,
                                    const AssignmentContext& context) const = 0;
};

} // namespace gts
