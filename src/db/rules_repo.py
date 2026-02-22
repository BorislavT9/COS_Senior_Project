"""
Repository for rules table operations.
"""

import sqlite3
from dataclasses import dataclass
from typing import Optional

from src.rules.models import Rule


@dataclass
class RuleRow:
    """Rule record from DB."""

    id: int
    name: str
    file_type: Optional[str]
    regex_pattern: str
    anchor_before: Optional[str]
    anchor_after: Optional[str]
    max_length: Optional[int]
    required: bool
    active: bool


def to_rule(row: RuleRow) -> Rule:
    """Convert RuleRow to Rule model."""
    return Rule(
        id=row.id,
        name=row.name,
        file_type=row.file_type,
        regex_pattern=row.regex_pattern,
        anchor_before=row.anchor_before,
        anchor_after=row.anchor_after,
        max_length=row.max_length,
        required=bool(row.required),
        active=bool(row.active),
    )


def insert(
    conn: sqlite3.Connection,
    name: str,
    regex_pattern: str,
    file_type: Optional[str] = None,
    anchor_before: Optional[str] = None,
    anchor_after: Optional[str] = None,
    max_length: Optional[int] = None,
    required: bool = False,
    active: bool = True,
) -> int:
    """Insert a rule. Returns new id."""
    cursor = conn.execute(
        """
        INSERT INTO rules (name, file_type, regex_pattern, anchor_before, anchor_after, max_length, required, active)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (name, file_type, regex_pattern, anchor_before, anchor_after, max_length, 1 if required else 0, 1 if active else 0),
    )
    conn.commit()
    return cursor.lastrowid


def update(
    conn: sqlite3.Connection,
    rule_id: int,
    *,
    name: Optional[str] = None,
    file_type: Optional[str] = None,
    regex_pattern: Optional[str] = None,
    anchor_before: Optional[str] = None,
    anchor_after: Optional[str] = None,
    max_length: Optional[int] = None,
    required: Optional[bool] = None,
) -> bool:
    """Update rule by id. Returns True if row was updated."""
    updates = []
    params = []
    if name is not None:
        updates.append("name = ?")
        params.append(name)
    if file_type is not None:
        updates.append("file_type = ?")
        params.append(file_type)
    if regex_pattern is not None:
        updates.append("regex_pattern = ?")
        params.append(regex_pattern)
    if anchor_before is not None:
        updates.append("anchor_before = ?")
        params.append(anchor_before)
    if anchor_after is not None:
        updates.append("anchor_after = ?")
        params.append(anchor_after)
    if max_length is not None:
        updates.append("max_length = ?")
        params.append(max_length)
    if required is not None:
        updates.append("required = ?")
        params.append(1 if required else 0)
    if not updates:
        return False
    params.append(rule_id)
    cursor = conn.execute(
        f"UPDATE rules SET {', '.join(updates)} WHERE id = ?",
        params,
    )
    conn.commit()
    return cursor.rowcount > 0


def set_active(conn: sqlite3.Connection, rule_id: int, active: bool) -> bool:
    """Toggle rule active status. Returns True if updated."""
    cursor = conn.execute("UPDATE rules SET active = ? WHERE id = ?", (1 if active else 0, rule_id))
    conn.commit()
    return cursor.rowcount > 0


def delete(conn: sqlite3.Connection, rule_id: int) -> bool:
    """Delete rule. Returns True if deleted."""
    cursor = conn.execute("DELETE FROM rules WHERE id = ?", (rule_id,))
    conn.commit()
    return cursor.rowcount > 0


def get_by_id(conn: sqlite3.Connection, rule_id: int) -> Optional[RuleRow]:
    """Get rule by id."""
    row = conn.execute(
        "SELECT id, name, file_type, regex_pattern, anchor_before, anchor_after, max_length, required, active FROM rules WHERE id = ?",
        (rule_id,),
    ).fetchone()
    if row is None:
        return None
    return RuleRow(
        id=row[0],
        name=row[1],
        file_type=row[2],
        regex_pattern=row[3],
        anchor_before=row[4],
        anchor_after=row[5],
        max_length=row[6],
        required=bool(row[7]),
        active=bool(row[8]),
    )


def list_all(
    conn: sqlite3.Connection,
    active_only: Optional[bool] = None,
    file_type: Optional[str] = None,
) -> list[RuleRow]:
    """List rules, optionally filtered by active and file_type."""
    query = "SELECT id, name, file_type, regex_pattern, anchor_before, anchor_after, max_length, required, active FROM rules WHERE 1=1"
    params = []
    if active_only is not None:
        query += " AND active = ?"
        params.append(1 if active_only else 0)
    if file_type is not None:
        query += " AND (file_type IS NULL OR file_type = ?)"
        params.append(file_type)
    query += " ORDER BY id"
    rows = conn.execute(query, params).fetchall()
    return [
        RuleRow(
            id=r[0],
            name=r[1],
            file_type=r[2],
            regex_pattern=r[3],
            anchor_before=r[4],
            anchor_after=r[5],
            max_length=r[6],
            required=bool(r[7]),
            active=bool(r[8]),
        )
        for r in rows
    ]
