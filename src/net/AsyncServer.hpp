#ifndef ASYNC_SERVER_HPP
#define ASYNC_SERVER_HPP

#include "NetworkInterfaces.hpp"
#include "../core/Result.hpp"
#include <vector>
#include <memory>
#include <atomic>

// Example of modular implementation for Server socket loop
namespace net {

class AsyncServer : public IServer {
private:
    std::vector<std::unique_ptr<IConnection>> active_connections_;
    std::atomic<bool> is_running_{false};
    uint16_t port_;

public:
    explicit AsyncServer(uint16_t port);
    virtual ~AsyncServer();

    core::Result<void> start() override;
    core::Result<void> stop() override;

private:
    void event_loop();
};

} // namespace net

#endif // ASYNC_SERVER_HPP
