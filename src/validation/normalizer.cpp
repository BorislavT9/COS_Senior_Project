#include "document_ingestion/normalizer.hpp"
#include <regex>
#include <algorithm>
#include <cctype>

namespace document_ingestion {

std::string normalize_value(const std::string& value, bool uppercase, bool remove_punctuation) {
  if (value.empty()) return "";

  std::string s = value;
  while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(0, 1);
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();

  s = std::regex_replace(s, std::regex(R"(\s+)"), " ");

  if (uppercase) {
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  if (remove_punctuation) {
    s = std::regex_replace(s, std::regex(R"(^[^\w\s]+|[^\w\s]+$)"), "");
  }
  return s;
}

std::string normalize_field_name(const std::string& name) {
  if (name.empty()) return "unnamed_field";

  std::string s = name;
  for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  s = std::regex_replace(s, std::regex(R"([^\w]+)"), "_");
  while (!s.empty() && s.front() == '_') s.erase(0, 1);
  while (!s.empty() && s.back() == '_') s.pop_back();
  if (!s.empty() && !std::isalpha(static_cast<unsigned char>(s[0])) && s[0] != '_')
    s = "_" + s;
  if (s.empty()) return "unnamed_field";
  return s;
}

std::string normalize(const std::string& value) {
  return normalize_value(value);
}

}  // namespace document_ingestion
