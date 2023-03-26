#ifndef SYSTEM_METRICS_HPP
#define SYSTEM_METRICS_HPP

#include "../core/Result.hpp"
#include <map>
#include <string>

namespace sys {

// Canonical OTEL-style Observer Interface
class IMetricObserver {
public:
    virtual ~IMetricObserver() = default;
    
    // Abstract observer callback matching OpenTelemetry meter gauge records
    virtual void Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) = 0;
};

// Dispatch metrics back dynamically to the observer reference without parsing filesystem explicitly
core::Result<void> CollectProcessMetrics(IMetricObserver* observer);

} // namespace sys

#endif // SYSTEM_METRICS_HPP
