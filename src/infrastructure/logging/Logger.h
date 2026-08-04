#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>

namespace eduerp::infra {

/**
 * @brief Centralized logging facade wrapping spdlog.
 *        Call Logger::init() once at application startup.
 */
class Logger {
public:
    static void init(const std::string& appName = "EduERP",
                     const std::string& logFilePath = "logs/eduerp.log") {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::info);

        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath, 1024 * 1024 * 5, 3); // 5MB per file, 3 rotations
        fileSink->set_level(spdlog::level::debug);

        auto logger = std::make_shared<spdlog::logger>(
            appName, spdlog::sinks_init_list{consoleSink, fileSink});
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        spdlog::set_default_logger(logger);
        spdlog::info("Logger initialized for {}", appName);
    }

    static void shutdown() {
        spdlog::shutdown();
    }
};

} // namespace eduerp::infra