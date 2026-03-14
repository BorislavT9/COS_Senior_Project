/**
 * Pipeline validation integration test.
 * End-to-end test: scan directory -> parse -> extract (rules) -> validate -> store.
 */

#include <gtest/gtest.h>
#include "document_ingestion/ingestion_service.hpp"
#include "document_ingestion/config.hpp"
#include "document_ingestion/database.hpp"
#include "document_ingestion/rules_repo.hpp"
#include <filesystem>
#include <fstream>

namespace {

namespace fs = std::filesystem;

TEST(PipelineTest, FullIngestion_ExtractsAndStores) {
  const fs::path orig_cwd = fs::current_path();
  const fs::path project_root = document_ingestion::get_project_root();
  const fs::path schema_src = project_root / "schema.sql";

  fs::path temp_root = fs::temp_directory_path() / "doc_ingest_pipeline_test";
  fs::create_directories(temp_root);
  temp_root = fs::canonical(temp_root);

  fs::create_directories(temp_root / "data");
  fs::create_directories(temp_root / "data_store");
  fs::path watch_dir = temp_root / "watch_dir";
  fs::create_directories(watch_dir);

  std::ofstream(temp_root / "CMakeLists.txt") << "# placeholder\n";

  if (fs::exists(schema_src)) {
    fs::copy_file(schema_src, temp_root / "schema.sql", fs::copy_options::overwrite_existing);
  } else {
    fs::remove_all(temp_root);
    GTEST_SKIP() << "schema.sql not found";
  }

  document_ingestion::init_db((temp_root / "data" / "app.db").string(),
                              (temp_root / "schema.sql").string());

  auto conn = document_ingestion::get_connection((temp_root / "data" / "app.db").string());
  document_ingestion::rule_insert(conn.get(), "invoice_number", R"(INV-\d{4}-\d{3})",
                                  std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, true);
  document_ingestion::rule_insert(conn.get(), "date_field", R"(\d{4}-\d{2}-\d{2})",
                                  std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, true);

  fs::path sample_txt = watch_dir / "sample.txt";
  std::ofstream f(sample_txt);
  f << "Invoice Number: INV-2024-001\nInvoice Date: 2024-01-15\n";
  f.close();

  fs::current_path(temp_root);

  document_ingestion::ExtractionStore store;
  auto summary = document_ingestion::run_scan(watch_dir.string(), &store);

  fs::current_path(orig_cwd);
  std::error_code ec;
  fs::remove_all(temp_root, ec);

  EXPECT_EQ(summary.processed, 1);
  EXPECT_EQ(summary.failed, 0);
  EXPECT_EQ(store.documents().size(), 1u);

  const auto& docs = store.documents();
  auto it = docs.begin();
  ASSERT_NE(it, docs.end());
  EXPECT_EQ(it->second.status, "SUCCESS");
  EXPECT_EQ(it->second.extracted.at("invoice_number"), "INV-2024-001");
  EXPECT_EQ(it->second.extracted.at("date_field"), "2024-01-15");

  auto results = store.query_equals("invoice_number", "INV-2024-001");
  EXPECT_EQ(results.size(), 1u);
}

}  // namespace
