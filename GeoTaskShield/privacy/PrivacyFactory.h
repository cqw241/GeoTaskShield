#pragma once

#include "privacy/IPrivacyMechanism.h"

#include <memory>
#include <string>

namespace gts {

class PrivacyFactory {
public:
    static std::unique_ptr<IPrivacyMechanism> create(const std::string& type);
};

} // namespace gts
