#pragma once

#include "document_ingestion/avl_tree.hpp"
#include "document_ingestion/models.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <optional>

namespace document_ingestion {

class ExtractionStore {
 public:
  ExtractionStore() = default;

  void add_document(const DocumentRecord& record);
  void index_extraction(const std::string& field, const std::string& value, const std::string& doc_id);
  std::vector<DocumentRecord> query_equals(const std::string& field, const std::string& value) const;
  std::vector<DocumentRecord> query_range(const std::string& field,
                                         const std::string& low,
                                         const std::string& high) const;
  std::vector<std::string> list_field_values(const std::string& field) const;
  std::vector<std::string> get_indexed_fields() const;

  void save(const std::filesystem::path& base_dir) const;
  void load(const std::filesystem::path& base_dir);

  std::optional<DocumentRecord> find_by_path_and_hash(const std::string& file_path,
                                                      const std::string& file_hash) const;

  const std::unordered_map<std::string, DocumentRecord>& documents() const { return documents_; }


 private:
  std::unordered_map<std::string, DocumentRecord> documents_;
  std::unordered_map<std::string, AVLTree> indexes_;
};

}  // namespace document_ingestion
