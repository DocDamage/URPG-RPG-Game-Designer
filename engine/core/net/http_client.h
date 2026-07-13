#pragma once

#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

namespace urpg::net {

struct HttpRequest {
    std::string url;
    std::map<std::string, std::string> headers;
    nlohmann::json jsonBody = nlohmann::json::object();
    std::chrono::milliseconds timeout{30000};
};

struct HttpResponse {
    int statusCode = 0;
    std::string body;
    std::string error;

    bool success() const { return statusCode >= 200 && statusCode < 300 && error.empty(); }
};

class IHttpClient {
  public:
    virtual ~IHttpClient() = default;
    virtual HttpResponse postJson(const HttpRequest& request) = 0;
};

IHttpClient& defaultHttpClient();

} // namespace urpg::net
