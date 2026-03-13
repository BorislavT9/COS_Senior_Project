#include "document_ingestion/parsers_internal.hpp"
#include <filesystem>
#include <stdexcept>
#include <sstream>

#if __has_include("miniz.h")
#include "miniz.h"
#include <pugixml.hpp>

namespace document_ingestion {

namespace {

std::vector<std::string> load_shared_strings(const std::string& file_path) {
  size_t size = 0;
  void* buf = mz_zip_extract_archive_file_to_heap(file_path.c_str(), "xl/sharedStrings.xml", &size, 0);
  if (!buf) return {};

  std::vector<std::string> result;
  pugi::xml_document doc;
  if (!doc.load_buffer(buf, size)) { mz_free(buf); return {}; }
  for (auto si : doc.select_nodes("//*[local-name()='si']")) {
    std::string s;
    for (auto t : si.node().select_nodes(".//*[local-name()='t']")) {
      s += t.node().child_value();
    }
    result.push_back(s);
  }
  mz_free(buf);
  return result;
}

std::string extract_sheet_text(const char* xml_data, size_t size,
                               const std::vector<std::string>& shared_strings) {
  pugi::xml_document doc;
  if (!doc.load_buffer(xml_data, size)) return "";

  std::ostringstream ss;
  auto sheetData = doc.select_node("//*[local-name()='sheetData']").node();
  if (!sheetData) return "";

  for (auto row : sheetData.select_nodes("./*[local-name()='row']")) {
    std::vector<std::string> row_cells;
    for (auto cell : row.node().select_nodes("*[local-name()='c']")) {
      auto v = cell.node().select_node("*[local-name()='v']").node();
      if (v) {
        std::string val = v.child_value();
        auto t = cell.node().attribute("t");
        if (t && std::string(t.value()) == "s" && !shared_strings.empty()) {
          try {
            int idx = std::stoi(val);
            if (idx >= 0 && idx < static_cast<int>(shared_strings.size()))
              val = shared_strings[idx];
          } catch (...) {}
        }
        row_cells.push_back(val);
      }
    }
    for (size_t i = 0; i < row_cells.size(); ++i) {
      if (i > 0) ss << "\t";
      ss << row_cells[i];
    }
    ss << "\n";
  }
  return ss.str();
}

}  // namespace

std::string parse_xlsx(const std::string& file_path) {
  std::filesystem::path p(file_path);
  if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p))
    throw std::runtime_error("XLSX not found: " + file_path);

  auto shared_strings = load_shared_strings(file_path);

  size_t size = 0;
  void* buf = mz_zip_extract_archive_file_to_heap(file_path.c_str(), "xl/worksheets/sheet1.xml", &size, 0);
  if (!buf) {
    buf = mz_zip_extract_archive_file_to_heap(file_path.c_str(), "xl/worksheets/sheet2.xml", &size, 0);
  }
  if (!buf) return "";

  std::string result = extract_sheet_text(static_cast<const char*>(buf), size, shared_strings);
  mz_free(buf);
  return result;
}

}  // namespace document_ingestion

#else

namespace document_ingestion {

std::string parse_xlsx(const std::string& file_path) {
  (void)file_path;
  throw std::runtime_error("XLSX parsing requires miniz and pugixml");
}

}  // namespace document_ingestion

#endif
