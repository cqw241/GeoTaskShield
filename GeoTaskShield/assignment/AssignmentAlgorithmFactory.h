#pragma once

#include "assignment/IAssignmentAlgorithm.h"

#include <memory>
#include <string>

namespace gts {

class AssignmentAlgorithmFactory {
public:
    static std::unique_ptr<IAssignmentAlgorithm> create(const std::string& type);
};

} // namespace gts
