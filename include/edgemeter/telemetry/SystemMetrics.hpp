#ifndef SYSTEM_METRICS_HPP
#define SYSTEM_METRICS_HPP

// C++23 concept-based metric observer natively bounding generic typestates.
// Uses decltype + std::is_same_v from <type_traits> cleanly validating evaluations.

#include <edgemeter/core/Result.hpp>
#include <span>
#include <type_traits>
#include <string_view>
#include <string>

#include <sys/resource.h>
#include <sys/time.h>

namespace sys {

struct Attribute {
    std::string_view key;
    std::string_view val;
};

// ---------------------------------------------------------------------------
// MetricObserver concept (GCC 9 compatible via type_traits)
// ---------------------------------------------------------------------------
template<typename T>
concept MetricObserver = std::is_same_v<
    decltype(std::declval<T&>().Observe(
        std::declval<std::string_view>(),
        std::declval<double>(),
        std::declval<std::span<const Attribute>>())),
    void>;

// ---------------------------------------------------------------------------
// CollectProcessMetrics — template constrained by MetricObserver.
// Inline so any observer type can be instantiated at the call site.
// ---------------------------------------------------------------------------
template<MetricObserver O>
inline core::Result<void> CollectProcessMetrics(O& observer) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        double max_rss_value = static_cast<double>(usage.ru_maxrss) * 1024.0;
        Attribute mem_attrs[] = { {"process.type", "native"} };
        observer.Observe("process.memory.usage", max_rss_value, std::span<const Attribute>(mem_attrs, 1));
    } else {
        return std::unexpected{"Platform failed getrusage polling"};
    }
    return {};
}

} // namespace sys

#endif // SYSTEM_METRICS_HPP
