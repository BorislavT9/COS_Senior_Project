#include "document_ingestion/logger.hpp"
#include <fstream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace document_ingestion {

namespace {

std::string log_path;
std::mutex log_mutex;

std::string timestamp() {
  auto now = std::time(nullptr);
  std::tm tm_buf;
#ifdef _WIN32
  localtime_s(&tm_buf, &now);
#else
  localtime_r(&now, &tm_buf);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

void write_log(const std::string& level, const std::string& name, const std::string& msg) {
  std::string line = "[" + timestamp() + "] [" + level + "] [" + name + "] " + msg + "\n";
  std::lock_guard<std::mutex> lock(log_mutex);
  std::cerr << line;
  if (!log_path.empty()) {
    std::ofstream f(log_path, std::ios::app);
    if (f) f << line;
  }
}

}  // namespace

void set_default_log_path(const std::string& path) {
  log_path = path;
}

void log_info(const std::string& name, const std::string& msg) {
  write_log("INFO", name, msg);
}

void log_warning(const std::string& name, const std::string& msg) {
  write_log("WARN", name, msg);
}

void log_error(const std::string& name, const std::string& msg) {
  write_log("ERROR", name, msg);
}

}  // namespace document_ingestion
