#ifndef TELEMETRY_EXPORTER_HPP
#define TELEMETRY_EXPORTER_HPP

// C++23 concept-based exporter interface.
// Exporter refines MetricObserver — any type satisfying MetricObserver
// also satisfies Exporter. No virtual dispatch, no vtable, no inheritance.

#include <edgemeter/telemetry/SystemMetrics.hpp>

namespace telemetry {

// An Exporter is any MetricObserver.
// The concept can be extended with additional requirements (e.g. flush())
// as the SDK grows.
template<typename T>
concept Exporter = sys::MetricObserver<T>;

// ---------------------------------------------------------------------------
// Transport mode typestates for compile-time TLS dispatch.
//
// Plain  — plain TCP; no TLS handshake performed.
// Secure — TLS wrapping; authenticate_as_client() called before sending.
//
// These are defined here (the single canonical location) and included by
// all exporter headers. The previous per-header #ifndef guard macros are gone.
// ---------------------------------------------------------------------------
struct Plain  {};
struct Secure {};

template<typename Mode>
concept TransportMode = std::is_same_v<Mode, Plain> || std::is_same_v<Mode, Secure>;

} // namespace telemetry

#endif // TELEMETRY_EXPORTER_HPP
