#include <edgemeter/core/Config.hpp>
#include <edgemeter/core/Logger.hpp>
#include <edgemeter/telemetry/TelemetryExporter.hpp>
#include <edgemeter/telemetry/exporters/HttpOtelExporter.hpp>
#include <edgemeter/telemetry/collectors/AbiPluginCollector.hpp>

#include <tuple>
#include <span>
#include <thread>
#include <chrono>

using ExporterT = telemetry::HttpOtelExporter<telemetry::Plain>;

template<telemetry::Exporter... Exporters>
class MultiplexingObserver {
    std::tuple<Exporters...> exporters_;
public:
    explicit MultiplexingObserver(Exporters&&... exp) : exporters_(std::forward<Exporters>(exp)...) {}

    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attrs = {}) {
        std::apply([&](auto&... e) {
            (e.Observe(name, value, attrs), ...);
        }, exporters_);
    }
};

int main(int argc, char** argv) {
    core::Logger::SetLevel(core::LogLevel::DEBUG);
    core::Config config;
    config.http.port = 4318; // Binding accurately to the explicit Go test handler constraint

#if __has_include(<dlfcn.h>)
    std::string plugin_path = (argc > 1) ? argv[1] : "./bin/mock_plugin.so";
    MultiplexingObserver observer{ExporterT{config}};
    
    // Wire the hardware collection plugin directly into the generic MetricObserver
    telemetry::collectors::AbiPluginCollector<decltype(observer)> plugin(observer);
    if (!plugin.initialize(plugin_path).has_value()) return 1;

    // Continuously simulate metrics polling bridging testing frameworks
    while (true) {
        plugin.poll(observer);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
#else
    return 1;
#endif
}
