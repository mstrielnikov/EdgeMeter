#ifndef OTEL_GRPC_EXPORTER_HPP
#define OTEL_GRPC_EXPORTER_HPP

#include "../TelemetryExporter.hpp"
#include "../../core/Config.hpp"
#include <string>

namespace telemetry {

class OtelGrpcExporter : public IExporter {
public:
    explicit OtelGrpcExporter(core::Config config);
    virtual ~OtelGrpcExporter() = default;
    
    // Abstract canonical observer catching unified metric payloads
    void Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) override;

private:
    core::Config config_;
};

}

#endif // OTEL_GRPC_EXPORTER_HPP
