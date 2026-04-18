#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string_view>

namespace core {

namespace Constants {
    constexpr std::string_view ProgramName = "EdgeMeter";
    constexpr std::string_view CppVersion = "C++23";
    constexpr int              DefaultHttpPort    = 4317;
    constexpr std::string_view DefaultHttpHost    = "127.0.0.1";
    constexpr std::string_view DefaultHttpPath    = "/v1/metrics";
    constexpr std::string_view DefaultHttpVersion = "1.1";

    constexpr int              DefaultWsPort        = 8080;
    constexpr std::string_view DefaultWsHost        = "127.0.0.1";
    constexpr std::string_view DefaultWsKey         = "dGhlIHNhbXBsZSBub25jZQ==";
    constexpr std::string_view DefaultWsHttpVersion = "1.1";
    constexpr std::string_view DefaultWsVersion     = "13";

    constexpr int              DefaultNatsPort  = 4222;
    constexpr std::string_view DefaultNatsHost  = "127.0.0.1";
    constexpr std::string_view DefaultNatsTopic = "telemetry.metrics.otlp";

    constexpr std::string_view DefaultHostname    = "edge-device-1";
    constexpr std::string_view DefaultLogLevel    = "INFO";
    constexpr std::string_view DefaultTlsVersion  = "";
    constexpr std::string_view DefaultTlsCertPath = "certs/server.crt";
    constexpr std::string_view DefaultTlsKeyPath  = "certs/server.key";
} // namespace Constants

// ---------------------------------------------------------------------------
// Composable sub-configs
//
// Each sub-config is a focused, zero-allocation aggregate for one protocol
// or concern. Consumers (exporters, TlsConnection) store only what they need.
// All fields are string_view — no heap allocation at construction.
// ---------------------------------------------------------------------------

// Application-level metadata shared across all exporters.
struct AppConfig {
    std::string_view program_name = Constants::ProgramName;
    std::string_view cpp_version  = Constants::CppVersion;
    std::string_view log_level    = Constants::DefaultLogLevel;
    std::string_view hostname     = Constants::DefaultHostname;
};

// HTTP/OTLP push exporter endpoint.
struct HttpConfig {
    int              port    = Constants::DefaultHttpPort;
    std::string_view host    = Constants::DefaultHttpHost;
    std::string_view path    = Constants::DefaultHttpPath;
    std::string_view version = Constants::DefaultHttpVersion;
};

// WebSocket (RFC 6455) exporter endpoint.
struct WsConfig {
    int              port         = Constants::DefaultWsPort;
    std::string_view host         = Constants::DefaultWsHost;
    std::string_view key          = Constants::DefaultWsKey;
    std::string_view http_version = Constants::DefaultWsHttpVersion;
    std::string_view ws_version   = Constants::DefaultWsVersion;
};

// NATS pub-sub exporter endpoint.
struct NatsConfig {
    int              port  = Constants::DefaultNatsPort;
    std::string_view host  = Constants::DefaultNatsHost;
    std::string_view topic = Constants::DefaultNatsTopic;
};

// TLS handshake parameters — always syntactically present.
// version defaults to "" (not configured / not applicable).
//   Plain builds: OtlpTemplateEngine renders "" as "none" in the OTLP payload.
//   Secure builds: resolve_tls_version("") safely selects TLS 1.3 as the default.
struct TlsConfig {
    std::string_view version   = Constants::DefaultTlsVersion;
    std::string_view cert_path = Constants::DefaultTlsCertPath;
    std::string_view key_path  = Constants::DefaultTlsKeyPath;
};

// ---------------------------------------------------------------------------
// Top-level composed Config.
//
// Zero-allocation, edge / embedded compatible.
// Sub-configs can be constructed and overridden independently:
//
//   core::Config cfg;
//   cfg.http.port = 9090;
//   cfg.tls.version = "TLSv1.2";
// ---------------------------------------------------------------------------
struct Config {
    AppConfig  app;
    HttpConfig http;
    WsConfig   ws;
    NatsConfig nats;
    TlsConfig  tls;
};

} // namespace core

#endif // CONFIG_HPP
