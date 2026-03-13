#include "document_ingestion/ingestion_service.hpp"
#include "document_ingestion/config.hpp"
#include "document_ingestion/database.hpp"
#include "document_ingestion/logs_repo.hpp"
#include "document_ingestion/rules_repo.hpp"
#include <sqlite3.h>
#include <random>
#include "document_ingestion/parsers.hpp"
#include "document_ingestion/rule_engine.hpp"
#include "document_ingestion/normalizer.hpp"
#include "document_ingestion/validator.hpp"
#include "document_ingestion/scanner.hpp"
#include "document_ingestion/registry.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace document_ingestion {

namespace {

std::string generate_uuid() {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  std::uniform_int_distribution<> dis2(8, 11);
  std::ostringstream ss;
  ss << std::hex;
  for (int i = 0; i < 8; i++) ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 4; i++) ss << dis(gen);
  ss << "-4";
  for (int i = 0; i < 3; i++) ss << dis(gen);
  ss << "-" << dis2(gen);
  for (int i = 0; i < 3; i++) ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 12; i++) ss << dis(gen);
  return ss.str();
}

std::string iso_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf;
#ifdef _WIN32
  localtime_s(&tm_buf, &time);
#else
  localtime_r(&time, &tm_buf);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
  return oss.str();
}

}  // namespace

ScanSummary run_scan(const std::string& watch_dir, ExtractionStore* store_ptr) {
  ScanSummary summary;

  ExtractionStore local_store;
  ExtractionStore* store = store_ptr ? store_ptr : &local_store;

  if (!store_ptr) {
    auto store_dir = get_data_store_dir();
    if (std::filesystem::exists(store_dir / "store.json")) {
      store->load(store_dir);
    }
  }

  auto db_path = get_db_path().string();
  std::unique_ptr<sqlite3, decltype(&sqlite3_close)> conn(nullptr, &sqlite3_close);
  try {
    conn = get_connection(db_path);
    log_insert(conn.get(), "INFO", "Scan started: watch_dir=" + watch_dir, &watch_dir);
  } catch (...) {
  }

  auto paths = scan_directory(watch_dir);
  auto supported = filter_supported(paths);

  for (const auto& file_path : supported) {
    try {
      std::string file_hash = compute_file_hash(file_path);
      auto existing = store->find_by_path_and_hash(file_path, file_hash);
      if (existing) {
        summary.skipped++;
        continue;
      }

      std::string file_type = get_file_type(file_path);
      std::string doc_id = generate_uuid();
      std::string processed_at = iso_timestamp();

      std::string text;
      try {
        text = parse_file(file_path);
      } catch (const std::exception& e) {
        DocumentRecord rec;
        rec.doc_id = doc_id;
        rec.file_path = file_path;
        rec.file_type = file_type;
        rec.file_hash = file_hash;
        rec.processed_at = processed_at;
        rec.status = "PARSE_FAILED";
        rec.error_message = e.what();
        store->add_document(rec);
        if (conn) { std::string ctx = rec.error_message.value_or(""); log_insert(conn.get(), "ERROR", std::string("Parse failed: ") + file_path, &ctx); }
        summary.failed++;
        continue;
      }

      std::vector<Rule> rules;
      if (conn) {
        auto rows = rules_list_all(conn.get(), true, file_type);
        for (const auto& row : rows) rules.push_back(to_rule(row));
      } else {
        try {
          auto c = get_connection(db_path);
          auto rows = rules_list_all(c.get(), true, file_type);
          for (const auto& row : rows) rules.push_back(to_rule(row));
        } catch (...) {}
      }

      std::unordered_map<std::string, std::string> extracted, extracted_raw;

      if (!rules.empty()) {
        auto candidates = apply_rules(text, rules);
        std::unordered_map<std::string, ExtractionCandidate> by_rule;
        for (const auto& c : candidates) by_rule[c.rule_name] = c;

        for (const auto& rule : rules) {
          auto it = by_rule.find(rule.name);
          if (it == by_rule.end()) {
            if (rule.required) summary.extractions++;
            continue;
          }
          const auto& cand = it->second;
          std::string raw_val = cand.raw_match;
          std::string normalized_val = normalize_value(cand.normalized_value);

          auto [valid, err_msg] = validate(cand, rule);
          if (!valid) {
            summary.extractions++;
            continue;
          }
          extracted[rule.name] = normalized_val;
          extracted_raw[rule.name] = raw_val;
          store->index_extraction(rule.name, normalized_val, doc_id);
          summary.extractions++;
        }
      }

      DocumentRecord rec;
      rec.doc_id = doc_id;
      rec.file_path = file_path;
      rec.file_type = file_type;
      rec.file_hash = file_hash;
      rec.processed_at = processed_at;
      rec.status = "SUCCESS";
      rec.extracted = extracted;
      rec.extracted_raw = extracted_raw;
      store->add_document(rec);
      summary.processed++;
    } catch (const std::exception& e) {
      summary.failed++;
      summary.errors.push_back(std::string(file_path) + ": " + e.what());
      if (conn) { std::string ctx = summary.errors.back(); log_insert(conn.get(), "ERROR", std::string("Processing failed: ") + file_path, &ctx); }
    }
  }

  if (conn) {
    std::string msg = "Scan completed: processed=" + std::to_string(summary.processed) +
                      ", skipped=" + std::to_string(summary.skipped) +
                      ", failed=" + std::to_string(summary.failed);
    log_insert(conn.get(), "INFO", msg, nullptr);
  }

  return summary;
}

}  // namespace document_ingestion
