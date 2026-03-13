#include "document_ingestion/parsers_internal.hpp"
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <string>

#if __has_include("miniz.h")
#include "miniz.h"
#include <pugixml.hpp>
#include <sstream>

namespace document_ingestion {

namespace {

std::string extract_text_from_docx_xml(const char* xml_data, size_t size) {
  pugi::xml_document doc;
  if (!doc.load_buffer(xml_data, size)) return "";

  std::ostringstream ss;
  for (auto node : doc.select_nodes("//*[local-name()='t']")) {
    std::string val = node.node().child_value();
    if (!val.empty()) ss << val;
  }
  return ss.str();
}

}  // namespace

std::string parse_docx(const std::string& file_path) {
  std::filesystem::path p(file_path);
  if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p))
    throw std::runtime_error("DOCX not found: " + file_path);

  size_t size = 0;
  void* buf = mz_zip_extract_archive_file_to_heap(file_path.c_str(), "word/document.xml", &size, 0);
  if (!buf) throw std::runtime_error("Failed to extract word/document.xml from DOCX: " + file_path);

  std::string result = extract_text_from_docx_xml(static_cast<const char*>(buf), size);
  mz_free(buf);
  return result;
}

}  // namespace document_ingestion

#else

namespace document_ingestion {

std::string parse_docx(const std::string& file_path) {
  (void)file_path;
  throw std::runtime_error("DOCX parsing requires miniz and pugixml");
}

}  // namespace document_ingestion

#endif
