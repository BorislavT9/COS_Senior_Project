#include "document_ingestion/rules_repo.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <cstdlib>

namespace document_ingestion {

Rule to_rule(const RuleRow& row) {
  Rule r;
  r.id = row.id;
  r.name = row.name;
  r.file_type = row.file_type;
  r.regex_pattern = row.regex_pattern;
  r.anchor_before = row.anchor_before;
  r.anchor_after = row.anchor_after;
  r.max_length = row.max_length;
  r.required = row.required;
  r.active = row.active;
  return r;
}

int rule_insert(sqlite3* conn, const std::string& name, const std::string& regex_pattern,
                const std::optional<std::string>& file_type,
                const std::optional<std::string>& anchor_before,
                const std::optional<std::string>& anchor_after,
                const std::optional<int>& max_length,
                bool required, bool active) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"(
    INSERT INTO rules (name, file_type, regex_pattern, anchor_before, anchor_after, max_length, required, active)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
  )";
  if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(conn));

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
  if (file_type)
    sqlite3_bind_text(stmt, 2, file_type->c_str(), -1, SQLITE_TRANSIENT);
  else
    sqlite3_bind_null(stmt, 2);
  sqlite3_bind_text(stmt, 3, regex_pattern.c_str(), -1, SQLITE_TRANSIENT);
  if (anchor_before)
    sqlite3_bind_text(stmt, 4, anchor_before->c_str(), -1, SQLITE_TRANSIENT);
  else
    sqlite3_bind_null(stmt, 4);
  if (anchor_after)
    sqlite3_bind_text(stmt, 5, anchor_after->c_str(), -1, SQLITE_TRANSIENT);
  else
    sqlite3_bind_null(stmt, 5);
  if (max_length)
    sqlite3_bind_int(stmt, 6, *max_length);
  else
    sqlite3_bind_null(stmt, 6);
  sqlite3_bind_int(stmt, 7, required ? 1 : 0);
  sqlite3_bind_int(stmt, 8, active ? 1 : 0);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    throw std::runtime_error(sqlite3_errmsg(conn));
  }
  int id = static_cast<int>(sqlite3_last_insert_rowid(conn));
  sqlite3_finalize(stmt);
  sqlite3_exec(conn, "COMMIT", nullptr, nullptr, nullptr);
  return id;
}

std::vector<RuleRow> rules_list_all(sqlite3* conn, const std::optional<bool>& active_only,
                                    const std::optional<std::string>& file_type) {
  std::string sql = "SELECT id, name, file_type, regex_pattern, anchor_before, anchor_after, max_length, required, active FROM rules WHERE 1=1";
  if (active_only) sql += " AND active = ?";
  if (file_type) sql += " AND (file_type IS NULL OR file_type = ?)";
  sql += " ORDER BY id";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(conn, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    return {};

  int idx = 1;
  if (active_only) sqlite3_bind_int(stmt, idx++, *active_only ? 1 : 0);
  if (file_type) sqlite3_bind_text(stmt, idx++, file_type->c_str(), -1, SQLITE_TRANSIENT);

  std::vector<RuleRow> result;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    RuleRow row;
    row.id = sqlite3_column_int(stmt, 0);
    row.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
      row.file_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    row.regex_pattern = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL)
      row.anchor_before = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    if (sqlite3_column_type(stmt, 5) != SQLITE_NULL)
      row.anchor_after = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    if (sqlite3_column_type(stmt, 6) != SQLITE_NULL)
      row.max_length = sqlite3_column_int(stmt, 6);
    row.required = sqlite3_column_int(stmt, 7) != 0;
    row.active = sqlite3_column_int(stmt, 8) != 0;
    result.push_back(row);
  }
  sqlite3_finalize(stmt);
  return result;
}

}  // namespace document_ingestion
