#include <edgemeter/telemetry/exporters/WebSocketExporter.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <format>

namespace telemetry {

// ---------------------------------------------------------------------------
// Plain (TCP) Implementation
// ---------------------------------------------------------------------------
void WebSocketExporter<Plain>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        core::Logger::Error("WebSocket Exporter: empty payload");
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.ws.port));
    inet_pton(AF_INET, config_.ws.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string req = std::format(
            "GET / HTTP/{}\r\n"
            "Host: {}:{}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: {}\r\n"
            "Sec-WebSocket-Version: {}\r\n\r\n",
            config_.ws.http_version, config_.ws.host, config_.ws.port,
            config_.ws.key, config_.ws.ws_version);
        char frame_hdr[10];
        size_t f_idx = 0;
        frame_hdr[f_idx++] = static_cast<char>(WS_OPCODE_TEXT);
        if (payload.size() <= 125) {
            frame_hdr[f_idx++] = static_cast<char>(payload.size() | WS_PAYLOAD_MASK);
        } else if (payload.size() <= 65535) {
            frame_hdr[f_idx++] = static_cast<char>(WS_EXTENDED_PAYLOAD_16 | WS_PAYLOAD_MASK);
            frame_hdr[f_idx++] = static_cast<char>((payload.size() >> 8) & 0xFF);
            frame_hdr[f_idx++] = static_cast<char>(payload.size() & 0xFF);
        }
        frame_hdr[f_idx++] = 0; frame_hdr[f_idx++] = 0; frame_hdr[f_idx++] = 0; frame_hdr[f_idx++] = 0;
        std::string_view frame_sv(frame_hdr, f_idx);
        std::string_view payload_sv(payload);

        if (send(sock, req.data(), req.size(), 0) < 0) {
            core::Logger::Error("WebSocket upgrade request failed");
            close(sock); return;
        }
        char buf[512];
        recv(sock, buf, sizeof(buf), 0);
        if (send(sock, frame_sv.data(), frame_sv.size(), 0) < 0 ||
            send(sock, payload_sv.data(), payload_sv.size(), 0) < 0) {
            core::Logger::Error("WebSocket Exporter failed to send TCP payload");
        }
    } else {
        core::Logger::Warn("WebSocket Exporter: connection refused, will retry");
    }
    close(sock);
}

// ---------------------------------------------------------------------------
// Secure (TLS) Implementation
// ---------------------------------------------------------------------------
void WebSocketExporter<Secure>::Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        core::Logger::Error("WebSocket Exporter: empty payload");
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(static_cast<uint16_t>(config_.ws.port));
    inet_pton(AF_INET, config_.ws.host.data(), &serv_addr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) >= 0) {
        std::string req = std::format(
            "GET / HTTP/{}\r\n"
            "Host: {}:{}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: {}\r\n"
            "Sec-WebSocket-Version: {}\r\n\r\n",
            config_.ws.http_version, config_.ws.host, config_.ws.port,
            config_.ws.key, config_.ws.ws_version);
        char frame_hdr[10];
        size_t f_idx = 0;
        frame_hdr[f_idx++] = static_cast<char>(WS_OPCODE_TEXT);
        if (payload.size() <= 125) {
            frame_hdr[f_idx++] = static_cast<char>(payload.size() | WS_PAYLOAD_MASK);
        } else if (payload.size() <= 65535) {
            frame_hdr[f_idx++] = static_cast<char>(WS_EXTENDED_PAYLOAD_16 | WS_PAYLOAD_MASK);
            frame_hdr[f_idx++] = static_cast<char>((payload.size() >> 8) & 0xFF);
            frame_hdr[f_idx++] = static_cast<char>(payload.size() & 0xFF);
        }
        frame_hdr[f_idx++] = 0;
        frame_hdr[f_idx++] = 0;
        frame_hdr[f_idx++] = 0;
        frame_hdr[f_idx++] = 0;
        std::string_view frame_sv(frame_hdr, f_idx);
        std::string_view payload_sv(payload);

        net::TlsConnection<net::Unauthenticated> unauth_conn(sock);
        auto auth_res = unauth_conn.authenticate_as_client(config_);
        if (auth_res.has_value()) {
            auto& tls_conn = auth_res.value();
            if (!tls_conn.send(req).has_value()) {
                core::Logger::Error("WebSocket TLS upgrade request failed");
                close(sock); return;
            }
            tls_conn.receive(512);
            if (!tls_conn.send(frame_sv).has_value() || !tls_conn.send(payload_sv).has_value()) {
                core::Logger::Error("WebSocket TLS Exporter failed to send payload");
            }
        } else {
            core::Logger::Error("WebSocket TLS handshake failed: " + std::string(auth_res.error()));
        }
    } else {
        core::Logger::Warn("WebSocket Exporter: connection refused, will retry");
    }
    close(sock);
}

} // namespace telemetry
