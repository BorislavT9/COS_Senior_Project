#include "document_ingestion/parsers_internal.hpp"
#include <filesystem>
#include <stdexcept>

#ifdef ENABLE_POPPLER
#include <poppler-document.h>
#include <poppler-page.h>
#endif

namespace document_ingestion {

std::string parse_pdf(const std::string& file_path) {
  std::filesystem::path p(file_path);
  if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p))
    throw std::runtime_error("PDF not found: " + file_path);

#ifdef ENABLE_POPPLER
  auto doc = poppler::document::load_from_file(file_path);
  if (!doc) throw std::runtime_error("Failed to load PDF: " + file_path);
  std::string result;
  for (int i = 0; i < doc->pages(); ++i) {
    auto page = doc->create_page(i);
    if (page) {
      result += page->text().to_latin1();
      if (i + 1 < doc->pages()) result += "\n";
    }
  }
  return result;
#else
  (void)file_path;
  throw std::runtime_error("PDF parsing requires build with -DENABLE_PDF=ON and Poppler");
#endif
}

}  // namespace document_ingestion
