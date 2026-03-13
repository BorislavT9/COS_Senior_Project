#include "document_ingestion/logs_repo.hpp"
#include <sqlite3.h>

namespace document_ingestion {

void log_insert(sqlite3* conn, const std::string& level, const std::string& message,
                const std::string* context) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = "INSERT INTO logs (level, message, context) VALUES (?, ?, ?)";
  if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
  sqlite3_bind_text(stmt, 1, level.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
  if (context)
    sqlite3_bind_text(stmt, 3, context->c_str(), -1, SQLITE_TRANSIENT);
  else
    sqlite3_bind_null(stmt, 3);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

}  // namespace document_ingestion
