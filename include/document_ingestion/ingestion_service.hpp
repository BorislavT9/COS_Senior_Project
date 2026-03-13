#pragma once

#include "document_ingestion/store.hpp"
#include <string>
#include <vector>
#include <map>

namespace document_ingestion {

struct ScanSummary {
  int processed = 0;
  int skipped = 0;
  int failed = 0;
  int extractions = 0;
  std::vector<std::string> errors;
};

ScanSummary run_scan(const std::string& watch_dir, ExtractionStore* store = nullptr);

}  // namespace document_ingestion
