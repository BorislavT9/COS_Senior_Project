#pragma once

#include <string>
#include <vector>

namespace document_ingestion {

std::string get_file_type(const std::string& file_path);
bool is_supported(const std::string& file_path);
std::vector<std::string> filter_supported(const std::vector<std::string>& paths);

}  // namespace document_ingestion
