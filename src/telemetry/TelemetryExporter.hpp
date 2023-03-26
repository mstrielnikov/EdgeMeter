#ifndef TELEMETRY_EXPORTER_HPP
#define TELEMETRY_EXPORTER_HPP

#include "../core/Result.hpp"
#include "SystemMetrics.hpp"

namespace telemetry {

// IExporter naturally integrates right as an IMetricObserver resolving standard exports
class IExporter : public sys::IMetricObserver {
public:
    virtual ~IExporter() = default;
};

} // namespace telemetry

#endif // TELEMETRY_EXPORTER_HPP
