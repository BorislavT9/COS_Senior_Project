#pragma once

#include <string>

namespace document_ingestion {

void set_default_log_path(const std::string& path);
void log_info(const std::string& name, const std::string& msg);
void log_warning(const std::string& name, const std::string& msg);
void log_error(const std::string& name, const std::string& msg);

}  // namespace document_ingestion
