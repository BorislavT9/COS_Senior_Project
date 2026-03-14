/**
 * Integration tests for store save/load and ExtractionStore operations.
 */

#include <gtest/gtest.h>
#include "document_ingestion/store.hpp"
#include "document_ingestion/models.hpp"
#include <filesystem>
#include <fstream>

namespace {

namespace fs = std::filesystem;

class StoreIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    test_dir_ = fs::temp_directory_path() / "document_ingestion_test";
    fs::create_directories(test_dir_);
  }
  void TearDown() override {
    fs::remove_all(test_dir_);
  }
  fs::path test_dir_;
};

TEST_F(StoreIntegrationTest, SaveAndLoad_PreservesData) {
  document_ingestion::ExtractionStore store;
  document_ingestion::DocumentRecord rec;
  rec.doc_id = "doc-1";
  rec.file_path = "/path/to/file.txt";
  rec.file_type = ".txt";
  rec.file_hash = "abc123";
  rec.status = "SUCCESS";
  rec.extracted["invoice_number"] = "INV-2024-001";
  rec.extracted["date_field"] = "2024-01-15";
  store.add_document(rec);
  store.index_extraction("invoice_number", "INV-2024-001", "doc-1");
  store.index_extraction("date_field", "2024-01-15", "doc-1");
  store.save(test_dir_);
  document_ingestion::ExtractionStore loaded;
  loaded.load(test_dir_);
  EXPECT_EQ(loaded.documents().size(), 1u);
  auto it = loaded.documents().find("doc-1");
  ASSERT_NE(it, loaded.documents().end());
  EXPECT_EQ(it->second.extracted.at("invoice_number"), "INV-2024-001");
  auto results = loaded.query_equals("invoice_number", "INV-2024-001");
  ASSERT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].doc_id, "doc-1");
}

TEST_F(StoreIntegrationTest, RangeQuery_AfterLoad) {
  document_ingestion::ExtractionStore store;
  for (int i = 0; i < 10; ++i) {
    document_ingestion::DocumentRecord rec;
    rec.doc_id = "doc-" + std::to_string(i);
    rec.file_path = "/path/" + std::to_string(i);
    rec.file_type = ".txt";
    rec.status = "SUCCESS";
    rec.extracted["id"] = "ID-" + std::to_string(i);
    store.add_document(rec);
    store.index_extraction("id", "ID-" + std::to_string(i), rec.doc_id);
  }
  store.save(test_dir_);
  document_ingestion::ExtractionStore loaded;
  loaded.load(test_dir_);
  auto results = loaded.query_range("id", "ID-2", "ID-5");
  EXPECT_EQ(results.size(), 4u);
}

}  // namespace
