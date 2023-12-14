#include "logger.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>

const char* Logger::level_names[static_cast<int>(LogLevel::COUNT)] = {
  "DEBUG",
  "INFO",
  "WARNING",
  "ERROR",
  "FATAL"
};

Logger& Logger::getInst() {
  static Logger logger;
  return logger;
}

Logger::LogLevel& Logger::level() {
  return level_;
}

void Logger::log(LogLevel level, const char* filename, int line, const char* format, ...) {
  if (level < level_) {
    return;
  }
  char timestamp[32]{0};
  time_t ticks = time(NULL);
  tm* ptm = localtime(&ticks);
  memset(timestamp, 0, sizeof(timestamp));
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);
  int need_size = snprintf(nullptr, 0, "[%s] %s %s:%d ", level_names[static_cast<int>(level)], timestamp, filename,  line); 
  assert(need_size > 0);
  char* title =  new char[need_size+1];
  snprintf(title, need_size+1, "[%s] %s %s:%d ", level_names[static_cast<int>(level)], timestamp, filename, line);
  va_list arg_ptr;
  va_start(arg_ptr, format);
  need_size = vsnprintf(nullptr, 0, format, arg_ptr);
  va_end(arg_ptr);
  assert(need_size > 0);
  char* content = new char[need_size+1];
  va_start(arg_ptr, format);
  need_size = vsnprintf(content, need_size+1, format, arg_ptr);
  va_end(arg_ptr);
  {
    std::lock_guard<std::mutex> lock(mtx_);
    std::cerr << title << " " << content << std::endl;
  }
  if (level == LogLevel::FATAL) {
    exit(-1);
  }
}