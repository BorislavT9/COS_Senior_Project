#pragma once

#include <string>

namespace document_ingestion {

std::string parse_txt(const std::string& file_path);
std::string parse_pdf(const std::string& file_path);
std::string parse_docx(const std::string& file_path);
std::string parse_xlsx(const std::string& file_path);

}  // namespace document_ingestion
