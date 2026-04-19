# EdgeMeter

EdgeMeter is a lightweight **Universal Metric Connector SDK** implemented in C++20/23. It provides a C-ABI compatible interface to define and collect hardware metrics from edge devices (eBPF, GPU, FPGA and other hardware accelerators) and export them to observability sinks in multi-cloud, hybrid and edge environments in OTLP format supporting HTTP, NATS and WebSockets optionally with TLS.

See [examples](examples/)

## Dependencies

- **TLS**: `openssl` (optional)
- **Json**: `inja` + `nlohmann-json`
- **C++17**: `std::string_view`
- **C++20**: `std::format`, `std::span`, `std::stop_token`, `std::jthread`
- **C++23**: `std::expected`

## Key Features

- Built natively for highly distributed topologies (Cross-Cloud, CDNs, and Edge routing setups). Optionally enforces **OpenSSL/TLS 1.3** encryption utilizing the **Type-State Pattern**. Unauthenticated sockets resolve static bounds converting into exclusively authorized `TlsConnection<Authenticated>` memory footprints—meaning network reads/writes cannot execute prior to the compiler proving cryptographic authentication.
- Logically categorized codebase boundaries (`src/core`, `src/net`, `src/telemetry`) mapping interface encapsulations.
- Features a fallback-driven `<nlohmann::json>` configuration architecture abstracting ports, protocol TLS versions, and C++ macro inferences without requiring hardcoded topology literals
- Purely stripped archaic `try/catch` throw schemas globally utilizing generic `core::Result<T, E>` boundary wrappers preventing edge-crashing stack unwinding.

## Unified Pipeline Exports

The telemetry observer internally manages flexible network configurations emitting localized schema formats based on compile-time targets:

1. **Native NATS (`nats_otlp`)**: Broadcasts heavily optimized raw TCP payload interactions natively binding into Canonical message passing topologies (e.g., `telemetry.metrics.otlp`). Delivers extreme horizontal scalability universally matched for IoT and Edge routing paradigms.
2. **WebSockets (`ws_otlp`)**: Broadcasts generic JSON/OTLP frames natively mapping unmasked connections.
3. **HTTP (1.0/2.0/3.0) OTLP Exporter (`http_otlp`)**: Formulates strict, highly scalable TCP `POST /v1/metrics` payloads explicitly targeting HTTP/1.0, HTTP/2.0, and HTTP/3.0 (TODO) observability sinks natively. Purpose-built to securely bridge robust federated networking architectures.

## Compilation Scenarios & Topologies

The build target utilizes dynamic environment toggles structurally adapting exactly to constrained limits.

```bash
# General Bootstrap: Install dynamic JSON/Inja templates (Run once)
make vcpkg_bootstrap

# General Bootstrap: Dynamically generate ad-hoc 4096-bit RSA OpenSSL TLS configurations
make certs
```

Below are strictly supported compilation paths architected spanning microcontrollers toward standard datacenters:

### 1. Federated Networks (TLS + NATS + OTLP)

Ideally structured for multi-cloud deployments where distributed edge endpoints broadcast generic payloads spanning global HTTP, NATS, or WebSocket boundaries.

```bash
make http_otlp USE_TLS=1
make ws_otlp USE_TLS=1
make nats_otlp USE_TLS=1
```

### 2. Embedded & Constrained Environments (TCP/NATS)

Designed formally targeting microcontrollers, embedded sensors, or raw FPGA pipelines actively dropping heavy HTTP wrapping, streaming standard pure TCP signals directly via NATS.

```bash
make nats_otlp USE_TLS=0
```

### 3. Build Full Integration Pipeline

Compiles every example agent target strictly for unauthenticated generic routing configurations natively.

```bash
make all
# Run automated validation logic integrating against Go integration tests
make test
```

## Planned Features & Hardware Integration

EdgeMeter aims to dramaticall y expand its telemetry observational limits across physical topologies. Future hardware integrations will directly map localized hardware states explicitly into our C++23 OTLP bounds:

- **GPU Analytics**: Dedicated observers mapping NVML/ROCm boundaries directly into OTLP pipelines.
- **eBPF Tracing**: Deep Linux kernel observers wrapping raw syscall limits bypassing `/proc` bottlenecks for true high frequency granular OS networking metrics and WASM runtimes.
- **FPGA Integrations**: Formal abstractions routing physical logic arrays strictly into identical JSON bounds natively capturing gate metrics locally.
- Additional network protocols: QUIC (as modern HTTP/3 option with TLS 1.3), gRPC (HTTP/2 based option and OTLP native option)
- Streaming over Wireguard for true cross-cloud / hybrid support
- Additional telemetry sources: MQTT (as lightweight messaging protocol for IoT and edge devices), AMQP (as robust messaging protocol for enterprise applications)
- Alternative builds for cryptographic libraries: mbedtls, wolfssl targeting either QUIC or PQC TLS 1.3 support
