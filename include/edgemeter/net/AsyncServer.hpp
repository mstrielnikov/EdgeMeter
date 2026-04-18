#ifndef ASYNC_SERVER_HPP
#define ASYNC_SERVER_HPP

// AsyncServer — concept-constrained, variant-based connection store.
// Satisfies the net::Server concept structurally (no IServer inheritance).
// active_connections_ stores concrete connection types in a std::variant —
// zero vtable overhead vs the old unique_ptr<IConnection> approach.

#include <edgemeter/net/NetworkInterfaces.hpp>
#include <edgemeter/core/Result.hpp>
#include <edgemeter/net/tls/TlsConnection.hpp>
#include <variant>
#include <vector>
#include <memory>
#include <atomic>

#include <thread>
#include <stop_token>

namespace net {

// All concrete connection types supported by the server.
// Extend this variant when adding new transport backends.
using AnyConnection = std::variant<TlsConnection<Authenticated>>;

// Verify AsyncServer satisfies net::Server at compile time (enforced below).
class AsyncServer {
private:
    std::vector<AnyConnection> active_connections_;
    std::jthread loop_thread_;

public:
    explicit AsyncServer(uint16_t port);
    ~AsyncServer();

    core::Result<void> start();
    core::Result<void> stop();

private:
    void event_loop(std::stop_token stoken);
};

// Static assertion: AsyncServer must satisfy the Server concept.
static_assert(Server<AsyncServer>, "AsyncServer must satisfy net::Server concept");

} // namespace net

#endif // ASYNC_SERVER_HPP
