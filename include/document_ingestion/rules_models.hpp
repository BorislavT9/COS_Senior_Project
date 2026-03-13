#pragma once

#include <string>
#include <optional>

namespace document_ingestion {

struct Rule {
  std::string name;
  std::string regex_pattern;
  std::optional<int> id;
  std::optional<std::string> file_type;
  std::optional<std::string> anchor_before;
  std::optional<std::string> anchor_after;
  std::optional<int> max_length;
  bool required = false;
  bool active = true;
};

struct ExtractionCandidate {
  std::string rule_name;
  std::string raw_match;
  std::string normalized_value;
  size_t start_idx = 0;
  size_t end_idx = 0;
};

}  // namespace document_ingestion
