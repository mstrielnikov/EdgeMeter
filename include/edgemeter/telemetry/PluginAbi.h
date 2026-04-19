#ifndef EDGEMETER_ABI_H
#define EDGEMETER_ABI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// EdgeMeter Plugin ABI
// ---------------------------------------------------------------------------
// A pure C interface enabling metric injections directly spanning
// cross-language environments (Rust FFI, Go CGO, Python ctypes, or kernel eBPF).

typedef enum {
    EDGEMETER_METRIC_GAUGE = 0,
    EDGEMETER_METRIC_SUM = 1
} edgemeter_metric_type_t;

// Callback given to plugins allowing them to inject metrics into the internal C++ observer loop.
typedef void (*edgemeter_observe_fn)(const char* name, double value, edgemeter_metric_type_t type, void* user_data);

// ---------------------------------------------------------------------------
// Expected Target Exports
// ---------------------------------------------------------------------------
// Any dynamically loaded `.so` shared object must explicitly export:
//    int  edgemeter_plugin_init(edgemeter_observe_fn callback, void* user_data);
//    void edgemeter_plugin_poll();
//    void edgemeter_plugin_shutdown();

typedef int  (*edgemeter_plugin_init_fn)(edgemeter_observe_fn callback, void* user_data);
typedef void (*edgemeter_plugin_poll_fn)();
typedef void (*edgemeter_plugin_shutdown_fn)();

#ifdef __cplusplus
}
#endif

#endif // EDGEMETER_ABI_H
