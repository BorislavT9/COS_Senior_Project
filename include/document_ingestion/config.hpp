#pragma once

#include <filesystem>

namespace document_ingestion {

void init_project_root_from_exe(const char* argv0);
void ensure_dirs();
std::filesystem::path get_project_root();
std::filesystem::path get_watch_dir();
std::filesystem::path get_db_path();
std::filesystem::path get_schema_path();
std::filesystem::path get_data_store_dir();
std::filesystem::path get_search_history_path();

}  // namespace document_ingestion
