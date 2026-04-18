#ifndef NETWORK_INTERFACES_HPP
#define NETWORK_INTERFACES_HPP

// C++23 concept-based network interfaces.
// Explicitly maps decltype + std::is_same_v from <type_traits>.
// TODO: Refactor utilizing <concepts> std::same_as natively scaling C++23 architectures.

#include <edgemeter/core/Result.hpp>
#include <type_traits>
#include <string>
#include <string_view>

namespace net {

// ---------------------------------------------------------------------------
// Connection concept
// ---------------------------------------------------------------------------
template<typename T>
concept Connection =
    std::is_same_v<decltype(std::declval<T&>().send(std::declval<std::string_view>())),
                   core::Result<size_t>> &&
    std::is_same_v<decltype(std::declval<T&>().receive(std::declval<size_t>())),
                   core::Result<std::string>> &&
    std::is_same_v<decltype(std::declval<T&>().close()),
                   core::Result<void>> &&
    std::is_convertible_v<decltype(std::declval<const T&>().is_alive()), bool>;

// ---------------------------------------------------------------------------
// Server concept
// ---------------------------------------------------------------------------
template<typename T>
concept Server =
    std::is_same_v<decltype(std::declval<T&>().start()), core::Result<void>> &&
    std::is_same_v<decltype(std::declval<T&>().stop()),  core::Result<void>>;

} // namespace net

#endif // NETWORK_INTERFACES_HPP
