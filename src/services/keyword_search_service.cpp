#include "document_ingestion/keyword_search_service.hpp"
#include <optional>
#include "document_ingestion/parsers.hpp"
#include "document_ingestion/registry.hpp"
#include "document_ingestion/scanner.hpp"
#include <algorithm>
#include <cctype>

namespace document_ingestion {

namespace {

std::string to_lower(const std::string& s) {
  std::string r = s;
  for (auto& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return r;
}

int count_substring(const std::string& haystack, const std::string& needle) {
  int count = 0;
  size_t pos = 0;
  while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    count++;
    pos += needle.size();
  }
  return count;
}

}  // namespace

std::vector<KeywordSearchResult> search_keyword_in_directory(
    const std::string& directory, const std::string& keyword,
    const std::optional<std::string>& file_type_filter) {
  std::vector<KeywordSearchResult> results;
  auto paths = scan_directory(directory);
  auto supported = filter_supported(paths);

  std::vector<std::string> filtered;
  if (file_type_filter && !file_type_filter->empty()) {
    for (const auto& p : supported) {
      if (get_file_type(p) == *file_type_filter) filtered.push_back(p);
    }
  } else {
    filtered = supported;
  }

  std::string keyword_lower = to_lower(keyword);

  for (const auto& file_path : filtered) {
    KeywordSearchResult r;
    r.file_path = file_path;
    r.file_type = get_file_type(file_path);
    try {
      std::string text = parse_file(file_path);
      std::string text_lower = to_lower(text);
      int matches = count_substring(text_lower, keyword_lower);
      r.matches = matches;
      r.status = matches > 0 ? "FOUND" : "NOT_FOUND";
    } catch (const std::exception& e) {
      r.matches = 0;
      r.status = "ERROR";
      r.error = e.what();
    }
    results.push_back(r);
  }
  return results;
}

}  // namespace document_ingestion
