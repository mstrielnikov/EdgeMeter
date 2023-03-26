#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "Result.hpp"

namespace core {

namespace Constants {
    constexpr const char* ProgramName = "EdgeMeter";
#if __cplusplus >= 202002L
    constexpr const char* CppVersion = "C++20";
#elif __cplusplus >= 201703L
    constexpr const char* CppVersion = "C++17";
#else
    constexpr const char* CppVersion = "C++11/14";
#endif

    constexpr const char* DefaultGrpcHost = "127.0.0.1";
    constexpr int DefaultGrpcPort = 4317;

    constexpr const char* DefaultWsEndpoint = "ws://127.0.0.1:8080";

    constexpr const char* DefaultNatsHost = "127.0.0.1";
    constexpr int DefaultNatsPort = 4222;
    constexpr const char* DefaultNatsTopic = "telemetry.metrics.otlp";

    constexpr const char* DefaultHostname = "edge-device-1";
    constexpr const char* TlsVersion = "TLSv1.3";
}

struct Config {
    int server_port = 44301;
    
    std::string grpc_host = Constants::DefaultGrpcHost;
    int grpc_port = Constants::DefaultGrpcPort;

    std::string ws_endpoint = Constants::DefaultWsEndpoint;

    std::string nats_host = Constants::DefaultNatsHost;
    int nats_port = Constants::DefaultNatsPort;
    std::string nats_topic = Constants::DefaultNatsTopic;

    std::string program_name = Constants::ProgramName;
    std::string cpp_version = Constants::CppVersion;
    std::string tls_version = Constants::TlsVersion;
    std::string hostname = Constants::DefaultHostname;

    static core::Result<Config> Load(const std::string& path);
};

} // namespace core

#endif // CONFIG_HPP
