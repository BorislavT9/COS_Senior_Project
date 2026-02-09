"""
Rule and ExtractionCandidate dataclasses for extraction engine.
"""

from dataclasses import dataclass
from typing import Optional


@dataclass
class Rule:
    """User-defined extraction rule: regex + optional anchors and constraints."""

    name: str
    regex_pattern: str
    id: Optional[int] = None
    file_type: Optional[str] = None
    anchor_before: Optional[str] = None
    anchor_after: Optional[str] = None
    max_length: Optional[int] = None
    required: bool = False
    active: bool = True


@dataclass
class ExtractionCandidate:
    """A single extraction result from applying a rule to text."""

    rule_name: str
    raw_match: str
    normalized_value: str
    start_idx: int
    end_idx: int
