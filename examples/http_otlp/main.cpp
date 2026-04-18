// HTTP/OTLP Example
// Pushes OpenTelemetry metrics over plain HTTP/1.1 (or HTTPS when compiled with USE_TLS=1).
//
// Build (plain):   make http_otlp USE_TLS=0
// Build (TLS):     make http_otlp USE_TLS=1

#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/telemetry/exporters/HttpOtelExporter.hpp>
#include "../common/agent.hpp"

#include <tuple>
#include <span>

#ifdef USE_TLS
    using ExporterT = telemetry::HttpOtelExporter<telemetry::Secure>;
#else
    using ExporterT = telemetry::HttpOtelExporter<telemetry::Plain>;
#endif

// ---------------------------------------------------------------------------
// Variadic MultiplexingObserver — concept-constrained, zero-allocation.
// Stores exporters by value in a std::tuple; no heap, no vtable.
// ---------------------------------------------------------------------------
template<telemetry::Exporter... Exporters>
class MultiplexingObserver {
    std::tuple<Exporters...> exporters_;
public:
    explicit MultiplexingObserver(Exporters&&... exp) : exporters_(std::forward<Exporters>(exp)...) {}

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attrs) {
        std::apply([&](auto&... e) {
            (e.Observe(name, value, attrs), ...);
        }, exporters_);
    }
};

int main() {
    edgemeter::setup_signals();

    core::Config config;
    // config.grpc_host = "otel-collector.example.com";
    // config.grpc_port = 4317;

    edgemeter::setup_logging(config);
    core::Logger::Info("Starting " + std::string(config.app.program_name) + " HTTP/OTLP agent");

    MultiplexingObserver observer{ExporterT{config}};
    core::Logger::Info("Registered HTTP/OTLP exporter");

    edgemeter::run_metric_loop(observer);

    core::Logger::Info("EdgeMeter HTTP/OTLP agent shutting down.");
    return 0;
}
