#ifndef HARDWARE_COLLECTOR_HPP
#define HARDWARE_COLLECTOR_HPP

#include <edgemeter/core/Result.hpp>
#include <concepts>
#include <string_view>

namespace telemetry {

// ---------------------------------------------------------------------------
// HardwareCollector C++23 Concept
// ---------------------------------------------------------------------------
// C++23 structural concept bounding ingestion endpoints.
// A valid hardware collector must implement `initialize` mapping generic
// configuration bounds safely, and an active `poll` function yielding its
// observed metrics into the generic metric framework.

template <typename T, typename Observer>
concept HardwareCollector = requires(T t, Observer& obs, const std::string_view& path) {
    { t.initialize(path) } -> std::same_as<core::Result<void>>;
    { t.poll(obs) } -> std::same_as<void>;
};

} // namespace telemetry

#endif // HARDWARE_COLLECTOR_HPP
