#include "document_ingestion/config.hpp"
#include <filesystem>
#include <cstdlib>

namespace document_ingestion {

namespace {

std::filesystem::path project_root;

std::filesystem::path find_project_root_from(std::filesystem::path start) {
  auto path = std::filesystem::absolute(start);
  for (int i = 0; i < 15; ++i) {
    if (std::filesystem::exists(path / "schema.sql") ||
        std::filesystem::exists(path / "CMakeLists.txt")) {
      return path;
    }
    if (!path.has_parent_path() || path == path.parent_path()) break;
    path = path.parent_path();
  }
  return std::filesystem::current_path();
}

}  // namespace

void init_project_root_from_exe(const char* argv0) {
  if (project_root.empty() && argv0) {
    std::filesystem::path exe_path = std::filesystem::absolute(argv0);
    project_root = find_project_root_from(exe_path.parent_path());
  }
}

void ensure_dirs() {
  auto root = get_project_root();
  for (const auto& name : {"data", "logs", "sample_docs", "data_store"}) {
    std::filesystem::create_directories(root / name);
  }
}

std::filesystem::path get_project_root() {
  if (project_root.empty()) {
    project_root = find_project_root_from(std::filesystem::current_path());
  }
  return project_root;
}

std::filesystem::path get_watch_dir() {
  return get_project_root() / "sample_docs";
}

std::filesystem::path get_db_path() {
  return get_project_root() / "data" / "app.db";
}

std::filesystem::path get_log_path() {
  return get_project_root() / "logs" / "app.log";
}

std::filesystem::path get_schema_path() {
  return get_project_root() / "schema.sql";
}

std::filesystem::path get_data_store_dir() {
  return get_project_root() / "data_store";
}

std::filesystem::path get_search_history_path() {
  return get_project_root() / "data_store" / "search_history.json";
}

}  // namespace document_ingestion
