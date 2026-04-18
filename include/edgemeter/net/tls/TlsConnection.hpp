#ifndef TLS_CONNECTION_HPP
#define TLS_CONNECTION_HPP

#include <edgemeter/core/Result.hpp>
#include <edgemeter/core/Config.hpp>
#include <type_traits>
#include <memory>
#include <string>
#include <openssl/ssl.h>

namespace net {

// ---------------------------------------------------------
// Typestate Definitions
// ---------------------------------------------------------
struct Unauthenticated {};
struct Authenticated {};

template<typename T>
concept TlsState = std::is_same_v<T, Unauthenticated> || std::is_same_v<T, Authenticated>;

// Shared internal context avoiding raw pointers
struct TlsContext {
    SSL* ssl = nullptr;
    SSL_CTX* ctx = nullptr;
    int socket_fd = -1;

    ~TlsContext() {
        if (ssl) SSL_free(ssl);
        if (ctx) SSL_CTX_free(ctx);
    }
};

template <typename State>
class TlsConnection;

// ---------------------------------------------------------
// Authenticated State — satisfies net::Connection concept
// ---------------------------------------------------------
template <>
class TlsConnection<Authenticated> {
private:
    std::shared_ptr<TlsContext> ctx_;
public:
    explicit TlsConnection(std::shared_ptr<TlsContext> ctx) : ctx_(std::move(ctx)) {}

    TlsConnection(const TlsConnection&) = default;
    TlsConnection& operator=(const TlsConnection&) = default;
    TlsConnection(TlsConnection&&) noexcept = default;
    TlsConnection& operator=(TlsConnection&&) noexcept = default;

    core::Result<size_t> send(std::string_view data);
    core::Result<std::string> receive(size_t max_bytes);

    core::Result<void> close() {
        return {};
    }

    bool is_alive() const {
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

    core::Result<TlsConnection<Authenticated>> authenticate_as_server(const core::Config& config);
    core::Result<TlsConnection<Authenticated>> authenticate_as_client(const core::Config& config);
};

} // namespace net

#endif // TLS_CONNECTION_HPP
