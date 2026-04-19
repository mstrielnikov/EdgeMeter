#ifndef NVML_COLLECTOR_HPP
#define NVML_COLLECTOR_HPP

#include <edgemeter/telemetry/HardwareCollector.hpp>
#include <edgemeter/telemetry/SystemMetrics.hpp>
#include <edgemeter/core/Logger.hpp>
#include <string_view>
#include <string>

namespace telemetry {
namespace collectors {

// ---------------------------------------------------------------------------
// NvmlCollector (Mocked Demonstration)
// ---------------------------------------------------------------------------
// Satisfies the HardwareCollector constraint. Simulates natively
// hooking into physical GPU hardware pipelines

template <typename Observer>
class NvmlCollector {
public:
    explicit NvmlCollector(Observer& obs) : obs_(obs) {}

    core::Result<void> initialize(std::string_view cfg_path) {
        // Mocking dlopen("libnvidia-ml.so.1", RTLD_NOW) and nvmlInit_v2()
        core::Logger::Info("Bound Mocked NVML Nvidia pipeline targeting: " + std::string(cfg_path));
        return {};
    }

    void poll(Observer&) {
        // Hardware interception mapping directly natively to OTLP JSON pipelines safely
        sys::Attribute gpu_attrs[] = { {"gpu.id", "0"}, {"gpu.model", "Mocked_H100"} };
        
        obs_.Observe("device.gpu.temperature", 62.4, std::span<const sys::Attribute>(gpu_attrs, 2));
        obs_.Observe("device.gpu.memory.used", 1240960.0, std::span<const sys::Attribute>(gpu_attrs, 2));
    }

private:
    Observer& obs_;
};

} // namespace collectors
} // namespace telemetry

#endif // NVML_COLLECTOR_HPP
