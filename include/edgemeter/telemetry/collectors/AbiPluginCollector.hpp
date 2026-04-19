#ifndef ABI_PLUGIN_COLLECTOR_HPP
#define ABI_PLUGIN_COLLECTOR_HPP

#if __has_include(<dlfcn.h>)

#include <edgemeter/telemetry/HardwareCollector.hpp>
#include <edgemeter/telemetry/PluginAbi.h>
#include <edgemeter/telemetry/SystemMetrics.hpp>
#include <edgemeter/core/Logger.hpp>
#include <dlfcn.h>
#include <string_view>

namespace telemetry {
namespace collectors {

// ---------------------------------------------------------------------------
// AbiPluginCollector
// ---------------------------------------------------------------------------
// Maps natively into the 'extern "C"' shared object boundary utilizing POSIX
// dlfcn. Automatically delegates memory polling over safe C ABI functions.

template <typename Observer>
class AbiPluginCollector {
public:
    explicit AbiPluginCollector(Observer& obs) 
        : obs_(obs), handle_(nullptr), poll_fn_(nullptr), shutdown_fn_(nullptr) {}

    ~AbiPluginCollector() {
        if (shutdown_fn_) shutdown_fn_();
        if (handle_) dlclose(handle_);
    }

    core::Result<void> initialize(std::string_view plugin_path) {
        handle_ = dlopen(plugin_path.data(), RTLD_NOW | RTLD_LOCAL);
        if (!handle_) {
            core::Logger::Error(std::string("Plugin Load Error: ") + dlerror());
            return std::unexpected("Failed to dynamically link plugin");
        }

        auto init_fn = reinterpret_cast<edgemeter_plugin_init_fn>(dlsym(handle_, "edgemeter_plugin_init"));
        poll_fn_ = reinterpret_cast<edgemeter_plugin_poll_fn>(dlsym(handle_, "edgemeter_plugin_poll"));
        shutdown_fn_ = reinterpret_cast<edgemeter_plugin_shutdown_fn>(dlsym(handle_, "edgemeter_plugin_shutdown"));

        if (!init_fn || !poll_fn_) {
            dlclose(handle_);
            handle_ = nullptr;
            return std::unexpected("Shared library missing edgemeter_plugin_init or edgemeter_plugin_poll exports");
        }

        int res = init_fn(&c_callback, this);
        if (res != 0) {
            dlclose(handle_);
            handle_ = nullptr;
            return std::unexpected("edgemeter_plugin_init returned non-zero error state");
        }

        return {};
    }

    void poll(Observer&) {
        if (poll_fn_) {
            poll_fn_();
        }
    }

private:
    Observer& obs_;
    void* handle_;
    edgemeter_plugin_poll_fn poll_fn_;
    edgemeter_plugin_shutdown_fn shutdown_fn_;

    // Static dispatch routing the C ABI callback seamlessly back into C++ Observer constraints
    static void c_callback(const char* name, double value, edgemeter_metric_type_t /*type*/, void* user_data) {
        auto* self = static_cast<AbiPluginCollector*>(user_data);
        sys::Attribute attrs[] = { {"source", "abi_plugin"} };
        self->obs_.Observe(std::string_view(name), value, std::span<const sys::Attribute>(attrs, 1));
    }
};

} // namespace collectors
} // namespace telemetry

#endif // __has_include(<dlfcn.h>)
#endif // ABI_PLUGIN_COLLECTOR_HPP
