#ifndef WEBSOCKET_EXPORTER_HPP
#define WEBSOCKET_EXPORTER_HPP

#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Result.hpp>
#include <span>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/OtlpTemplateEngine.hpp>
#include <edgemeter/net/tls/TlsConnection.hpp>

#include <string_view>
#include <string>

// RFC 6455 framing constants
#define WS_OPCODE_TEXT         0x81
#define WS_PAYLOAD_MASK        0x80
#define WS_EXTENDED_PAYLOAD_16 126

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace telemetry {

// Plain and Secure typestates are defined in TelemetryExporter.hpp.

template <TransportMode Mode>
class WebSocketExporter;

// ---------------------------------------------------------------------------
// Plain (TCP) — no TLS handshake
// ---------------------------------------------------------------------------
template <>
class WebSocketExporter<Plain> {
public:
    explicit WebSocketExporter(core::Config config) : config_(std::move(config)) {}
    ~WebSocketExporter() = default;

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes);
private:
    core::Config config_;
};

// ---------------------------------------------------------------------------
// Secure (TLS) — authenticate_as_client() called before each send
// ---------------------------------------------------------------------------
template <>
class WebSocketExporter<Secure> {
public:
    explicit WebSocketExporter(core::Config config) : config_(std::move(config)) {}
    ~WebSocketExporter() = default;

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes);
private:
    core::Config config_;
};

static_assert(telemetry::Exporter<WebSocketExporter<Plain>>,  "WebSocketExporter<Plain> must satisfy telemetry::Exporter");
static_assert(telemetry::Exporter<WebSocketExporter<Secure>>, "WebSocketExporter<Secure> must satisfy telemetry::Exporter");

}

#endif // WEBSOCKET_EXPORTER_HPP
