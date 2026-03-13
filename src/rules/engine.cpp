#include "document_ingestion/rule_engine.hpp"
#include "document_ingestion/normalizer.hpp"
#include <regex>
#include <algorithm>

namespace document_ingestion {

namespace {

std::pair<std::string, size_t> get_search_window(const std::string& text, const Rule& rule) {
  size_t start_offset = 0;
  std::string window = text;

  if (rule.anchor_before) {
    size_t idx = window.rfind(*rule.anchor_before);
    if (idx != std::string::npos) {
      start_offset = idx + rule.anchor_before->size();
      window = text.substr(start_offset);
    }
  }

  if (rule.anchor_after) {
    size_t idx = window.find(*rule.anchor_after);
    if (idx != std::string::npos) {
      window = window.substr(0, idx);
    }
  }
  return {window, start_offset};
}

}  // namespace

std::vector<ExtractionCandidate> apply_rules(const std::string& text,
                                            const std::vector<Rule>& rules) {
  std::vector<ExtractionCandidate> results;
  for (const auto& rule : rules) {
    if (!rule.active) continue;
    try {
      auto [window, start_offset] = get_search_window(text, rule);
      std::regex re(rule.regex_pattern);
      std::smatch m;
      if (std::regex_search(window, m, re)) {
        std::string raw = m.str(0);
        std::string normalized = normalize(raw);
        size_t abs_start = start_offset + static_cast<size_t>(m.position(0));
        size_t abs_end = abs_start + raw.size();
        results.push_back({rule.name, raw, normalized, abs_start, abs_end});
      }
    } catch (...) {
      continue;
    }
  }
  return results;
}

}  // namespace document_ingestion
