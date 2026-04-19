#include <edgemeter/telemetry/collectors/NvmlCollector.hpp>
#include <edgemeter/core/Logger.hpp>
#include <thread>
#include <chrono>
#include <string>

// A mock observer satisfying the MetricObserver concept
struct StdoutObserver {
    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attrs = {}) {
        std::string extra_attrs;
        for (const auto& a : attrs) {
            extra_attrs += std::string("[") + std::string(a.key) + "=" + std::string(a.val) + "] ";
        }
        core::Logger::Info(std::string("Hardware Metric: ") + std::string(name) + 
                           " -> " + std::to_string(value) + " " + extra_attrs);
    }
};

int main() {
    core::Logger::SetLevel(core::LogLevel::INFO);
    core::Logger::Info("Starting standalone NVML Mock Collector Execution Boundary...");

    StdoutObserver observer;
    
    // Explicitly constructing the Collector typestate mappings 
    telemetry::collectors::NvmlCollector<StdoutObserver> nvml(observer);

    auto res = nvml.initialize("/usr/lib/libnvidia-ml.so");
    if (!res.has_value()) {
        core::Logger::Error("Failed to mount NVML pipeline: " + std::string(res.error()));
        return 1;
    }

    core::Logger::Info("Gathering explicit GPU bounds...");
    for (int i = 0; i < 3; ++i) {
        nvml.poll(observer);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    core::Logger::Info("NVML Collector shutting down cleanly.");
    return 0;
}
