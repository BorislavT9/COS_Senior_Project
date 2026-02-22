"""
Repository for logs table operations.
"""

import sqlite3
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Optional

ISO_FMT = "%Y-%m-%dT%H:%M:%S"


@dataclass
class LogRow:
    """Log record from DB."""

    id: int
    level: str
    message: str
    context: Optional[str]
    created_at: str


def insert(
    conn: sqlite3.Connection,
    level: str,
    message: str,
    context: Optional[str] = None,
) -> int:
    """Insert log. Returns new id."""
    now = datetime.now(timezone.utc).strftime(ISO_FMT)
    cursor = conn.execute(
        "INSERT INTO logs (level, message, context, created_at) VALUES (?, ?, ?, ?)",
        (level, message, context, now),
    )
    conn.commit()
    return cursor.lastrowid


def list_all(
    conn: sqlite3.Connection,
    level: Optional[str] = None,
    limit: int = 200,
) -> list[LogRow]:
    """List logs, optionally filtered by level."""
    if level:
        rows = conn.execute(
            "SELECT id, level, message, context, created_at FROM logs WHERE level = ? ORDER BY id DESC LIMIT ?",
            (level, limit),
        ).fetchall()
    else:
        rows = conn.execute(
            "SELECT id, level, message, context, created_at FROM logs ORDER BY id DESC LIMIT ?",
            (limit,),
        ).fetchall()
    return [
        LogRow(id=r[0], level=r[1], message=r[2], context=r[3], created_at=r[4])
        for r in rows
    ]
