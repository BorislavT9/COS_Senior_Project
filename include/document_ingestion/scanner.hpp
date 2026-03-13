#pragma once

#include <string>
#include <vector>

namespace document_ingestion {

std::vector<std::string> scan_directory(const std::string& path);
std::string compute_file_hash(const std::string& path);

}  // namespace document_ingestion
