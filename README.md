# EdgeMeter

A modern, ultra-lightweight **Universal Metric Connector** written in **C++20**.

## Purpose

EdgeMeter is purpose-built to securely extract and normalize observability metrics across constrained hardware boundaries. It is designed to operate seamlessly on embedded microcontrollers, edge routing nodes, hardware accelerators (GPUs), and plugged FPGAs. It collects physical parameters and emits **OpenTelemetry (OTLP)** JSON metrics destined for observability sinks.

Operating independently of Protocol Buffer SDKs, it places an explicit structural focus on portability, minimal memory footprints, continuous cross-cloud mobility, and active memory safety boundaries.

## Key Features

- **Federated & Zero-Trust Ready (TLS 1.3)**: Built natively for highly distributed topologies (Cross-Cloud, CDNs, and Edge routing setups). EdgeMeter optionally enforces **OpenSSL/TLS 1.3** encryption utilizing the **Type-State Pattern**. Unauthenticated sockets resolve static bounds converting into exclusively authorized `TlsConnection<Authenticated>` memory footprints—meaning network reads/writes cannot execute prior to the compiler proving cryptographic authentication.
- **Strict Decoupled Subsystems**: Logically categorized codebase boundaries (`src/core`, `src/net`, `src/telemetry`) mapping clean interface encapsulations organically.
- **Dynamic JSON Configuration Layer**: Features a robust, fallback-driven `<nlohmann::json>` configuration architecture abstracting ports, protocol TLS versions, and C++ macro inferences gracefully strictly without requiring hardcoded topology literals!
- **Rust-Inspired Error Handling**: Purely stripped archaic `try/catch` throw schemas globally utilizing generic monadic `core::Result<T, E>` boundary wrappers preventing edge-crashing stack unwinding.
- **Universal Portability (WASM)**: Architected utilizing standard generic constraints compiling interchangeably perfectly against low-power Linux Daemons or shifted transparently into WebAssembly (`__EMSCRIPTEN__`) contexts statically easily securely cleanly structurally!
- **Dynamic Canonical Templating**: Erased large monolithic SDK frameworks dynamically rendering comprehensively standardized OTLP string payloads linearly utilizing extremely fast overhead `Inja` mapping engines seamlessly reliably!

## Unified Pipeline Exports

The telemetry observer internally manages flexible network configurations selectively emitting localized schema formats passively securely based exclusively on compile-time targets:

1. **Native NATS PubSub (`USE_NATS=1`)**: Broadcasts heavily optimized raw TCP payload interactions natively binding into Canonical message passing topologies (e.g., `telemetry.metrics.otlp`). Delivers extreme horizontal scalability universally matched for IoT and Edge routing paradigms.
2. **WebSockets (`USE_WEBSOCKETS=1`)**: Broadcasts generic JSON frames securely parsed as Canonical OTLP schema objects. Operates identically securely natively mapping unmasked connections internally or shifting perfectly natively across native Browser WASM deployments dynamically.
3. **HTTP/gRPC OTLP (`USE_GRPC=1`)**: Rapidly formulates internal TCP socket `POST /v1/metrics` payload interactions reliably streaming standard JSON array chunks explicitly natively without requiring extensive library links appropriately efficiently!

## Compilation Scenarios & Topologies

The build target utilizes dynamic environment toggles structurally adapting exactly to constrained limits.

```bash
# General Bootstrap: Install dynamic JSON/Inja templates (Run explicitly once natively)
make vcpkg_bootstrap

# General Bootstrap: Dynamically generate ad-hoc 4096-bit RSA OpenSSL TLS configurations
make certs
```

Below are strictly supported compilation paths architected spanning microcontrollers toward standard federated datacenters:

### 1. Federated Networks & Zero-Trust (TLS + NATS + OTLP)

Ideally structured for multi-cloud deployments where distributed edge endpoints actively broadcast cryptographically verified generic payloads safely gracefully spanning global boundaries.

```bash
make USE_NATS=1 USE_GRPC=1 USE_TLS=1
```

### 2. Embedded & Constrained Environments (NATS Only)

Designed formally targeting microcontrollers, embedded sensors, or raw FPGA pipelines actively dropping explicit heavy HTTP/gRPC wrapping securely streaming standard pure TCP signals directly.

```bash
make USE_NATS=1 USE_GRPC=0 USE_WEBSOCKETS=0 USE_TLS=0
```

### 3. Edge CDN & WebAssembly Runtimes (WASM via WebSockets & NATS)

Shifts the identical observability metrics mapping framework directly inside memory constrained browser pipelines or wasm runtimes. Emscripten's native POSIX bounds bridge NATS unencrypted pipelines logically accurately cleanly.

```bash
make CXX=emcc USE_WEBSOCKETS=1 USE_NATS=1 USE_GRPC=0 USE_TLS=0 # either WebSockets or NATS (without TLS) works natively in WASM
```

### 4. Standard Omnipresent Linux Daemon (All Toggles Active)

Binds every canonical framework simultaneously routing generic identical observer limits exactly!

```bash
make USE_NATS=1 USE_GRPC=1 USE_WEBSOCKETS=1 USE_TLS=0
# Optional: Register explicitly alongside systemd limits!
sudo make service
```

## Planned Features & Hardware Integration

EdgeMeter aims to dramatically expand its telemetry observational limits across physical topologies. Future hardware integrations will directly map localized hardware states explicitly into our C++20 OTLP bounds:

- **GPU Analytics**: Dedicated observers mapping NVML boundaries directly into OTLP pipelines.
- **eBPF Tracing**: Deep Linux kernel observers wrapping raw syscall limits bypassing `/proc` bottlenecks for true high frequency granular OS networking metrics.
- **FPGA Integrations**: Formal abstractions routing physical logic arrays strictly into identical JSON bounds natively capturing gate metrics locally.
