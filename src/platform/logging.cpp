#include "logging.h"

namespace Engine {

std::shared_ptr<spdlog::logger> Log::s_logger;

void Log::init() {
    if (s_logger) return;

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("engine.log", true));

    s_logger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
    s_logger->set_level(spdlog::level::trace);
    s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::register_logger(s_logger);
    info("Logging initialized");
}

void Log::shutdown() {
    info("Logging shutdown");
    spdlog::shutdown();
    s_logger.reset();
}

} // namespace Engine

