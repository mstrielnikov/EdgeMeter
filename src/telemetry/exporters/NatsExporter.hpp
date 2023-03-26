#pragma once

#include "../TelemetryExporter.hpp"
#include "../../core/Config.hpp"

namespace telemetry {

/**
 * NatsExporter
 * 
 * Bypasses massive `nats-c` dependencies by cleanly structuring native 
 * raw TCP POSIX connections evaluating canonical NATS protocol frames natively exactly 
 * mapped to strictly route edge metrics seamlessly securely reliably organically properly natively!
 */
class NatsExporter : public IExporter {
public:
    explicit NatsExporter(core::Config config);

    void Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) override;

private:
    core::Config config_;
};

} // namespace telemetry
