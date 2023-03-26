#include "telemetry/SystemMetrics.hpp"
#include "telemetry/exporters/OtelGrpcExporter.hpp"
#include "telemetry/exporters/WebSocketExporter.hpp"
#include "telemetry/exporters/NatsExporter.hpp"
#include "net/AsyncServer.hpp"
#include "core/Logger.hpp"
#include "core/Config.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <csignal>

#ifndef __EMSCRIPTEN__
#include <thread>
#include <chrono>
#endif

std::atomic<bool> g_running{true};

// Handle operational observability controls without crashing forcefully
void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        LOG_INFO("Shutdown signal received, initiating graceful exit...");
        g_running = false;
    } else if (signum == SIGHUP) {
        if (core::Logger::GetLevel() == core::LogLevel::DEBUG) {
            core::Logger::SetLevel(core::LogLevel::INFO);
            LOG_INFO("SIGHUP: Log Level verbosity shifted to INFO.");
        } else {
            core::Logger::SetLevel(core::LogLevel::DEBUG);
            LOG_INFO("SIGHUP: Log Level verbosity shifted to DEBUG.");
        }
    }
}

// We create a MultiplexingObserver to broadcast cleanly scaling out metrics independently of payloads
class MultiplexingObserver : public sys::IMetricObserver {
    std::vector<std::unique_ptr<telemetry::IExporter>> exporters_;
public:
    void add_exporter(std::unique_ptr<telemetry::IExporter> exporter) {
        exporters_.push_back(std::move(exporter));
    }
    void Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) override {
        for (auto& exp : exporters_) {
            exp->Observe(name, value, attributes);
        }
    }
};

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGHUP, signal_handler);

    core::Logger::SetLevel(core::LogLevel::INFO);
    LOG_INFO("Starting EdgeMeter C++20...");

    auto config_res = core::Config::Load("config.json");
    core::Config config;
    if (config_res.is_ok()) {
        config = std::move(config_res.unwrap());
        LOG_INFO("Loaded explicit configuration natively.");
    } else {
        LOG_INFO("Configuration file absent or malformed, reverting strictly to Constants.");
    }

    MultiplexingObserver observer;

#ifdef USE_GRPC
    observer.add_exporter(std::make_unique<telemetry::OtelGrpcExporter>(config));
    LOG_INFO("Registered OTEL/gRPC exporter");
#endif

#ifdef USE_WEBSOCKETS
    observer.add_exporter(std::make_unique<telemetry::WebSocketExporter>(config));
    LOG_INFO("Registered WebSocket exporter");
#endif

#ifdef USE_NATS
    observer.add_exporter(std::make_unique<telemetry::NatsExporter>(config));
    LOG_INFO("Registered Native NATS TCP exporter");
#endif

    net::AsyncServer server(config.server_port);
    core::Result<void> start_res = server.start();
    if (start_res.is_err()) {
        LOG_ERROR("Failed to start server: " + start_res.unwrap_err());
        return 1;
    }

    // Unbounded telemetry lifecycle gracefully terminated by registered signals natively
    while (g_running) {
        core::Result<void> res = sys::CollectProcessMetrics(&observer);
        if (res.is_err()) {
            LOG_ERROR("Failed to collect canonical metrics: " + res.unwrap_err());
        }

#ifndef __EMSCRIPTEN__
        std::this_thread::sleep_for(std::chrono::seconds(1));
#endif
    }

    LOG_INFO("EdgeMeter shutting down safely.");
    server.stop();
    return 0;
}
