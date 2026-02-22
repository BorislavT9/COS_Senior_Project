"""
Rules repository: facade over db rules_repo for rules package.
"""

from typing import Optional

from src.db.rules_repo import (
    delete as db_delete,
    get_by_id as db_get,
    insert as db_insert,
    list_all as db_list,
    set_active as db_set_active,
    to_rule,
    update as db_update,
)
from src.rules.models import Rule


def get_connection():
    """Get DB connection. Uses config."""
    from src.config import get_db_path
    from src.db import database
    return database.get_connection(str(get_db_path()))


def create(
    name: str,
    regex_pattern: str,
    file_type: Optional[str] = None,
    anchor_before: Optional[str] = None,
    anchor_after: Optional[str] = None,
    max_length: Optional[int] = None,
    required: bool = False,
    active: bool = True,
) -> int:
    """Create rule. Returns new id."""
    conn = get_connection()
    try:
        return db_insert(
            conn, name, regex_pattern,
            file_type=file_type,
            anchor_before=anchor_before,
            anchor_after=anchor_after,
            max_length=max_length,
            required=required,
            active=active,
        )
    finally:
        conn.close()


def get_active_rules(file_type: Optional[str] = None) -> list[Rule]:
    """Load active rules, optionally filtered by file_type."""
    conn = get_connection()
    try:
        rows = db_list(conn, active_only=True, file_type=file_type)
        return [to_rule(r) for r in rows]
    finally:
        conn.close()
