#include "document_ingestion/database.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace document_ingestion {

void init_db(const std::string& db_path, const std::string& schema_path) {
  std::filesystem::create_directories(std::filesystem::path(db_path).parent_path());
  if (!std::filesystem::exists(schema_path))
    throw std::runtime_error("Schema file not found: " + schema_path);

  std::ifstream f(schema_path);
  std::string sql((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  f.close();

  sqlite3* db = nullptr;
  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    std::string err = sqlite3_errmsg(db);
    sqlite3_close(db);
    throw std::runtime_error("Cannot open DB: " + err);
  }

  char* errmsg = nullptr;
  if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
    std::string err(errmsg);
    sqlite3_free(errmsg);
    sqlite3_close(db);
    throw std::runtime_error("Schema exec failed: " + err);
  }
  sqlite3_close(db);
}

struct ConnectionDeleter {
  void operator()(sqlite3* p) const { if (p) sqlite3_close(p); }
};

std::unique_ptr<sqlite3, decltype(&sqlite3_close)> get_connection(const std::string& db_path) {
  std::filesystem::create_directories(std::filesystem::path(db_path).parent_path());
  sqlite3* db = nullptr;
  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    throw std::runtime_error(std::string("Cannot open DB: ") + sqlite3_errmsg(db));
  }
  return std::unique_ptr<sqlite3, decltype(&sqlite3_close)>(db, &sqlite3_close);
}

}  // namespace document_ingestion
