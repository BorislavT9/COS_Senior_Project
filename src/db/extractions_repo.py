"""
Repository for extractions table operations.
"""

import sqlite3
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Optional

ISO_FMT = "%Y-%m-%dT%H:%M:%S"


@dataclass
class ExtractionRow:
    """Extraction record from DB."""

    id: int
    document_id: int
    rule_id: int
    extracted_value: Optional[str]
    status: str
    error_message: Optional[str]
    created_at: str


def insert(
    conn: sqlite3.Connection,
    document_id: int,
    rule_id: int,
    status: str,
    extracted_value: Optional[str] = None,
    error_message: Optional[str] = None,
) -> int:
    """Insert extraction. Returns new id."""
    now = datetime.now(timezone.utc).strftime(ISO_FMT)
    cursor = conn.execute(
        """
        INSERT INTO extractions (document_id, rule_id, extracted_value, status, error_message, created_at)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (document_id, rule_id, extracted_value, status, error_message, now),
    )
    conn.commit()
    return cursor.lastrowid


def list_all(
    conn: sqlite3.Connection,
    document_id: Optional[int] = None,
    rule_id: Optional[int] = None,
    status: Optional[str] = None,
    limit: int = 500,
) -> list[ExtractionRow]:
    """List extractions with optional filters."""
    query = """
        SELECT id, document_id, rule_id, extracted_value, status, error_message, created_at
        FROM extractions WHERE 1=1
    """
    params = []
    if document_id is not None:
        query += " AND document_id = ?"
        params.append(document_id)
    if rule_id is not None:
        query += " AND rule_id = ?"
        params.append(rule_id)
    if status is not None:
        query += " AND status = ?"
        params.append(status)
    query += " ORDER BY id DESC LIMIT ?"
    params.append(limit)
    rows = conn.execute(query, params).fetchall()
    return [
        ExtractionRow(
            id=r[0],
            document_id=r[1],
            rule_id=r[2],
            extracted_value=r[3],
            status=r[4],
            error_message=r[5],
            created_at=r[6],
        )
        for r in rows
    ]
