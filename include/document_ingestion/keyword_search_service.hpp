#pragma once

#include <string>
#include <vector>
#include <optional>

namespace document_ingestion {

struct KeywordSearchResult {
  std::string file_path;
  int matches = 0;
  std::string status;  // FOUND, NOT_FOUND, ERROR
  std::string file_type;
  std::optional<std::string> error;
};

std::vector<KeywordSearchResult> search_keyword_in_directory(
    const std::string& directory, const std::string& keyword,
    const std::optional<std::string>& file_type_filter = std::nullopt);

}  // namespace document_ingestion
