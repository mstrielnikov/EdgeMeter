#include <edgemeter/net/AsyncServer.hpp>
#include <edgemeter/core/Logger.hpp>

#include <thread>
#include <chrono>

namespace net {

AsyncServer::AsyncServer(uint16_t port) {
    (void)port;
}

AsyncServer::~AsyncServer() {
    stop();
}

core::Result<void> AsyncServer::start() {
    if (loop_thread_.joinable()) {
        return std::unexpected{"Server is already running"};
    }
    loop_thread_ = std::jthread([this](std::stop_token stoken) {
        this->event_loop(stoken);
    });

    return {};
}

core::Result<void> AsyncServer::stop() {
    bool stopped = false;
    if (loop_thread_.joinable()) {
        loop_thread_.request_stop();
        loop_thread_.join();
        stopped = true;
    }

    if (stopped || !active_connections_.empty()) {
        // Drain all active connections via variant visit — no vtable dispatch
        for (auto& conn : active_connections_) {
            std::visit([](auto& c) {
                if (c.is_alive()) c.close();
            }, conn);
        }
        active_connections_.clear();
        core::Logger::Info("AsyncServer gracefully stopped and connections drained.");
    }
    return {};
}

void AsyncServer::event_loop(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        // Native non-blocking poll loop placeholder
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace net
