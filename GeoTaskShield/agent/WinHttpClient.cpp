#include "agent/WinHttpClient.h"

#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#endif

namespace gts {

namespace {

#ifdef _WIN32

struct WinHttpHandle {
    HINTERNET value{};

    explicit WinHttpHandle(HINTERNET handle)
        : value(handle)
    {
    }

    WinHttpHandle(const WinHttpHandle&) = delete;
    WinHttpHandle& operator=(const WinHttpHandle&) = delete;

    ~WinHttpHandle()
    {
        if (value != nullptr) {
            WinHttpCloseHandle(value);
        }
    }
};

std::wstring utf8ToWide(std::string_view value)
{
    if (value.empty()) {
        return {};
    }

    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(),
                                         static_cast<int>(value.size()),
                                         nullptr, 0);
    if (size <= 0) {
        return {};
    }

    std::wstring output(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        output.data(), size);
    return output;
}

std::string winHttpError(const std::string& action)
{
    return action + " failed with WinHTTP error " + std::to_string(GetLastError());
}

struct ParsedUrl {
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port{};
    bool secure{};
    std::string errorMessage;
};

ParsedUrl parseUrl(const std::string& url)
{
    ParsedUrl parsed;
    const std::wstring wideUrl = utf8ToWide(url);
    if (wideUrl.empty()) {
        parsed.errorMessage = "URL is empty or not valid UTF-8.";
        return parsed;
    }

    URL_COMPONENTSW components{};
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = static_cast<DWORD>(-1);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &components)) {
        parsed.errorMessage = winHttpError("WinHttpCrackUrl");
        return parsed;
    }

    if (components.nScheme != INTERNET_SCHEME_HTTPS) {
        parsed.errorMessage = "Only HTTPS provider URLs are supported.";
        return parsed;
    }

    parsed.secure = true;
    parsed.port = components.nPort;
    parsed.host.assign(components.lpszHostName, components.dwHostNameLength);
    parsed.path.assign(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.dwExtraInfoLength > 0) {
        parsed.path.append(components.lpszExtraInfo,
                           components.dwExtraInfoLength);
    }
    if (parsed.path.empty()) {
        parsed.path = L"/";
    }

    return parsed;
}

std::wstring requestHeaders(
    const std::vector<std::pair<std::string, std::string>>& headers)
{
    std::wstring output;
    for (const auto& [name, value] : headers) {
        output += utf8ToWide(name);
        output += L": ";
        output += utf8ToWide(value);
        output += L"\r\n";
    }
    return output;
}

#endif

} // namespace

HttpResponse WinHttpClient::postJson(const HttpRequest& request) const
{
#ifdef _WIN32
    HttpResponse response;
    const ParsedUrl parsed = parseUrl(request.url);
    if (!parsed.errorMessage.empty()) {
        response.errorMessage = parsed.errorMessage;
        return response;
    }

    WinHttpHandle session(WinHttpOpen(L"GeoTaskShield/1.0",
                                      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS, 0));
    if (session.value == nullptr) {
        response.errorMessage = winHttpError("WinHttpOpen");
        return response;
    }

    WinHttpHandle connection(
        WinHttpConnect(session.value, parsed.host.c_str(), parsed.port, 0));
    if (connection.value == nullptr) {
        response.errorMessage = winHttpError("WinHttpConnect");
        return response;
    }

    WinHttpHandle httpRequest(WinHttpOpenRequest(
        connection.value, L"POST", parsed.path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE));
    if (httpRequest.value == nullptr) {
        response.errorMessage = winHttpError("WinHttpOpenRequest");
        return response;
    }
    if (!WinHttpSetTimeouts(httpRequest.value, request.timeoutMs,
                            request.timeoutMs, request.timeoutMs,
                            request.timeoutMs)) {
        response.errorMessage = winHttpError("WinHttpSetTimeouts");
        return response;
    }

    const std::wstring headers = requestHeaders(request.headers);
    const DWORD bodySize = static_cast<DWORD>(request.body.size());
    if (!WinHttpSendRequest(httpRequest.value, headers.c_str(),
                            static_cast<DWORD>(headers.size()),
                            const_cast<char*>(request.body.data()), bodySize,
                            bodySize, 0)) {
        response.errorMessage = winHttpError("WinHttpSendRequest");
        return response;
    }

    if (!WinHttpReceiveResponse(httpRequest.value, nullptr)) {
        response.errorMessage = winHttpError("WinHttpReceiveResponse");
        return response;
    }

    DWORD statusCode{};
    DWORD statusSize = sizeof(statusCode);
    if (WinHttpQueryHeaders(httpRequest.value,
                            WINHTTP_QUERY_STATUS_CODE |
                                WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode,
                            &statusSize, WINHTTP_NO_HEADER_INDEX)) {
        response.statusCode = static_cast<int>(statusCode);
    }

    DWORD available{};
    while (WinHttpQueryDataAvailable(httpRequest.value, &available) &&
           available > 0) {
        std::string chunk(static_cast<std::size_t>(available), '\0');
        DWORD bytesRead{};
        if (!WinHttpReadData(httpRequest.value, chunk.data(), available,
                             &bytesRead)) {
            response.errorMessage = winHttpError("WinHttpReadData");
            return response;
        }
        chunk.resize(static_cast<std::size_t>(bytesRead));
        response.body += chunk;
    }

    response.success = response.statusCode >= 200 && response.statusCode < 300;
    if (!response.success) {
        response.errorMessage =
            "Provider returned HTTP status " +
            std::to_string(response.statusCode);
    }
    return response;
#else
    (void)request;
    return HttpResponse{false, 0, {},
                        "HTTP provider calls are only implemented on Windows."};
#endif
}

} // namespace gts
