#ifndef NETWORK_INTERFACES_HPP
#define NETWORK_INTERFACES_HPP

#include "../core/Result.hpp"
#include <cstdint>
#include <string>

namespace net {

class IConnection {
public:
    virtual ~IConnection() = default;
    
    // Core operations returning Result instead of throwing
    virtual core::Result<size_t> send(const std::string& data) = 0;
    virtual core::Result<std::string> receive(size_t max_bytes) = 0;
    virtual core::Result<void> close() = 0;
    
    virtual bool is_alive() const = 0;
};

class IServer {
public:
    virtual ~IServer() = default;
    virtual core::Result<void> start() = 0;
    virtual core::Result<void> stop() = 0;
};

} // namespace net

#endif // NETWORK_INTERFACES_HPP
