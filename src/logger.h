#ifndef __LOGGER_H
#define __LOGGER_H


#include <mutex>


class Logger {
  public:
    enum class LogLevel:int {
      DEBUG = 0,
      INFO,
      WARNING,
      ERROR,
      FATAL,
      COUNT
    };
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
    static Logger& getInst();
    LogLevel& level();
    void log(LogLevel level, const char* filename, int line, const char* format, ...);
  private:
    Logger() { level_ = LogLevel::DEBUG;}
    std::mutex mtx_;
    LogLevel level_;
    static const char* level_names[static_cast<int>(LogLevel::COUNT)];
};

#define LOG(level, format, ...) \
  Logger::getInst().log(Logger::LogLevel::level, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define DEBUG(format, ...) \
  LOG(DEBUG, format, ##__VA_ARGS__)

#define INFO(format, ...) \
  LOG(INFO, format, ##__VA_ARGS__)

#define WARNING(format, ...) \
  LOG(WARNING, format, ##__VA_ARGS__)

#define ERROR(format, ...) \
  LOG(ERROR, format, ##__VA_ARGS__)

#define FATAL(format, ...) \
  LOG(FATAL, format, ##__VA_ARGS__)

#define CHECK(expr)              \
  if (!(expr)) {                 \
    FATAL("expact %s", #expr)    \
  }

#endif