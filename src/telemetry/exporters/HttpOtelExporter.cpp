#include <edgemeter/telemetry/exporters/HttpOtelExporter.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <format>

namespace telemetry {

// ---------------------------------------------------------------------------
// Plain (TCP) Implementation
// ---------------------------------------------------------------------------
void HttpOtelExporter<Plain>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        core::Logger::Error("OTEL/HTTP Exporter aborted: empty payload");
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.http.port));
    inet_pton(AF_INET, config_.http.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string hdr = std::format(
            "POST {} HTTP/{}\r\n"
            "Host: {}:{}\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: {}\r\n\r\n",
            config_.http.path, config_.http.version,
            config_.http.host, config_.http.port, payload.size());
        std::string_view payload_sv(payload);

        if (send(sock, hdr.data(), hdr.size(), 0) < 0 ||
            send(sock, payload_sv.data(), payload_sv.size(), 0) < 0) {
            core::Logger::Error("OTEL/HTTP Exporter failed to send TCP payload");
        }
    } else {
        core::Logger::Warn("OTEL/HTTP Exporter: connection refused, will retry");
    }
    close(sock);
}

// ---------------------------------------------------------------------------
// Secure (TLS) Implementation
// ---------------------------------------------------------------------------
void HttpOtelExporter<Secure>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        core::Logger::Error("OTEL/HTTP Exporter aborted: empty payload");
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.http.port));
    inet_pton(AF_INET, config_.http.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string hdr = std::format(
            "POST {} HTTP/{}\r\n"
            "Host: {}:{}\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: {}\r\n\r\n",
            config_.http.path, config_.http.version,
            config_.http.host, config_.http.port, payload.size());
        std::string_view payload_sv(payload);

        net::TlsConnection<net::Unauthenticated> unauth_conn(sock);
        auto auth_res = unauth_conn.authenticate_as_client(config_);
        if (auth_res.has_value()) {
            auto& tls_conn = auth_res.value();
            if (!tls_conn.send(hdr).has_value() || !tls_conn.send(payload_sv).has_value()) {
                core::Logger::Error("OTEL/HTTP TLS Exporter failed to send data");
            }
        } else {
            core::Logger::Error("OTEL/HTTP TLS handshake failed: " + std::string(auth_res.error()));
        }
    } else {
        core::Logger::Warn("OTEL/HTTP Exporter: connection refused, will retry");
    }
    close(sock);
}

} // namespace telemetry