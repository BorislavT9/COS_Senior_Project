#pragma once

#include <string>
#include <unordered_map>
#include <optional>

namespace document_ingestion {

struct DocumentRecord {
  std::string doc_id;
  std::string file_path;
  std::string file_type;
  std::string file_hash;
  std::string processed_at;
  std::string status;
  std::optional<std::string> error_message;
  std::unordered_map<std::string, std::string> extracted;
  std::unordered_map<std::string, std::string> extracted_raw;
};

struct ExtractionResult {
  std::string field;
  std::optional<std::string> raw_value;
  std::optional<std::string> normalized_value;
  std::string status = "SUCCESS";
  std::optional<std::string> error;
};

}  // namespace document_ingestion
