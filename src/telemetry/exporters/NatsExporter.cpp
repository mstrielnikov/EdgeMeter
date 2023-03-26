#include "NatsExporter.hpp"
#include "../../core/Logger.hpp"
#include "../OtlpTemplateEngine.hpp"

#ifdef USE_NATS
#ifndef __EMSCRIPTEN__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../../net/tls/TlsConnection.hpp"
#endif
#endif

namespace telemetry {

NatsExporter::NatsExporter(core::Config config) 
    : config_(std::move(config)) {}

void NatsExporter::Observe(const std::string& name, double value, const std::map<std::string, std::string>& attributes) {
    std::string payload = OtlpTemplateEngine::render_payload(config_, name, value, attributes);
    if (payload.empty()) {
        LOG_ERROR("NATS Exporter aborted: Generated payload is empty for metric '" + name + "'");
        return; 
    }
    LOG_DEBUG("NATS Emitting Formatted Canonical OTLP: \n" + payload);

#ifdef USE_NATS
#ifndef __EMSCRIPTEN__
    // Raw native integration targeting NATS servers safely dynamically without nats-c libs!
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(config_.nats_port);
        inet_pton(AF_INET, config_.nats_host.c_str(), &serv_addr.sin_addr);
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
#ifdef USE_TLS
            net::TlsConnection<net::Unauthenticated> unauth_conn(sock);
            auto auth_res = unauth_conn.authenticate_as_client(config_);
            if (auth_res.is_ok()) {
                auto& tls_conn = auth_res.unwrap();
                constexpr std::string_view connect_req = "CONNECT {\"verbose\":false,\"pedantic\":false,\"tls_required\":true}\r\n";
                tls_conn.send(std::string(connect_req));
                std::string pub_req = "PUB " + config_.nats_topic + " " + std::to_string(payload.size()) + "\r\n" + payload + "\r\n";
                tls_conn.send(pub_req);
            } else {
                LOG_ERROR("Failed to establish NATS TLS Typestate: " + auth_res.unwrap_err());
            }
#else
            // Emit lightweight Protocol Connect
            constexpr std::string_view connect_req = "CONNECT {\"verbose\":false,\"pedantic\":false,\"tls_required\":false}\r\n";
            send(sock, connect_req.data(), connect_req.size(), 0);
            
            // Emit PUB string length frames securely cleanly
            std::string pub_req = "PUB " + config_.nats_topic + " " + std::to_string(payload.size()) + "\r\n" + payload + "\r\n";
            send(sock, pub_req.c_str(), pub_req.size(), 0);
#endif
        }
        close(sock);
    }
#endif
#endif
}

} // namespace telemetry
