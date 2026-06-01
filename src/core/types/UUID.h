#pragma once

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>

namespace eduerp::core {

/**
 * @brief Lightweight UUID v4 generator without boost dependency.
 */
class UUID {
private:
    std::array<uint8_t, 16> m_bytes{};

public:
    UUID() = default;

    static UUID generate() {
        UUID uuid;
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist;

        auto val1 = dist(gen);
        auto val2 = dist(gen);
        std::memcpy(uuid.m_bytes.data(), &val1, 8);
        std::memcpy(uuid.m_bytes.data() + 8, &val2, 8);

        // Set version 4 bits
        uuid.m_bytes[6] = (uuid.m_bytes[6] & 0x0F) | 0x40;
        // Set variant bits
        uuid.m_bytes[8] = (uuid.m_bytes[8] & 0x3F) | 0x80;

        return uuid;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < 16; ++i) {
            if (i == 4 || i == 6 || i == 8 || i == 10) oss << '-';
            oss << std::setw(2) << static_cast<int>(m_bytes[i]);
        }
        return oss.str();
    }

    bool operator==(const UUID& other) const { return m_bytes == other.m_bytes; }
    bool operator!=(const UUID& other) const { return !(*this == other); }
};

} // namespace eduerp::core
