#include <edgemeter/telemetry/exporters/NatsExporter.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <format>

namespace telemetry {

// ---------------------------------------------------------------------------
// Plain (TCP) Implementation
// ---------------------------------------------------------------------------
void NatsExporter<Plain>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) return;

    constexpr std::string_view nats_connect_req = "CONNECT {\"verbose\":false,\"pedantic\":false,\"tls_required\":false}\r\n";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.nats.port));
    inet_pton(AF_INET, config_.nats.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string hdr = std::format("PUB {} {}\r\n", config_.nats.topic, payload.size());
        std::string_view payload_sv(payload);
        std::string_view end_sv("\r\n", 2);

        if (send(sock, nats_connect_req.data(), nats_connect_req.size(), 0) < 0 ||
            send(sock, hdr.data(), hdr.size(), 0) < 0 ||
            send(sock, payload_sv.data(), payload_sv.size(), 0) < 0 ||
            send(sock, end_sv.data(), end_sv.size(), 0) < 0) {
            core::Logger::Error("NATS Exporter failed to send TCP payload");
        }
    } else {
        core::Logger::Warn("NATS Exporter: connection refused, will retry");
    }
    close(sock);
}

// ---------------------------------------------------------------------------
// Secure (TLS) Implementation
// ---------------------------------------------------------------------------
void NatsExporter<Secure>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) return;

    constexpr std::string_view nats_connect_req = "CONNECT {\"verbose\":false,\"pedantic\":false,\"tls_required\":true}\r\n";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.nats.port));
    inet_pton(AF_INET, config_.nats.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string hdr = std::format("PUB {} {}\r\n", config_.nats.topic, payload.size());
        std::string_view payload_sv(payload);
        std::string_view end_sv("\r\n", 2);

        net::TlsConnection<net::Unauthenticated> unauth_conn(sock);
        auto auth_res = unauth_conn.authenticate_as_client(config_);
        if (auth_res.has_value()) {
            auto& tls_conn = auth_res.value();
            if (!tls_conn.send(nats_connect_req).has_value() ||
                !tls_conn.send(hdr).has_value() ||
                !tls_conn.send(payload_sv).has_value() ||
                !tls_conn.send(end_sv).has_value()) {
                core::Logger::Error("NATS TLS Exporter failed to send data");
            }
        } else {
            core::Logger::Error("NATS TLS handshake failed: " + std::string(auth_res.error()));
        }
    } else {
        core::Logger::Warn("NATS Exporter: connection refused, will retry");
    }
    close(sock);
}

} // namespace telemetry