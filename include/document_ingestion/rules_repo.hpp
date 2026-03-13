#pragma once

#include "document_ingestion/rules_models.hpp"
#include <sqlite3.h>
#include <vector>
#include <optional>

namespace document_ingestion {

struct RuleRow {
  int id;
  std::string name;
  std::optional<std::string> file_type;
  std::string regex_pattern;
  std::optional<std::string> anchor_before;
  std::optional<std::string> anchor_after;
  std::optional<int> max_length;
  bool required;
  bool active;
};

Rule to_rule(const RuleRow& row);
int rule_insert(sqlite3* conn, const std::string& name, const std::string& regex_pattern,
                const std::optional<std::string>& file_type = std::nullopt,
                const std::optional<std::string>& anchor_before = std::nullopt,
                const std::optional<std::string>& anchor_after = std::nullopt,
                const std::optional<int>& max_length = std::nullopt,
                bool required = false, bool active = true);
std::vector<RuleRow> rules_list_all(sqlite3* conn, const std::optional<bool>& active_only = std::nullopt,
                                    const std::optional<std::string>& file_type = std::nullopt);

}  // namespace document_ingestion
