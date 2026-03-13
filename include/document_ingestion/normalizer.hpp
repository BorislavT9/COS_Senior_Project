#pragma once

#include <string>

namespace document_ingestion {

std::string normalize_value(const std::string& value, bool uppercase = false,
                            bool remove_punctuation = false);
std::string normalize_field_name(const std::string& name);
std::string normalize(const std::string& value);

}  // namespace document_ingestion
