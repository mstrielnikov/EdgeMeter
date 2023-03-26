#ifndef WEBSOCKET_EXPORTER_HPP
#define WEBSOCKET_EXPORTER_HPP

#include "../TelemetryExporter.hpp"
#include "../../core/Config.hpp"
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/websocket.h>
#endif

namespace telemetry {

class WebSocketExporter : public IExporter {
public:
    explicit WebSocketExporter(core::Config config);
    virtual ~WebSocketExporter() = default;
    
    void Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) override;

private:
    core::Config config_;
    // If not emscripten, we would hold an IConnection / Websocket client context here
};

}

#endif // WEBSOCKET_EXPORTER_HPP
