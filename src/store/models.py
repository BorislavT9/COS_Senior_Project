"""
Data models for document records and extraction results.
"""

from dataclasses import dataclass, field
from typing import Optional


@dataclass
class DocumentRecord:
    """
    Metadata and extracted fields for a processed document.
    """
    doc_id: str  # UUID
    file_path: str
    file_type: str
    file_hash: str
    processed_at: str  # ISO format timestamp
    status: str  # SUCCESS, PARSE_FAILED, etc.
    error_message: Optional[str] = None
    extracted: dict[str, str] = field(default_factory=dict)  # field_name -> normalized_value
    extracted_raw: dict[str, str] = field(default_factory=dict)  # field_name -> raw_value


@dataclass
class ExtractionResult:
    """
    Result of applying a single extraction rule to a document.
    """
    field: str  # rule name / field name
    raw_value: Optional[str] = None
    normalized_value: Optional[str] = None
    status: str = "SUCCESS"  # SUCCESS, NO_MATCH, INVALID
    error: Optional[str] = None
