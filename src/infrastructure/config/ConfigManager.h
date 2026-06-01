#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::infra {

/**
 * @brief Manages application configuration from JSON file and environment variables.
 */
class ConfigManager {
private:
    nlohmann::json m_config;
    std::string m_filePath;

public:
    explicit ConfigManager(const std::string& filePath = "config.json")
        : m_filePath(filePath) {}

    bool load() {
        std::ifstream file(m_filePath);
        if (!file.is_open()) {
            spdlog::warn("Config file not found at '{}', using defaults", m_filePath);
            setDefaults();
            return false;
        }
        try {
            file >> m_config;
            spdlog::info("Configuration loaded from '{}'", m_filePath);
            return true;
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("Failed to parse config file: {}", e.what());
            setDefaults();
            return false;
        }
    }

    std::string getString(const std::string& key, const std::string& defaultVal = "") const {
        if (m_config.contains(key) && m_config[key].is_string()) {
            return m_config[key].get<std::string>();
        }
        return defaultVal;
    }

    int getInt(const std::string& key, int defaultVal = 0) const {
        if (m_config.contains(key) && m_config[key].is_number_integer()) {
            return m_config[key].get<int>();
        }
        return defaultVal;
    }

    bool getBool(const std::string& key, bool defaultVal = false) const {
        if (m_config.contains(key) && m_config[key].is_boolean()) {
            return m_config[key].get<bool>();
        }
        return defaultVal;
    }

private:
    void setDefaults() {
        m_config = {
            {"api_base_url", "https://api.eduerp.example.com"},
            {"api_version", "v1"},
            {"websocket_url", "wss://api.eduerp.example.com/ws"},
            {"default_language", "nl-BE"},
            {"log_level", "info"},
            {"cache_max_size_mb", 50},
            {"connection_timeout_ms", 10000},
            {"heartbeat_interval_s", 30}
        };
    }
};

} // namespace eduerp::infra
