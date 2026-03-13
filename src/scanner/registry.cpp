#include "document_ingestion/registry.hpp"
#include <filesystem>
#include <algorithm>

namespace document_ingestion {

static const std::vector<std::string> SUPPORTED_EXTENSIONS = {".pdf", ".docx", ".txt", ".xlsx"};

std::string get_file_type(const std::string& file_path) {
  std::string ext = std::filesystem::path(file_path).extension().string();
  for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  for (const auto& supported : SUPPORTED_EXTENSIONS) {
    if (ext == supported) return ext;
  }
  return "";
}

bool is_supported(const std::string& file_path) {
  return !get_file_type(file_path).empty();
}

std::vector<std::string> filter_supported(const std::vector<std::string>& paths) {
  std::vector<std::string> result;
  for (const auto& p : paths) {
    if (is_supported(p)) result.push_back(p);
  }
  return result;
}

}  // namespace document_ingestion
