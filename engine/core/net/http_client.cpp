#include "engine/core/net/http_client.h"

namespace urpg::net {
namespace {

class UnconfiguredHttpClient final : public IHttpClient {
  public:
    HttpResponse postJson(const HttpRequest&) override {
        return {0, "", "native_http_client_not_configured"};
    }
};

} // namespace

IHttpClient& defaultHttpClient() {
    static UnconfiguredHttpClient client;
    return client;
}

} // namespace urpg::net
