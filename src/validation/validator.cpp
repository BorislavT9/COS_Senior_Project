#include "document_ingestion/validator.hpp"

namespace document_ingestion {

std::tuple<bool, std::string> validate(const ExtractionCandidate& candidate, const Rule& rule) {
  if (rule.required && candidate.raw_match.empty()) {
    return {false, "required rule produced no match"};
  }
  if (rule.max_length && candidate.normalized_value.size() > static_cast<size_t>(*rule.max_length)) {
    return {false, "value exceeds max_length"};
  }
  return {true, ""};
}

}  // namespace document_ingestion
