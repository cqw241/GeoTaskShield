#pragma once

#include "agent/HttpClient.h"

namespace gts {

class WinHttpClient : public IHttpClient {
public:
    [[nodiscard]] HttpResponse postJson(
        const HttpRequest& request) const override;
};

} // namespace gts
