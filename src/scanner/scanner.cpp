#include "document_ingestion/scanner.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>

#define PICOSHA2_BUFFER_SIZE_FOR_INPUT_ITERATOR 4
#include "picosha2.h"

namespace document_ingestion {

std::vector<std::string> scan_directory(const std::string& path) {
  std::vector<std::string> result;
#ifdef _WIN32
  std::filesystem::path p = std::filesystem::u8path(path);
#else
  std::filesystem::path p(path);
#endif
  std::error_code ec;
  if (!std::filesystem::exists(p, ec) || ec) return result;
  if (!std::filesystem::is_directory(p, ec) || ec) {
    auto abs = std::filesystem::absolute(p, ec);
    if (!ec) {
#ifdef _WIN32
      result.push_back(abs.u8string());
#else
      result.push_back(abs.string());
#endif
    }
    return result;
  }

  using rdi = std::filesystem::recursive_directory_iterator;
  auto opts = std::filesystem::directory_options::skip_permission_denied;
  std::error_code it_ec;
  auto it = rdi(p, opts, it_ec);
  if (it_ec) return result;
  const auto end = rdi();
  while (it != end) {
    try {
      if (it->is_regular_file(it_ec) && !it_ec) {
#ifdef _WIN32
        result.push_back(it->path().u8string());
#else
        result.push_back(it->path().string());
#endif
      }
    } catch (...) {
    }
    it_ec.clear();
    it.increment(it_ec);
    if (it_ec) {
      it_ec.clear();
      try { ++it; } catch (...) { break; }
    }
  }
  return result;
}

std::string compute_file_hash(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) throw std::runtime_error("File not found: " + path);
  std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return picosha2::hash256_hex_string(content);
}

}  // namespace document_ingestion
