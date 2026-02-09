"""
Tests for rules engine: regex extraction, anchor_before / anchor_after windowing.
"""

import pytest
from src.rules.models import Rule, ExtractionCandidate
from src.rules.engine import apply_rules


def test_regex_extraction_basic() -> None:
    text = "Invoice #INV-2024-001 and another INV-2024-002"
    rule = Rule(name="inv", regex_pattern=r"INV-\d{4}-\d{3}")
    rules = [rule]
    result = apply_rules(text, rules)
    assert len(result) == 1
    assert result[0].rule_name == "inv"
    assert result[0].normalized_value == "INV-2024-001"
    assert result[0].raw_match.strip() == "INV-2024-001"


def test_anchor_before_windowing() -> None:
    text = "Prefix ID: X1 Suffix. Later ID: X2"
    # Only search after "Later "
    rule = Rule(name="id_after_later", regex_pattern=r"ID:\s*(\w+)", anchor_before="Later ")
    result = apply_rules(text, [rule])
    assert len(result) == 1
    assert result[0].normalized_value == "ID: X2"


def test_anchor_after_windowing() -> None:
    text = "Start ID: first End ID: second More"
    # Search before first " End"
    rule = Rule(name="id_before_end", regex_pattern=r"ID:\s*(\w+)", anchor_after=" End")
    result = apply_rules(text, [rule])
    assert len(result) == 1
    assert "first" in result[0].normalized_value


def test_anchor_before_and_anchor_after() -> None:
    # After last "tag ", window is "D E"; before first " tag" leaves "D E". First [A-Z] match is "D".
    text = "A tag B C tag D E"
    rule = Rule(name="middle", regex_pattern=r"([A-Z])", anchor_before="tag ", anchor_after=" tag")
    result = apply_rules(text, [rule])
    assert len(result) >= 1
    assert result[0].normalized_value == "D"
