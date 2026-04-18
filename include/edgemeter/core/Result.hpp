#ifndef RESULT_HPP
#define RESULT_HPP

// core::Result<T, E> is an alias for the native C++23 std::expected.

#include <expected>
#include <string_view>

namespace core {

// ---------------------------------------------------------------------------
// Result<T, E>  
// ---------------------------------------------------------------------------
template<typename T, typename E = std::string_view>
using Result = std::expected<T, E>;

} // namespace core

#endif // RESULT_HPP
