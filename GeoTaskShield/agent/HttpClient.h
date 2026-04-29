#pragma once

#include <string>
#include <utility>
#include <vector>

namespace gts {

struct HttpRequest {
    std::string url;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
    int timeoutMs{15000};
};

struct HttpResponse {
    bool success{};
    int statusCode{};
    std::string body;
    std::string errorMessage;
};

class IHttpClient {
public:
    virtual ~IHttpClient() = default;

    [[nodiscard]] virtual HttpResponse postJson(
        const HttpRequest& request) const = 0;
};

} // namespace gts
