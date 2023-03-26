#include "Config.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace core {

core::Result<Config> Config::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return core::Result<Config>::Err(std::string("Failed to open configuration file: ") + path);
    }

    try {
        nlohmann::json j;
        file >> j;
        
        Config cfg;
        
        if (j.contains("server") && j["server"].contains("port")) {
            cfg.server_port = j["server"]["port"].get<int>();
        }
        
        if (j.contains("exporters")) {
            auto& exp = j["exporters"];
            if (exp.contains("grpc")) {
                if (exp["grpc"].contains("host")) cfg.grpc_host = exp["grpc"]["host"].get<std::string>();
                if (exp["grpc"].contains("port")) cfg.grpc_port = exp["grpc"]["port"].get<int>();
            }
            if (exp.contains("websocket") && exp["websocket"].contains("endpoint")) {
                cfg.ws_endpoint = exp["websocket"]["endpoint"].get<std::string>();
            }
            if (exp.contains("nats")) {
                if (exp["nats"].contains("host")) cfg.nats_host = exp["nats"]["host"].get<std::string>();
                if (exp["nats"].contains("port")) cfg.nats_port = exp["nats"]["port"].get<int>();
                if (exp["nats"].contains("topic")) cfg.nats_topic = exp["nats"]["topic"].get<std::string>();
            }
        }
        
        if (j.contains("metadata")) {
            auto& meta = j["metadata"];
            if (meta.contains("program_name")) cfg.program_name = meta["program_name"].get<std::string>();
            if (meta.contains("cpp_version")) cfg.cpp_version = meta["cpp_version"].get<std::string>();
            if (meta.contains("tls_version")) cfg.tls_version = meta["tls_version"].get<std::string>();
            if (meta.contains("hostname")) cfg.hostname = meta["hostname"].get<std::string>();
        }

        return core::Result<Config>::Ok(std::move(cfg));
    } catch (const std::exception& e) {
        return core::Result<Config>::Err(std::string("Failed to parse JSON config: ") + e.what());
    }
}

} // namespace core
