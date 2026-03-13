#pragma once

#include "document_ingestion/rules_models.hpp"
#include <string>
#include <vector>

namespace document_ingestion {

std::vector<ExtractionCandidate> apply_rules(const std::string& text,
                                             const std::vector<Rule>& rules);

}  // namespace document_ingestion
