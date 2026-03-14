/**
 * Rule engine unit tests.
 * Tests apply_rules with various regex patterns, anchors, and edge cases.
 */

#include <gtest/gtest.h>
#include "document_ingestion/rule_engine.hpp"
#include "document_ingestion/rules_models.hpp"
#include <optional>

namespace {

using document_ingestion::Rule;
using document_ingestion::ExtractionCandidate;
using document_ingestion::apply_rules;

Rule make_rule(const std::string& name, const std::string& pattern,
               const std::optional<std::string>& anchor_before = std::nullopt,
               const std::optional<std::string>& anchor_after = std::nullopt) {
  Rule r;
  r.name = name;
  r.regex_pattern = pattern;
  r.anchor_before = anchor_before;
  r.anchor_after = anchor_after;
  r.active = true;
  return r;
}

TEST(RuleEngineTest, SimpleRegexMatch) {
  std::string text = "Invoice Number: INV-2024-001";
  std::vector<Rule> rules = {make_rule("invoice_number", R"(INV-\d{4}-\d{3})")};
  auto results = apply_rules(text, rules);
  ASSERT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].rule_name, "invoice_number");
  EXPECT_EQ(results[0].raw_match, "INV-2024-001");
}

TEST(RuleEngineTest, AnchorBefore) {
  std::string text = "Total: 1000. Other: 2000. Total: 3000.";
  std::vector<Rule> rules = {make_rule("total", R"(\d+)", "Total: ")};
  auto results = apply_rules(text, rules);
  ASSERT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].raw_match, "3000");  // Last "Total:" section
}

TEST(RuleEngineTest, DatePattern) {
  std::string text = "Invoice Date: 2024-01-15";
  std::vector<Rule> rules = {make_rule("date_field", R"(\d{4}-\d{2}-\d{2})")};
  auto results = apply_rules(text, rules);
  ASSERT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].raw_match, "2024-01-15");
}

TEST(RuleEngineTest, NoMatch_ReturnsEmpty) {
  std::string text = "No invoice here";
  std::vector<Rule> rules = {make_rule("invoice_number", R"(INV-\d{4}-\d{3})")};
  auto results = apply_rules(text, rules);
  EXPECT_TRUE(results.empty());
}

TEST(RuleEngineTest, MultipleRules) {
  std::string text = "Invoice: INV-2024-001, Date: 2024-01-15";
  std::vector<Rule> rules = {
    make_rule("invoice_number", R"(INV-\d{4}-\d{3})"),
    make_rule("date_field", R"(\d{4}-\d{2}-\d{2})"),
  };
  auto results = apply_rules(text, rules);
  ASSERT_EQ(results.size(), 2u);
  EXPECT_EQ(results[0].rule_name, "invoice_number");
  EXPECT_EQ(results[1].rule_name, "date_field");
}

TEST(RuleEngineTest, InactiveRule_Skipped) {
  std::string text = "INV-2024-001";
  Rule r = make_rule("invoice_number", R"(INV-\d{4}-\d{3})");
  r.active = false;
  std::vector<Rule> rules = {r};
  auto results = apply_rules(text, rules);
  EXPECT_TRUE(results.empty());
}

}  // namespace
