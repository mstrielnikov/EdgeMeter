#ifndef RESULT_HPP
#define RESULT_HPP

#include <variant>
#include <stdexcept>
#include <string>

namespace core {

struct Empty {};

template <typename T>
struct Ok {
    T value;
};

template <typename E>
struct Err {
    E error;
};

template <typename T, typename E = std::string>
class Result {
private:
    std::variant<T, E> value_;
    bool is_ok_;

public:
    // Rust-like implicit constructors resolving ambiguity via distinct structural template types
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U>>>
    Result(Ok<U> ok) : value_(std::in_place_index<0>, std::move(ok.value)), is_ok_(true) {}

    template <typename U, typename = std::enable_if_t<std::is_constructible_v<E, U>>>
    Result(Err<U> err) : value_(std::in_place_index<1>, std::move(err.error)), is_ok_(false) {}

    // Legacy static methods for backward compatibility
    static Result<T, E> Ok(T value) {
        return Result<T, E>(std::move(value), true);
    }

    static Result<T, E> Err(E error) {
        return Result<T, E>(std::move(error), false, 0);
    }

    bool is_ok() const { return is_ok_; }
    bool is_err() const { return !is_ok_; }

    T& unwrap() {
        if (!is_ok_) {
            std::abort();
        }
        return std::get<0>(value_);
    }

    const T& unwrap() const {
        if (!is_ok_) {
            std::abort();
        }
        return std::get<0>(value_);
    }

    E& unwrap_err() {
        if (is_ok_) {
            std::abort();
        }
        return std::get<1>(value_);
    }

    const E& unwrap_err() const {
        if (is_ok_) {
            std::abort();
        }
        return std::get<1>(value_);
    }

protected:
    Result(T val, bool is_ok_flag) : value_(std::in_place_index<0>, std::move(val)), is_ok_(is_ok_flag) {}
    Result(E err, bool is_ok_flag, int /*dummy*/) : value_(std::in_place_index<1>, std::move(err)), is_ok_(is_ok_flag) {}
};

// Specialization for Result<void> using Empty
template <typename E>
class Result<void, E> : public Result<Empty, E> {
public:
    static Result<void, E> Ok() {
        return Result<void, E>(Empty{}, true);
    }

    // Implicitly adopt Rust-like Err<U> dynamically safely!
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<E, U>>>
    Result(Err<U> err) : Result<Empty, E>::Result(std::move(err.error), false, 0) {}

    static Result<void, E> Err(E error) {
        return Result<void, E>(std::move(error), false, 0);
    }
private:
    Result(Empty val, bool ok) : Result<Empty, E>::Result(val, ok) {}
    Result(E err, bool ok, int dummy) : Result<Empty, E>::Result(err, ok, dummy) {}
};

} // namespace core

#endif // RESULT_HPP
