#pragma once

#include <sqlite3.h>
#include <string>

namespace document_ingestion {

void log_insert(sqlite3* conn, const std::string& level, const std::string& message,
                const std::string* context = nullptr);

}  // namespace document_ingestion
