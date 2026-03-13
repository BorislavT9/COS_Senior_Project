#pragma once

#include "document_ingestion/rules_models.hpp"
#include <string>
#include <tuple>

namespace document_ingestion {

std::tuple<bool, std::string> validate(const ExtractionCandidate& candidate, const Rule& rule);

}  // namespace document_ingestion
