"""
Tests for validation: max_length, required rule with empty candidate fails.
"""

import pytest
from src.rules.models import Rule, ExtractionCandidate
from src.validation.validator import validate


def test_max_length_validation() -> None:
    rule = Rule(name="short", regex_pattern=r".*", max_length=5)
    ok_candidate = ExtractionCandidate("short", "hi", "hi", 0, 2)
    is_valid, msg = validate(ok_candidate, rule)
    assert is_valid is True
    assert msg == ""

    long_candidate = ExtractionCandidate("short", "hello world", "hello world", 0, 11)
    is_valid, msg = validate(long_candidate, rule)
    assert is_valid is False
    assert "max_length" in msg


def test_required_rule_with_empty_candidate_fails() -> None:
    rule = Rule(name="req", regex_pattern=r".*", required=True)
    empty = ExtractionCandidate("req", "", "", 0, 0)
    is_valid, msg = validate(empty, rule)
    assert is_valid is False
    assert "required" in msg.lower() or "empty" in msg.lower()


def test_required_rule_with_non_empty_passes() -> None:
    rule = Rule(name="req", regex_pattern=r".*", required=True)
    candidate = ExtractionCandidate("req", "x", "x", 0, 1)
    is_valid, msg = validate(candidate, rule)
    assert is_valid is True
    assert msg == ""
