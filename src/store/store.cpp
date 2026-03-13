#include "document_ingestion/store.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <algorithm>

namespace document_ingestion {

void ExtractionStore::add_document(const DocumentRecord& record) {
  documents_[record.doc_id] = record;
}

void ExtractionStore::index_extraction(const std::string& field, const std::string& value,
                                       const std::string& doc_id) {
  if (indexes_.find(field) == indexes_.end()) {
    indexes_[field] = AVLTree();
  }
  indexes_[field].insert(value, doc_id);
}

std::vector<DocumentRecord> ExtractionStore::query_equals(const std::string& field,
                                                         const std::string& value) const {
  auto it = indexes_.find(field);
  if (it == indexes_.end()) return {};
  auto doc_ids = it->second.search(value);
  std::vector<DocumentRecord> result;
  for (const auto& id : doc_ids) {
    auto doc_it = documents_.find(id);
    if (doc_it != documents_.end()) result.push_back(doc_it->second);
  }
  return result;
}

std::vector<DocumentRecord> ExtractionStore::query_range(const std::string& field,
                                                          const std::string& low,
                                                          const std::string& high) const {
  auto it = indexes_.find(field);
  if (it == indexes_.end()) return {};
  auto doc_ids = it->second.range_query(low, high);
  std::vector<DocumentRecord> result;
  for (const auto& id : doc_ids) {
    auto doc_it = documents_.find(id);
    if (doc_it != documents_.end()) result.push_back(doc_it->second);
  }
  return result;
}

std::vector<std::string> ExtractionStore::list_field_values(const std::string& field) const {
  auto it = indexes_.find(field);
  if (it == indexes_.end()) return {};
  auto items = it->second.inorder_items();
  std::vector<std::string> result;
  for (const auto& [key, _] : items) result.push_back(key);
  return result;
}

void ExtractionStore::save(const std::filesystem::path& base_dir) const {
  std::filesystem::create_directories(base_dir);
  nlohmann::json docs;
  for (const auto& [doc_id, record] : documents_) {
    nlohmann::json rec;
    rec["doc_id"] = record.doc_id;
    rec["file_path"] = record.file_path;
    rec["file_type"] = record.file_type;
    rec["file_hash"] = record.file_hash;
    rec["processed_at"] = record.processed_at;
    rec["status"] = record.status;
    if (record.error_message) rec["error_message"] = *record.error_message;
    rec["extracted"] = record.extracted;
    rec["extracted_raw"] = record.extracted_raw;
    docs[doc_id] = rec;
  }
  std::ofstream f(base_dir / "store.json");
  f << docs.dump(2);
  for (const auto& [field, tree] : indexes_) {
    nlohmann::json index_data;
    for (const auto& [key, doc_ids] : tree.inorder_items()) {
      index_data.push_back({{"key", key}, {"doc_ids", doc_ids}});
    }
    std::ofstream idx(base_dir / ("index_" + field + ".json"));
    idx << index_data.dump(2);
  }
}

void ExtractionStore::load(const std::filesystem::path& base_dir) {
  auto store_path = base_dir / "store.json";
  if (std::filesystem::exists(store_path)) {
    std::ifstream f(store_path);
    auto docs = nlohmann::json::parse(f);
    for (auto it = docs.begin(); it != docs.end(); ++it) {
      DocumentRecord rec;
      rec.doc_id = it.value()["doc_id"];
      rec.file_path = it.value()["file_path"];
      rec.file_type = it.value()["file_type"];
      rec.file_hash = it.value()["file_hash"];
      rec.processed_at = it.value()["processed_at"];
      rec.status = it.value()["status"];
      if (it.value().contains("error_message") && !it.value()["error_message"].is_null())
        rec.error_message = it.value()["error_message"].get<std::string>();
      if (it.value().contains("extracted"))
        for (auto& [k, v] : it.value()["extracted"].items())
          rec.extracted[k] = v.get<std::string>();
      if (it.value().contains("extracted_raw"))
        for (auto& [k, v] : it.value()["extracted_raw"].items())
          rec.extracted_raw[k] = v.get<std::string>();
      documents_[rec.doc_id] = std::move(rec);
    }
  }
  for (const auto& entry : std::filesystem::directory_iterator(base_dir)) {
    if (!entry.is_regular_file()) continue;
    auto name = entry.path().filename().string();
    if (name.size() > 12 && name.compare(0, 6, "index_") == 0 && name.compare(name.size() - 5, 5, ".json") == 0) {
      std::string field = name.substr(6, name.size() - 6 - 5);  // "index_" + field + ".json"
      std::ifstream idx(entry.path());
      if (!idx) continue;
      try {
        auto index_data = nlohmann::json::parse(idx);
        if (!index_data.is_array()) continue;
        AVLTree tree;
        for (const auto& item : index_data) {
          if (!item.contains("key") || !item.contains("doc_ids")) continue;
          std::string key = item["key"].get<std::string>();
          for (const auto& doc_id : item["doc_ids"]) {
            tree.insert(key, doc_id.get<std::string>());
          }
        }
        indexes_[field] = std::move(tree);
      } catch (...) {}
    }
  }
}

std::vector<std::string> ExtractionStore::get_indexed_fields() const {
  std::vector<std::string> result;
  for (const auto& [k, _] : indexes_) result.push_back(k);
  std::sort(result.begin(), result.end());
  return result;
}

std::optional<DocumentRecord> ExtractionStore::find_by_path_and_hash(
    const std::string& file_path, const std::string& file_hash) const {
  for (const auto& [_, rec] : documents_) {
    if (rec.file_path == file_path && rec.file_hash == file_hash)
      return rec;
  }
  return std::nullopt;
}

}  // namespace document_ingestion
