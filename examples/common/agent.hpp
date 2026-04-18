#pragma once

// Shared bootstrap utilities for EdgeMeter example programs.
// Concepts-based: run_metric_loop<O> is constrained by sys::MetricObserver.

#include <edgemeter/core/Logger.hpp>
#include <edgemeter/core/Config.hpp>
#include <edgemeter/telemetry/SystemMetrics.hpp>

#include <atomic>
#include <csignal>
#include <signal.h>

#include <thread>
#include <chrono>

namespace edgemeter {

// ─── Signal Handling Cycle ────────────────────────────────────────────────────
//
// Design: a two-phase, async-signal-safe pattern.
//
//  Phase 1 — Signal delivery (kernel → signal_handler):
//    The OS interrupts the process and calls signal_handler() on any thread.
//    POSIX mandates that only a restricted set of functions are safe to call
//    here (the "async-signal-safe" list). Notably, stdio (printf, std::cout),
//    memory allocation (new/malloc), and mutex operations are ALL unsafe.
//    Therefore the handler ONLY writes to std::atomic values — a lock-free,
//    async-signal-safe primitive guaranteed by the C++ memory model.
//
//  Phase 2 — Drain (main loop → run_metric_loop):
//    On every iteration of the main metric loop, g_signal_received is checked
//    and exchanged atomically. Any side-effects that require stdio or heap
//    (e.g. Logger::Info, SetLevel) are executed here, safely on the main thread.
//
//  sigaction vs std::signal:
//    SA_RESTART causes the kernel to automatically restart any interrupted
//    blocking syscall (e.g. sleep, read) rather than returning EINTR, which
//    would otherwise break std::this_thread::sleep_for silently.
// ─────────────────────────────────────────────────────────────────────────────

// Phase 1 state: written only from signal_handler, read from the main loop.
inline std::atomic<bool> g_running{true};       // cleared on SIGINT/SIGTERM
inline std::atomic<int>  g_signal_received{0};  // carries last non-terminal signal

// Phase 1: async-signal-safe handler — only atomics, no stdio, no heap.
inline void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        // Set the loop exit flag; the main loop will break on next iteration.
        g_running.store(false, std::memory_order_relaxed);
    } else if (signum == SIGHUP) {
        // Store the signal number; Phase 2 will perform the actual log-level
        // toggle and logging once back on the main thread.
        g_signal_received.store(SIGHUP, std::memory_order_relaxed);
    }
}

// Register handlers via sigaction (POSIX).
// SA_RESTART ensures interrupted syscalls are retried automatically instead
// of failing with EINTR (which would silently break sleep_for).
inline void setup_signals() {
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sa.sa_flags   = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, nullptr);  // Ctrl+C / kill default
    sigaction(SIGTERM, &sa, nullptr);  // systemd stop, graceful shutdown
    sigaction(SIGHUP,  &sa, nullptr);  // log-level toggle (send with: kill -HUP <pid>)
}

// Configure log level from the config struct (in-memory only, no file I/O).
inline void setup_logging(const core::Config& config) {
    core::LogLevel level = core::LogLevel::INFO;
    if (config.app.log_level == "DEBUG") level = core::LogLevel::DEBUG;
    if (config.app.log_level == "ERROR") level = core::LogLevel::ERROR;
    core::Logger::SetLevel(level);
}

// Phase 2: metric loop — drains pending signals and performs safe side-effects.
// Constrained by MetricObserver concept — accepts any conforming observer.
template<sys::MetricObserver O>
inline void run_metric_loop(O& observer) {
    while (g_running) {
        // Exchange atomically: read the pending signal and reset to 0 in one op.
        // This ensures a concurrent SIGHUP is never lost or double-processed.
        int sig = g_signal_received.exchange(0, std::memory_order_relaxed);
        if (sig == SIGHUP) {
            // Safe to call Logger here — we are on the main thread, not in a handler.
            if (core::Logger::GetLevel() == core::LogLevel::DEBUG) {
                core::Logger::SetLevel(core::LogLevel::INFO);
                core::Logger::Info("SIGHUP: Log level shifted to INFO.");
            } else {
                core::Logger::SetLevel(core::LogLevel::DEBUG);
                core::Logger::Info("SIGHUP: Log level shifted to DEBUG.");
            }
        }

        auto res = sys::CollectProcessMetrics(observer);
        if (!res.has_value()) {
            core::Logger::Error("Failed to collect metrics: " + std::string(res.error()));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Reached only after g_running is cleared by SIGINT/SIGTERM.
    // Safe to log here — we are back in normal thread execution context.
    core::Logger::Info("Shutdown signal received, initiating graceful exit...");
}

} // namespace edgemeter
