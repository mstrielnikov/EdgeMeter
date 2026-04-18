// NATS/OTLP Example
// Pushes OpenTelemetry metrics over native NATS TCP PUB protocol.
//
// Build (plain):   make nats_otlp USE_TLS=0
// Build (TLS):     make nats_otlp USE_TLS=1

#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/telemetry/exporters/NatsExporter.hpp>
#include "../common/agent.hpp"

#include <tuple>
#include <span>

#ifdef USE_TLS
    using ExporterT = telemetry::NatsExporter<telemetry::Secure>;
#else
    using ExporterT = telemetry::NatsExporter<telemetry::Plain>;
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
    // config.nats_host  = "nats.example.com";
    // config.nats_port  = 4222;
    // config.nats_topic = "telemetry.metrics.otlp";

    edgemeter::setup_logging(config);
    core::Logger::Info("Starting " + std::string(config.app.program_name) + " NATS/OTLP agent");

    MultiplexingObserver observer{ExporterT{config}};
    core::Logger::Info("Registered NATS/OTLP exporter");

    edgemeter::run_metric_loop(observer);

    core::Logger::Info("EdgeMeter NATS/OTLP agent shutting down.");
    return 0;
}
