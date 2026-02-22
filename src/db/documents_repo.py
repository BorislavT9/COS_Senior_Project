"""
Repository for documents table operations.
"""

import sqlite3
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Optional

ISO_FMT = "%Y-%m-%dT%H:%M:%S"


@dataclass
class Document:
    """Document record from DB."""

    id: int
    file_path: str
    file_hash: str
    file_type: str
    processed_at: Optional[str]
    status: str
    error_message: Optional[str]


def insert(
    conn: sqlite3.Connection,
    file_path: str,
    file_hash: str,
    file_type: str,
    status: str = "pending",
    processed_at: Optional[str] = None,
    error_message: Optional[str] = None,
) -> int:
    """Insert a document. Returns new id."""
    cursor = conn.execute(
        """
        INSERT INTO documents (file_path, file_hash, file_type, processed_at, status, error_message)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (file_path, file_hash, file_type, processed_at, status, error_message),
    )
    conn.commit()
    return cursor.lastrowid


def update_status(
    conn: sqlite3.Connection,
    doc_id: int,
    status: str,
    processed_at: Optional[str] = None,
    error_message: Optional[str] = None,
) -> None:
    """Update document status and optional processed_at, error_message."""
    now = datetime.now(timezone.utc).strftime(ISO_FMT) if processed_at is None else processed_at
    conn.execute(
        """
        UPDATE documents SET status = ?, processed_at = ?, error_message = ?
        WHERE id = ?
        """,
        (status, now, error_message, doc_id),
    )
    conn.commit()


def find_by_path_and_hash(conn: sqlite3.Connection, file_path: str, file_hash: str) -> Optional[Document]:
    """Return document if same path and hash exists (unchanged)."""
    row = conn.execute(
        "SELECT id, file_path, file_hash, file_type, processed_at, status, error_message FROM documents WHERE file_path = ? AND file_hash = ?",
        (file_path, file_hash),
    ).fetchone()
    if row is None:
        return None
    return Document(
        id=row[0],
        file_path=row[1],
        file_hash=row[2],
        file_type=row[3],
        processed_at=row[4],
        status=row[5],
        error_message=row[6],
    )


def get_by_id(conn: sqlite3.Connection, doc_id: int) -> Optional[Document]:
    """Get document by id."""
    row = conn.execute(
        "SELECT id, file_path, file_hash, file_type, processed_at, status, error_message FROM documents WHERE id = ?",
        (doc_id,),
    ).fetchone()
    if row is None:
        return None
    return Document(
        id=row[0],
        file_path=row[1],
        file_hash=row[2],
        file_type=row[3],
        processed_at=row[4],
        status=row[5],
        error_message=row[6],
    )


def list_all(
    conn: sqlite3.Connection,
    status: Optional[str] = None,
    limit: int = 500,
) -> list[Document]:
    """List documents, optionally filtered by status."""
    if status:
        rows = conn.execute(
            "SELECT id, file_path, file_hash, file_type, processed_at, status, error_message FROM documents WHERE status = ? ORDER BY id DESC LIMIT ?",
            (status, limit),
        ).fetchall()
    else:
        rows = conn.execute(
            "SELECT id, file_path, file_hash, file_type, processed_at, status, error_message FROM documents ORDER BY id DESC LIMIT ?",
            (limit,),
        ).fetchall()
    return [
        Document(
            id=r[0],
            file_path=r[1],
            file_hash=r[2],
            file_type=r[3],
            processed_at=r[4],
            status=r[5],
            error_message=r[6],
        )
        for r in rows
    ]
