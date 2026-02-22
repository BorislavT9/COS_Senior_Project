"""
Documents API route: list documents.
"""

from typing import Optional

from fastapi import APIRouter

from src.config import get_db_path
from src.db import database
from src.db.documents_repo import list_all as documents_list

router = APIRouter(prefix="/documents", tags=["documents"])


@router.get("")
def list_documents(status: Optional[str] = None) -> list[dict]:
    """List documents with optional status filter."""
    conn = database.get_connection(str(get_db_path()))
    try:
        rows = documents_list(conn, status=status)
        return [
            {
                "id": r.id,
                "file_path": r.file_path,
                "file_type": r.file_type,
                "processed_at": r.processed_at,
                "status": r.status,
            }
            for r in rows
        ]
    finally:
        conn.close()
