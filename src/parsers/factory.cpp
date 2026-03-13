#include "document_ingestion/parsers.hpp"
#include "document_ingestion/parsers_internal.hpp"
#include <filesystem>
#include <stdexcept>
#include <map>

namespace document_ingestion {

std::string parse_file(const std::string& file_path) {
  std::string ext = std::filesystem::path(file_path).extension().string();
  for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

  if (ext == ".txt") return parse_txt(file_path);
  if (ext == ".pdf") return parse_pdf(file_path);
  if (ext == ".docx") return parse_docx(file_path);
  if (ext == ".xlsx") return parse_xlsx(file_path);

  throw std::runtime_error("Unsupported file type: " + ext + " (path: " + file_path + ")");
}

}  // namespace document_ingestion
