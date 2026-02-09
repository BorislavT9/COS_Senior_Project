"""
Rule engine: apply regex rules with optional anchor_before / anchor_after windowing.
Returns first match per rule; normalizes by stripping whitespace.
"""

import re
from typing import List

from .models import Rule, ExtractionCandidate


def _get_search_window(text: str, rule: Rule) -> tuple[str, int]:
    """
    Restrict text to the window after anchor_before and before anchor_after.
    Returns (window_text, start_offset_in_original_text).
    """
    start_offset = 0
    window = text

    if rule.anchor_before:
        idx = window.rfind(rule.anchor_before)
        if idx >= 0:
            start_offset = idx + len(rule.anchor_before)
            window = text[start_offset:]
        # else: no anchor found, use full text

    if rule.anchor_after:
        idx = window.find(rule.anchor_after)
        if idx >= 0:
            window = window[:idx]

    return window, start_offset


def apply_rules(text: str, rules: List[Rule]) -> List[ExtractionCandidate]:
    """
    Apply each active rule to text. First match only per rule.
    Uses anchor_before (search after last occurrence) and anchor_after (search before first occurrence).
    Normalizes matches by stripping whitespace.
    """
    results: List[ExtractionCandidate] = []
    for rule in rules:
        if not rule.active:
            continue
        try:
            window, start_offset = _get_search_window(text, rule)
            pattern = re.compile(rule.regex_pattern)
            m = pattern.search(window)
            if m is None:
                continue
            raw = m.group(0)
            normalized = raw.strip()
            abs_start = start_offset + m.start()
            abs_end = start_offset + m.end()
            results.append(
                ExtractionCandidate(
                    rule_name=rule.name,
                    raw_match=raw,
                    normalized_value=normalized,
                    start_idx=abs_start,
                    end_idx=abs_end,
                )
            )
        except re.error:
            # Invalid regex: skip rule, could log
            continue
    return results
