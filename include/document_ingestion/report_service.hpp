#pragma once

#include "document_ingestion/store.hpp"
#include <filesystem>

namespace document_ingestion {

void generate_html_report(const ExtractionStore& store, const std::filesystem::path& output_path);

}  // namespace document_ingestion
