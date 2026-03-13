#include "document_ingestion/parsers_internal.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace document_ingestion {

std::string parse_txt(const std::string& file_path) {
  std::filesystem::path p(file_path);
  if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p))
    throw std::runtime_error("TXT not found: " + file_path);
  std::ifstream f(file_path);
  if (!f) throw std::runtime_error("Cannot open: " + file_path);
  std::stringstream ss;
  ss << f.rdbuf();
  std::string content = ss.str();
  for (size_t i = 0; i < content.size(); ++i) {
    if (static_cast<unsigned char>(content[i]) >= 0x80 && 
        static_cast<unsigned char>(content[i]) < 0xC0) {
      content[i] = '?';
    }
  }
  return content;
}

}  // namespace document_ingestion
