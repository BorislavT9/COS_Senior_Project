#pragma once

#include <string>
#include <memory>
#include <sqlite3.h>

namespace document_ingestion {

void init_db(const std::string& db_path, const std::string& schema_path);
std::unique_ptr<sqlite3, decltype(&sqlite3_close)> get_connection(const std::string& db_path);

}  // namespace document_ingestion
