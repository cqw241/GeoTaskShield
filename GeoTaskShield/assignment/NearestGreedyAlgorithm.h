#pragma once

#include "assignment/IAssignmentAlgorithm.h"

namespace gts {

class NearestGreedyAlgorithm final : public IAssignmentAlgorithm {
public:
    std::string name() const override;
    AssignmentResult assign(const std::vector<Task>& tasks,
                            const std::vector<Worker>& workers,
                            const AssignmentContext& context) const override;
};

} // namespace gts
