#include <edgemeter/telemetry/PluginAbi.h>

static edgemeter_observe_fn g_callback = nullptr;
static void* g_user_data = nullptr;

extern "C" {

int edgemeter_plugin_init(edgemeter_observe_fn callback, void* user_data) {
    if (!callback) return -1;
    g_callback = callback;
    g_user_data = user_data;
    return 0;
}

void edgemeter_plugin_poll() {
    if (g_callback) {
        g_callback("plugin.external.mock", 99.9, EDGEMETER_METRIC_GAUGE, g_user_data);
    }
}

void edgemeter_plugin_shutdown() {
    g_callback = nullptr;
}

}
