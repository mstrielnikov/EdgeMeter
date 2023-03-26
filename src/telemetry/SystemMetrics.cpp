#include "SystemMetrics.hpp"
#include <sys/resource.h>
#include <sys/time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace sys {

core::Result<void> CollectProcessMetrics(IMetricObserver* observer) {
    if (!observer) return core::Result<void>::Err("Observer is inherently missing");

    // Retrieve Process / Box resources robustly mimicking canonical OTEL host metrics
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        
        // CPU Time measurements
        double user_time = usage.ru_utime.tv_sec + (usage.ru_utime.tv_usec / 1000000.0);
        double sys_time = usage.ru_stime.tv_sec + (usage.ru_stime.tv_usec / 1000000.0);
        
        observer->Observe("process.cpu.time", user_time + sys_time, {{"state", "total"}});
        
        // Resident Set Size (Memory Consumption)
        // Linux reports in KB, maxrss can vary by target, abstract as process bound:
        double max_rss_value = static_cast<double>(usage.ru_maxrss);
        observer->Observe("process.memory.usage", max_rss_value, {{"type", "max_rss"}});
        
    } else {
        return core::Result<void>::Err("Platform failed getrusage polling");
    }

#ifdef __EMSCRIPTEN__
    // Direct WASM box bounding captures using explicit emscripten host mappings
    double heap_size = static_cast<double>(emscripten_get_heap_size());
    observer->Observe("wasm.box.heap_size", heap_size, {{"unit", "bytes"}});
#endif

    return core::Result<void>::Ok();
}

} // namespace sys
