#include "privacy/PrivacyFactory.h"

#include "privacy/GridPrivacy.h"
#include "privacy/KAnonymityPrivacy.h"
#include "privacy/LaplaceNoisePrivacy.h"

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

std::unique_ptr<IPrivacyMechanism> PrivacyFactory::create(const std::string& type)
{
    const std::string key = normalized(type);
    if (key == "grid") {
        return std::make_unique<GridPrivacy>();
    }
    if (key == "k-anonymity" || key == "kanonymity" || key == "k") {
        return std::make_unique<KAnonymityPrivacy>();
    }
    if (key == "laplace" || key == "laplace-noise") {
        return std::make_unique<LaplaceNoisePrivacy>();
    }
    return nullptr;
}

} // namespace gts
