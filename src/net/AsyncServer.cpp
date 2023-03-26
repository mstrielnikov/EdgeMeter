#include "AsyncServer.hpp"
#include "../core/Logger.hpp"

#ifndef __EMSCRIPTEN__
#include <thread>
#include <chrono>
#else
#include <emscripten.h>
#endif

namespace net {

AsyncServer::AsyncServer(uint16_t port) : port_(port) {}

AsyncServer::~AsyncServer() {
    stop();
}

core::Result<void> AsyncServer::start() {
    if (is_running_) {
        return core::Result<void>::Err("Server is already running");
    }
    is_running_ = true;

#ifndef __EMSCRIPTEN__
    std::thread([this]() {
        this->event_loop();
    }).detach();
#else
    // WebAssembly requires yielding to the browser event loop
    // Typically integrated via emscripten_set_main_loop
    LOG_INFO("[WASM] Listening loop initialized");
#endif

    return core::Result<void>::Ok();
}

core::Result<void> AsyncServer::stop() {
    is_running_ = false;
    return core::Result<void>::Ok();
}

void AsyncServer::event_loop() {
#ifndef __EMSCRIPTEN__
    while (is_running_) {
        // Native non-blocking poll loop placeholder
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif
}

} // namespace net
