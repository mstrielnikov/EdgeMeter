#include <edgemeter/net/tls/TlsConnection.hpp>
#include <edgemeter/core/Logger.hpp>

namespace net {

// ---------------------------------------------------------------------------
// TLS version resolver
//
// Validates config.tls_version and maps it to an OpenSSL version constant.
//
// Policy:
//   SSLv2 / SSLv3 / TLSv1.0   → Err  (broken protocols, POODLE/DROWN, never allow)
//   TLSv1.1 → Warn + allow (deprecated, PCI-DSS non-compliant)
//   TLSv1.2         → Ok  (minimum recommended)
//   TLSv1.3         → Ok  (preferred)
//   <unknown>       → Err
// ---------------------------------------------------------------------------
static core::Result<int> resolve_tls_version(std::string_view ver) {
    if (ver == "SSLv2" || ver == "SSLv3" || ver == "TLSv1.0") {
        return std::unexpected{
            "TLS: Refusing broken protocol '" + std::string(ver) +
            "'. SSLv2/SSLv3/TLSv1.0 are vulnerable (POODLE / DROWN / Heartbleed / BEAST / CRIME / BREACH / LOGJAM / FREAK). Use TLSv1.2+."};
    }
    if (ver == "TLSv1.1") {
        core::Logger::Warn(
            "TLS: TLSv1.1 is deprecated and fails PCI-DSS compliance. "
            "Upgrade to TLSv1.2 or TLSv1.3.");
        return TLS1_1_VERSION;
    }
    if (ver == "TLSv1.2") {
        return TLS1_2_VERSION;
    }
    if (ver == "TLSv1.3" || ver.empty()) {
        return TLS1_3_VERSION;
    }
    core::Logger::Error(
        "TLS: Unknown tls_version '" + std::string(ver) + "'.");
    return std::unexpected{"TLS: Unrecognized tls_version '" + std::string(ver) + "'. Accepted: TLSv1.1 (deprecated), TLSv1.2, TLSv1.3."};
}

// ---------------------------------------------------------------------------
// Authenticated State Methods
// ---------------------------------------------------------------------------
core::Result<size_t> TlsConnection<Authenticated>::send(std::string_view data) {
    if (!ctx_->ssl) return std::unexpected{"SSL object is null"};
    int ret = SSL_write(ctx_->ssl, data.data(), static_cast<int>(data.length()));
    if (ret <= 0) return std::unexpected{"SSL_write returned <= 0"};
    return static_cast<size_t>(ret);
}

core::Result<std::string> TlsConnection<Authenticated>::receive(size_t max_bytes) {
    if (!ctx_->ssl) return std::unexpected{"SSL object is null"};
    std::string buffer(max_bytes, '\0');
    int ret = SSL_read(ctx_->ssl, buffer.data(), static_cast<int>(max_bytes));
    if (ret <= 0) return std::unexpected{"SSL_read returned <= 0"};
    buffer.resize(static_cast<size_t>(ret));
    return buffer;
}

// ---------------------------------------------------------------------------
// Unauthenticated State Methods
// ---------------------------------------------------------------------------
core::Result<TlsConnection<Authenticated>> TlsConnection<Unauthenticated>::authenticate_as_server(const core::Config& config) {
    ctx_->ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx_->ctx) return std::unexpected{"Failed to create SSL_CTX"};

    auto ver_res = resolve_tls_version(config.tls.version);
    if (!ver_res.has_value()) return std::unexpected{std::string_view(ver_res.error())};
    SSL_CTX_set_min_proto_version(ctx_->ctx, ver_res.value());

    if (SSL_CTX_use_certificate_file(ctx_->ctx, config.tls.cert_path.data(), SSL_FILETYPE_PEM) <= 0) {
        return std::unexpected{"Failed to load certificate"};
    }
    if (SSL_CTX_use_PrivateKey_file(ctx_->ctx, config.tls.key_path.data(), SSL_FILETYPE_PEM) <= 0) {
        return std::unexpected{"Failed to load private key"};
    }

    ctx_->ssl = SSL_new(ctx_->ctx);
    SSL_set_fd(ctx_->ssl, ctx_->socket_fd);

    if (SSL_accept(ctx_->ssl) <= 0) {
        return std::unexpected{"SSL_accept failed"};
    }

    return TlsConnection<Authenticated>(ctx_);
}

core::Result<TlsConnection<Authenticated>> TlsConnection<Unauthenticated>::authenticate_as_client(const core::Config& config) {
    ctx_->ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx_->ctx) return std::unexpected{"Failed to create SSL_CTX"};

    auto ver_res = resolve_tls_version(config.tls.version);
    if (!ver_res.has_value()) return std::unexpected{std::string_view(ver_res.error())};
    SSL_CTX_set_min_proto_version(ctx_->ctx, ver_res.value());

    ctx_->ssl = SSL_new(ctx_->ctx);
    SSL_set_fd(ctx_->ssl, ctx_->socket_fd);

    if (SSL_connect(ctx_->ssl) <= 0) {
        return std::unexpected{"SSL_connect failed"};
    }

    return TlsConnection<Authenticated>(ctx_);
}

} // namespace net
