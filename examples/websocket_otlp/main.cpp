// WebSocket/OTLP Example
// Pushes OpenTelemetry metrics over WebSocket (RFC 6455) with JSON payload in OTEL format.
//
// Build (plain):   make ws_otlp USE_TLS=0
// Build (TLS):     make ws_otlp USE_TLS=1

#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/telemetry/exporters/WebSocketExporter.hpp>
#include "../common/agent.hpp"

#include <tuple>
#include <span>

#ifdef USE_TLS
    using ExporterT = telemetry::WebSocketExporter<telemetry::Secure>;
#else
    using ExporterT = telemetry::WebSocketExporter<telemetry::Plain>;
#endif

template<telemetry::Exporter... Exporters>
class MultiplexingObserver {
    std::tuple<Exporters...> exporters_;
public:
    explicit MultiplexingObserver(Exporters&&... exp): exporters_(std::forward<Exporters>(exp)...) {}

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attrs) {
        std::apply([&](auto&... e) {
            (e.Observe(name, value, attrs), ...);
        }, exporters_);
    }
};

int main() {
    edgemeter::setup_signals();

    core::Config config;
    // config.ws_host = "metrics.example.com";
    // config.ws_port = 8080;

    edgemeter::setup_logging(config);
    core::Logger::Info("Starting " + std::string(config.app.program_name) + " WebSocket/OTLP agent");

    MultiplexingObserver observer{ExporterT{config}};
    core::Logger::Info("Registered WebSocket/OTLP exporter");

    edgemeter::run_metric_loop(observer);

    core::Logger::Info("EdgeMeter WebSocket/OTLP agent shutting down.");
    return 0;
}
