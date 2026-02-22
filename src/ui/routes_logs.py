"""
Logs API route: list logs with optional level filter.
"""

from typing import Optional

from fastapi import APIRouter

from src.config import get_db_path
from src.db import database
from src.db.logs_repo import list_all as logs_list

router = APIRouter(prefix="/logs", tags=["logs"])


@router.get("")
def list_logs(level: Optional[str] = None) -> list[dict]:
    """List logs, optionally filtered by level (INFO, WARNING, ERROR, DEBUG)."""
    conn = database.get_connection(str(get_db_path()))
    try:
        rows = logs_list(conn, level=level)
        return [
            {
                "id": r.id,
                "level": r.level,
                "message": r.message,
                "context": r.context,
                "created_at": r.created_at,
            }
            for r in rows
        ]
    finally:
        conn.close()
