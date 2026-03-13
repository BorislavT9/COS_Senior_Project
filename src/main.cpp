#include "document_ingestion/config.hpp"
#include "document_ingestion/store.hpp"
#include "document_ingestion/search_history.hpp"
#include "document_ingestion/ingestion_service.hpp"
#include "document_ingestion/report_service.hpp"
#include "document_ingestion/keyword_search_service.hpp"
#include "document_ingestion/database.hpp"
#include "document_ingestion/rules_repo.hpp"
#include "document_ingestion/logger.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

static document_ingestion::ExtractionStore load_store() {
  document_ingestion::ExtractionStore store;
  auto dir = document_ingestion::get_data_store_dir();
  if (fs::exists(dir / "store.json")) {
    store.load(dir);
  }
  return store;
}

static void cmd_ingest(const std::string& watch_dir) {
  auto store = load_store();
  auto summary = document_ingestion::run_scan(watch_dir, &store);
  store.save(document_ingestion::get_data_store_dir());
  auto report_path = document_ingestion::get_data_store_dir() / "report.html";
  document_ingestion::generate_html_report(store, report_path);
#ifdef _WIN32
  ShellExecuteA(nullptr, "open", report_path.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
  std::cout << "\n=== Ingestion Summary ===\n"
            << "Processed: " << summary.processed << "\n"
            << "Skipped (unchanged): " << summary.skipped << "\n"
            << "Failed: " << summary.failed << "\n"
            << "Total extractions: " << summary.extractions << "\n";
  for (const auto& e : summary.errors) std::cout << "  - " << e << "\n";
}

static void cmd_query(const std::string& field, const std::string& equals,
                      const std::string& range_low, const std::string& range_high) {
  auto store = load_store();
  if (!equals.empty()) {
    auto results = store.query_equals(field, equals);
    std::cout << "\nFound " << results.size() << " document(s) where " << field << " = '" << equals << "':\n";
    for (const auto& doc : results) {
      std::cout << "  - " << doc.file_path << " (ID: " << doc.doc_id << ")\n";
      if (!doc.extracted.empty()) {
        for (const auto& [k, v] : doc.extracted) std::cout << "    " << k << ": " << v << "\n";
      }
    }
  } else if (!range_low.empty() && !range_high.empty()) {
    auto results = store.query_range(field, range_low, range_high);
    std::cout << "\nFound " << results.size() << " document(s) where " << field << " in ['" << range_low << "', '" << range_high << "']:\n";
    for (const auto& doc : results) {
      std::cout << "  - " << doc.file_path << "\n";
    }
  } else {
    std::cerr << "Error: Specify --equals or --range\n";
    exit(1);
  }
}

static void cmd_list(const std::string& field) {
  auto store = load_store();
  auto values = store.list_field_values(field);
  auto indexed = store.get_indexed_fields();
  if (values.empty()) {
    if (indexed.empty()) {
      std::cout << "No indexed fields. Run ingest or --demo first.\n";
    } else {
      std::cout << "No values for '" << field << "'. Available fields: ";
      for (size_t i = 0; i < indexed.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << indexed[i];
      }
      std::cout << "\n";
    }
    return;
  }
  std::cout << "\nUnique values for " << field << " (" << values.size() << " total):\n";
  for (const auto& v : values) std::cout << "  - " << v << "\n";
}

static void cmd_export(const std::string& format, const std::string& output_path) {
  auto store = load_store();
  if (format == "json") {
    nlohmann::json j;
    j["documents"] = nlohmann::json::array();
    for (const auto& [id, doc] : store.documents()) {
      nlohmann::json d;
      d["doc_id"] = doc.doc_id;
      d["file_path"] = doc.file_path;
      d["file_type"] = doc.file_type;
      d["extracted"] = doc.extracted;
      j["documents"].push_back(d);
    }
    std::ofstream f(output_path);
    f << j.dump(2);
    std::cout << "Exported " << store.documents().size() << " documents to " << output_path << "\n";
  } else if (format == "csv") {
    std::ofstream f(output_path);
    f << "doc_id,file_path,field,value\n";
    int count = 0;
    for (const auto& [_, doc] : store.documents()) {
      for (const auto& [k, v] : doc.extracted) {
        f << "\"" << doc.doc_id << "\",\"" << doc.file_path << "\",\"" << k << "\",\"" << v << "\"\n";
        count++;
      }
    }
    std::cout << "Exported " << count << " rows to " << output_path << "\n";
  }
}

static void run_demo() {
  document_ingestion::ensure_dirs();
  document_ingestion::init_db(document_ingestion::get_db_path().string(),
                               document_ingestion::get_schema_path().string());
  auto conn = document_ingestion::get_connection(document_ingestion::get_db_path().string());
  auto rules = document_ingestion::rules_list_all(conn.get(), std::nullopt, std::nullopt);
  if (rules.empty()) {
    document_ingestion::rule_insert(conn.get(), "invoice_number", R"(INV-\d{4}-\d{3})",
                                   ".txt", std::nullopt, std::nullopt, std::nullopt, false, true);
    document_ingestion::rule_insert(conn.get(), "date_field", R"(\d{4}-\d{2}-\d{2})",
                                   ".txt", std::nullopt, std::nullopt, std::nullopt, false, true);
    std::cout << "Inserted sample rules: invoice_number, date_field\n";
  }
  auto watch_dir = document_ingestion::get_watch_dir().string();
  std::cout << "Scanning: " << watch_dir << "\n";
  document_ingestion::ExtractionStore store;
  auto summary = document_ingestion::run_scan(watch_dir, &store);
  store.save(document_ingestion::get_data_store_dir());
  std::cout << "\n--- Demo Scan Summary ---\n"
            << "Processed: " << summary.processed << "\n"
            << "Skipped: " << summary.skipped << "\n"
            << "Failed: " << summary.failed << "\n"
            << "Extractions: " << summary.extractions << "\n";
}

static void interactive_menu() {
  document_ingestion::SearchHistory history(document_ingestion::get_search_history_path());
  for (;;) {
    std::cout << "\n============================================================\n"
              << "     DOCUMENT SEARCH & EXTRACTION SYSTEM\n"
              << "============================================================\n\n"
              << "  Main Menu:\n"
              << "  1. Search for new files\n"
              << "  2. Show previously searched files\n"
              << "  3. Exit\n\n"
              << "Select option (1-3): ";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
      std::cout << "\nEnter directory path to search: ";
      std::string dir;
      std::getline(std::cin, dir);
      if (!dir.empty()) {
        while (dir.back() == ' ') dir.pop_back();
        while (!dir.empty() && dir.front() == ' ') dir.erase(0, 1);
      }
      std::cout << "Enter keyword: ";
      std::string keyword;
      std::getline(std::cin, keyword);
      auto results = document_ingestion::search_keyword_in_directory(dir, keyword, std::nullopt);
      std::vector<document_ingestion::FoundFile> found_files;
      for (const auto& r : results) {
        if (r.matches > 0) {
          std::cout << "  Found: " << r.file_path << " (" << r.matches << " matches)\n";
          found_files.push_back({r.file_path, r.matches});
        }
      }
      history.add_search("ALL", dir, keyword, found_files, 0);
      try {
        auto store = load_store();
        auto summary = document_ingestion::run_scan(dir, &store);
        store.save(document_ingestion::get_data_store_dir());
        std::cout << "Stored " << summary.processed << " files.\n";
      } catch (const std::exception& e) {
        std::cout << "Ingest warning: " << e.what() << " (search was saved)\n";
      }
    } else if (choice == "2") {
      auto searches = history.get_all_searches();
      if (searches.empty()) std::cout << "\nNo previous searches.\n";
      else {
        std::cout << "\n--- Previously searched files ---\n";
        for (size_t i = 0; i < searches.size(); ++i) {
          const auto& s = searches[i];
          for (const auto& f : s.found_files) {
            std::cout << "  " << f.file_path << "\n    Keyword: \"" << s.keyword << "\" (" << f.matches << " matches)\n";
          }
        }
      }
    } else if (choice == "3") {
      std::cout << "\nGoodbye!\n";
      break;
    } else {
      std::cout << "Invalid choice.\n";
    }
  }
}

int main(int argc, char* argv[]) {
  document_ingestion::init_project_root_from_exe(argc > 0 ? argv[0] : nullptr);
  document_ingestion::ensure_dirs();
  document_ingestion::set_default_log_path(document_ingestion::get_log_path().string());

  std::string cmd, watch_dir, field, equals, range_low, range_high, format, out_path;
  bool serve = false, demo = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "ingest") cmd = "ingest";
    else if (arg == "query") cmd = "query";
    else if (arg == "list") cmd = "list";
    else if (arg == "export") cmd = "export";
    else if (arg == "--serve") serve = true;
    else if (arg == "--demo") demo = true;
    else if (arg == "--watch_dir" && i + 1 < argc) watch_dir = argv[++i];
    else if (arg == "--field" && i + 1 < argc) field = argv[++i];
    else if (arg == "--equals" && i + 1 < argc) equals = argv[++i];
    else if (arg == "--range" && i + 2 < argc) { range_low = argv[++i]; range_high = argv[++i]; }
    else if (arg == "--format" && i + 1 < argc) format = argv[++i];
    else if (arg == "--out" && i + 1 < argc) out_path = argv[++i];
  }

  if (demo) { run_demo(); return 0; }
  if (serve) {
    std::cout << "Web UI: run with Python for now (python -m src.main --serve)\n";
    return 0;
  }
  if (cmd == "ingest") {
    if (watch_dir.empty()) watch_dir = document_ingestion::get_watch_dir().string();
    cmd_ingest(watch_dir);
    return 0;
  }
  if (cmd == "query") {
    if (field.empty()) { std::cerr << "Error: --field required\n"; return 1; }
    cmd_query(field, equals, range_low, range_high);
    return 0;
  }
  if (cmd == "list") {
    if (field.empty()) { std::cerr << "Error: --field required\n"; return 1; }
    cmd_list(field);
    return 0;
  }
  if (cmd == "export") {
    if (format.empty() || out_path.empty()) { std::cerr << "Error: --format and --out required\n"; return 1; }
    cmd_export(format, out_path);
    return 0;
  }

  interactive_menu();
  return 0;
}
