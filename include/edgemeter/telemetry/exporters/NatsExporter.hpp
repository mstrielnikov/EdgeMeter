#ifndef NATS_EXPORTER_HPP
#define NATS_EXPORTER_HPP

#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Result.hpp>
#include <span>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/OtlpTemplateEngine.hpp>
#include <edgemeter/net/tls/TlsConnection.hpp>

#include <string_view>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace telemetry {

// Plain and Secure typestates are defined in TelemetryExporter.hpp.

template <TransportMode Mode>
class NatsExporter;

// ---------------------------------------------------------------------------
// Plain (TCP) — no TLS handshake
// ---------------------------------------------------------------------------
template <>
class NatsExporter<Plain> {
public:
    explicit NatsExporter(core::Config config) : config_(std::move(config)) {}
    ~NatsExporter() = default;

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes);
private:
    core::Config config_;
};

// ---------------------------------------------------------------------------
// Secure (TLS) — authenticate_as_client() called before each send
// ---------------------------------------------------------------------------
template <>
class NatsExporter<Secure> {
public:
    explicit NatsExporter(core::Config config) : config_(std::move(config)) {}
    ~NatsExporter() = default;

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attributes);
private:
    core::Config config_;
};

static_assert(telemetry::Exporter<NatsExporter<Plain>>,  "NatsExporter<Plain> must satisfy telemetry::Exporter");
static_assert(telemetry::Exporter<NatsExporter<Secure>>, "NatsExporter<Secure> must satisfy telemetry::Exporter");

} // namespace telemetry

#endif // NATS_EXPORTER_HPP
