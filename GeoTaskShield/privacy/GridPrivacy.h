#pragma once

#include "privacy/IPrivacyMechanism.h"

namespace gts {

class GridPrivacy final : public IPrivacyMechanism {
public:
    std::string name() const override;
    PrivacyResult apply(const std::vector<Worker>& workers,
                        const PrivacyContext& context) const override;
};

} // namespace gts
