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
  std::filesystem::path p(path);
  if (!std::filesystem::exists(p)) return result;
  if (!std::filesystem::is_directory(p)) {
    result.push_back(std::filesystem::absolute(p).string());
    return result;
  }
  for (const auto& entry : std::filesystem::recursive_directory_iterator(p)) {
    if (entry.is_regular_file()) {
      result.push_back(entry.path().string());
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
