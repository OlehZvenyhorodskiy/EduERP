#pragma once

#include <variant>
#include <system_error>
#include <string>

namespace eduerp::core {

/**
 * @brief Represents a standardized error in the EduERP system.
 */
struct Error {
    int code;
    std::string message;

    Error(int c, std::string msg) : code(c), message(std::move(msg)) {}
    Error(std::error_code ec) : code(ec.value()), message(ec.message()) {}
};

/**
 * @brief Standardized Result type for error handling instead of exceptions.
 */
template <typename T>
class Result {
private:
    std::variant<T, Error> m_data;

public:
    // Success constructor
    Result(T val) : m_data(std::move(val)) {}
    
    // Error constructor
    Result(Error err) : m_data(std::move(err)) {}

    bool isOk() const { return std::holds_alternative<T>(m_data); }
    bool isError() const { return std::holds_alternative<Error>(m_data); }

    T& value() {
        if (!isOk()) {
            throw std::runtime_error("Attempted to unwrap an error Result: " + error().message);
        }
        return std::get<T>(m_data);
    }
    
    const T& value() const {
        if (!isOk()) {
            throw std::runtime_error("Attempted to unwrap an error Result: " + error().message);
        }
        return std::get<T>(m_data);
    }

    const Error& error() const {
        return std::get<Error>(m_data);
    }
};

// Specialization for void
template <>
class Result<void> {
private:
    std::unique_ptr<Error> m_error;

public:
    Result() : m_error(nullptr) {}
    Result(Error err) : m_error(std::make_unique<Error>(std::move(err))) {}

    bool isOk() const { return m_error == nullptr; }
    bool isError() const { return m_error != nullptr; }

    const Error& error() const {
        return *m_error;
    }
};

} // namespace eduerp::core