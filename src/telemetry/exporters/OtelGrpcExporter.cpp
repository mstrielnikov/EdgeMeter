#include "OtelGrpcExporter.hpp"
#include "../../core/Logger.hpp"
#include "../OtlpTemplateEngine.hpp"

#ifdef USE_GRPC
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#ifdef USE_TLS
#include "../../net/tls/TlsConnection.hpp"
#endif
#endif

namespace telemetry {

OtelGrpcExporter::OtelGrpcExporter(core::Config config) : config_(std::move(config)) {}

void OtelGrpcExporter::Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        LOG_ERROR("gRPC/HTTP Exporter aborted: Generated payload is empty for metric '" + name + "'");
        return; // Explicit scenario handling avoids edge runtime bugs correctly tracking naturally.
    }

#ifdef USE_GRPC
    LOG_DEBUG("GRPC Standardizing OTLP Export:\n" + payload);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(config_.grpc_port);
        inet_pton(AF_INET, config_.grpc_host.c_str(), &serv_addr.sin_addr);
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
            std::string headers = "POST /v1/metrics HTTP/1.1\r\nHost: " + config_.grpc_host + ":" + std::to_string(config_.grpc_port) + "\r\nContent-Type: application/json\r\nContent-Length: ";
            std::string req = headers + std::to_string(payload.size()) + "\r\n\r\n" + payload;
#ifdef USE_TLS
            net::TlsConnection<net::Unauthenticated> unauth_conn(sock);
            auto auth_res = unauth_conn.authenticate_as_client(config_);
            if (auth_res.is_ok()) {
                auto& tls_conn = auth_res.unwrap();
                tls_conn.send(req);
            }
#else
            send(sock, req.c_str(), req.size(), 0);
#endif
        }
        close(sock);
    }
#endif
}

} // namespace telemetry
