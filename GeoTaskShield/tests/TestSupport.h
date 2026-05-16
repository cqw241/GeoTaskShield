#pragma once

#include "agent/HttpClient.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace gts_test {

inline void require(bool condition, const std::string& message)
{
    if (!condition) {
        std::cerr << "Test failed: " << message << '\n';
        std::exit(1);
    }
}

inline bool near(double lhs, double rhs, double tolerance = 1e-9)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

inline bool contains(const std::string& value, const std::string& expected)
{
    return value.find(expected) != std::string::npos;
}

class FakeHttpClient : public gts::IHttpClient {
public:
    mutable int callCount{};
    mutable gts::HttpRequest lastRequest;
    gts::HttpResponse response;

    [[nodiscard]] gts::HttpResponse postJson(
        const gts::HttpRequest& request) const override
    {
        ++callCount;
        lastRequest = request;
        return response;
    }
};

inline void setEnvValue(const std::string& name, const std::string& value)
{
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

inline void clearEnvValue(const std::string& name)
{
#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif
}

inline bool hasHeader(const std::vector<std::pair<std::string, std::string>>& headers,
                      const std::string& name,
                      const std::string& value)
{
    for (const auto& [headerName, headerValue] : headers) {
        if (headerName == name && headerValue == value) {
            return true;
        }
    }
    return false;
}

inline std::string writeTempCsv(const std::string& name, const std::string& content)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::ofstream file(path, std::ios::binary);
    file << content;
    return path.string();
}

} // namespace gts_test
