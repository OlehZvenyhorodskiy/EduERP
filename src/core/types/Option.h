#pragma once

#include <optional>
#include <stdexcept>

namespace eduerp::core {

/**
 * @brief Wrapper around std::optional with ergonomic access methods.
 *        Used to express "might not have a value" without null pointers.
 */
template <typename T>
class Option {
private:
    std::optional<T> m_value;

public:
    Option() = default;
    Option(T val) : m_value(std::move(val)) {}
    Option(std::nullopt_t) : m_value(std::nullopt) {}

    static Option<T> some(T val) { return Option<T>(std::move(val)); }
    static Option<T> none() { return Option<T>(std::nullopt); }

    bool hasValue() const { return m_value.has_value(); }
    explicit operator bool() const { return hasValue(); }

    T& value() {
        if (!hasValue()) {
            throw std::runtime_error("Attempted to unwrap an empty Option");
        }
        return *m_value;
    }

    const T& value() const {
        if (!hasValue()) {
            throw std::runtime_error("Attempted to unwrap an empty Option");
        }
        return *m_value;
    }

    T valueOr(T fallback) const {
        return hasValue() ? *m_value : std::move(fallback);
    }

    template <typename F>
    auto map(F&& func) const -> Option<decltype(func(std::declval<T>()))> {
        if (hasValue()) {
            return Option<decltype(func(std::declval<T>()))>(func(*m_value));
        }
        return Option<decltype(func(std::declval<T>()))>(std::nullopt);
    }
};

} // namespace eduerp::core