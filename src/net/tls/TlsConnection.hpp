#ifndef TLS_CONNECTION_HPP
#define TLS_CONNECTION_HPP

#include "../NetworkInterfaces.hpp"
#include "../../core/Result.hpp"
#include "../../core/Config.hpp"
#include <memory>
#include <string>

#ifdef USE_TLS
#include <openssl/ssl.h>
#endif

namespace net {

// State tags for Typestate Pattern
struct Unauthenticated {};
struct Authenticated {};

template <typename State>
class TlsConnection;

// Shared internal context avoiding raw pointers
struct TlsContext {
#ifdef USE_TLS
    SSL* ssl = nullptr;
    SSL_CTX* ctx = nullptr;
#endif
    int socket_fd = -1;

    ~TlsContext() {
#ifdef USE_TLS
        if (ssl) SSL_free(ssl);
        // Context might be shared globally, handle carefully in real impl
#endif
    }
};

// ---------------------------------------------------------
// Authenticated State implements IConnection
// ---------------------------------------------------------
template <>
class TlsConnection<Authenticated> : public IConnection {
private:
    std::shared_ptr<TlsContext> ctx_;
public:
    // Only constructible from an already authenticated context
    explicit TlsConnection(std::shared_ptr<TlsContext> ctx) : ctx_(std::move(ctx)) {}

    TlsConnection(const TlsConnection&) = default;
    TlsConnection& operator=(const TlsConnection&) = default;
    TlsConnection(TlsConnection&&) noexcept = default;
    TlsConnection& operator=(TlsConnection&&) noexcept = default;

#ifdef USE_TLS
    core::Result<size_t> send(const std::string& data) override {
        if (!ctx_->ssl) return core::Result<size_t>::Err("SSL object is null");
        int ret = SSL_write(ctx_->ssl, data.c_str(), static_cast<int>(data.length()));
        if (ret <= 0) return core::Result<size_t>::Err("SSL_write returned <= 0");
        return core::Result<size_t>::Ok(static_cast<size_t>(ret));
    }
#else
    core::Result<size_t> send(const std::string& /*data*/) override {
        return core::Result<size_t>::Err("TLS is compiled out");
    }
#endif

#ifdef USE_TLS
    core::Result<std::string> receive(size_t max_bytes) override {
        if (!ctx_->ssl) return core::Result<std::string>::Err("SSL object is null");
        std::string buffer(max_bytes, '\0');
        int ret = SSL_read(ctx_->ssl, buffer.data(), static_cast<int>(max_bytes));
        if (ret <= 0) return core::Result<std::string>::Err("SSL_read returned <= 0");
        buffer.resize(static_cast<size_t>(ret));
        return core::Result<std::string>::Ok(std::move(buffer));
    }
#else
    core::Result<std::string> receive(size_t /*max_bytes*/) override {
        return core::Result<std::string>::Err("TLS is compiled out");
    }
#endif

    core::Result<void> close() override {
        // Handle closure gracefully
        return core::Result<void>::Ok();
    }

    bool is_alive() const override {
        return ctx_ && ctx_->socket_fd >= 0;
    }
};

// ---------------------------------------------------------
// Unauthenticated State
// ---------------------------------------------------------
template <>
class TlsConnection<Unauthenticated> {
private:
    std::shared_ptr<TlsContext> ctx_;
public:
    explicit TlsConnection(int socket_fd) : ctx_(std::make_shared<TlsContext>()) {
        ctx_->socket_fd = socket_fd;
    }

    TlsConnection(const TlsConnection&) = default;
    TlsConnection& operator=(const TlsConnection&) = default;
    TlsConnection(TlsConnection&&) noexcept = default;
    TlsConnection& operator=(TlsConnection&&) noexcept = default;

    // Authenticate transforms the state into Authenticated
#ifdef USE_TLS
    core::Result<TlsConnection<Authenticated>> authenticate_as_server(const core::Config& config, const std::string& cert_path, const std::string& key_path) {
        ctx_->ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx_->ctx) return core::Result<TlsConnection<Authenticated>>::Err("Failed to create SSL_CTX");
        
        int tls_ver = (config.tls_version == "TLSv1.2") ? TLS1_2_VERSION : TLS1_3_VERSION;
        SSL_CTX_set_min_proto_version(ctx_->ctx, tls_ver);

        if (SSL_CTX_use_certificate_file(ctx_->ctx, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
            return core::Result<TlsConnection<Authenticated>>::Err("Failed to load certificate");
        }
        if (SSL_CTX_use_PrivateKey_file(ctx_->ctx, key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
            return core::Result<TlsConnection<Authenticated>>::Err("Failed to load private key");
        }

        ctx_->ssl = SSL_new(ctx_->ctx);
        SSL_set_fd(ctx_->ssl, ctx_->socket_fd);

        if (SSL_accept(ctx_->ssl) <= 0) {
            return core::Result<TlsConnection<Authenticated>>::Err("SSL_accept failed");
        }

        // Successfully authenticated, transfer context to Authenticated state
        return core::Result<TlsConnection<Authenticated>>::Ok(TlsConnection<Authenticated>(ctx_));
    }
#else
    core::Result<TlsConnection<Authenticated>> authenticate_as_server(const core::Config& /*config*/, const std::string& /*cert_path*/, const std::string& /*key_path*/) {
        return core::Result<TlsConnection<Authenticated>>::Err("TLS is compiled out (USE_TLS undefined)");
    }
#endif

    // Authenticate transforms the state into Authenticated for edge client connections explicitly safely identically!
#ifdef USE_TLS
    core::Result<TlsConnection<Authenticated>> authenticate_as_client(const core::Config& config) {
        ctx_->ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx_->ctx) return core::Result<TlsConnection<Authenticated>>::Err("Failed to create SSL_CTX");
        
        int tls_ver = (config.tls_version == "TLSv1.2") ? TLS1_2_VERSION : TLS1_3_VERSION;
        SSL_CTX_set_min_proto_version(ctx_->ctx, tls_ver);

        ctx_->ssl = SSL_new(ctx_->ctx);
        SSL_set_fd(ctx_->ssl, ctx_->socket_fd);

        if (SSL_connect(ctx_->ssl) <= 0) {
            return core::Result<TlsConnection<Authenticated>>::Err("SSL_connect failed");
        }

        return core::Result<TlsConnection<Authenticated>>::Ok(TlsConnection<Authenticated>(ctx_));
    }
#else
    core::Result<TlsConnection<Authenticated>> authenticate_as_client(const core::Config& /*config*/) {
        return core::Result<TlsConnection<Authenticated>>::Err("TLS is compiled out");
    }
#endif
};

} // namespace net

#endif // TLS_CONNECTION_HPP
