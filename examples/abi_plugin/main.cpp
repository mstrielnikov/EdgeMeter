#include <edgemeter/telemetry/collectors/AbiPluginCollector.hpp>
#include <edgemeter/core/Logger.hpp>
#include <thread>
#include <chrono>

// Exact replica bounding MetricObserver behaviors
struct StdoutObserver {
    void Observe(std::string_view name, double value, std::span<const sys::Attribute> attrs = {}) {
        std::string extra_attrs;
        for (const auto& a : attrs) {
            extra_attrs += std::string("[") + std::string(a.key) + "=" + std::string(a.val) + "] ";
        }
        core::Logger::Info(std::string("Plugin Emit: ") + std::string(name) + 
                           " -> " + std::to_string(value) + " " + extra_attrs);
    }
};

int main(int argc, char** argv) {
    core::Logger::SetLevel(core::LogLevel::INFO);
    core::Logger::Info("Starting standalone C ABI Plugin Boundary Demo...");

#if __has_include(<dlfcn.h>)
    std::string plugin_path = (argc > 1) ? argv[1] : "./bin/mock_plugin.so";
    core::Logger::Info("Attempting to load standard 'extern \"C\"' plugin at: " + plugin_path);
    
    StdoutObserver observer;
    telemetry::collectors::AbiPluginCollector<StdoutObserver> plugin(observer);

    auto result = plugin.initialize(plugin_path);
    if (!result.has_value()) {
        core::Logger::Error("Failed opening the ABI Shared Object securely: " + std::string(result.error()));
        return 1;
    }

    core::Logger::Info("Beginning plugin cycle (listening for external intercepts)...");
    for (int i = 0; i < 3; ++i) {
        plugin.poll(observer);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    core::Logger::Info("External ABI Plugin cleanly detached.");
    return 0;
#else
    core::Logger::Error("Critical failure: The underlying OS target does not map <dlfcn.h>. Dynamic plugins disabled!");
    return 1;
#endif
}
