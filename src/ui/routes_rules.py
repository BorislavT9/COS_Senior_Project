"""
Rules API routes: CRUD, toggle active.
"""

import re
from typing import Optional

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel

from src.config import get_db_path
from src.db import database
from src.db.rules_repo import (
    delete as rule_delete,
    get_by_id,
    insert as rule_insert,
    list_all as rules_list,
    set_active,
    to_rule,
    update as rule_update,
)

router = APIRouter(prefix="/rules", tags=["rules"])


class RuleCreate(BaseModel):
    name: str
    regex_pattern: str
    file_type: Optional[str] = None
    anchor_before: Optional[str] = None
    anchor_after: Optional[str] = None
    max_length: Optional[int] = None
    required: bool = False
    active: bool = True


class RuleUpdate(BaseModel):
    name: Optional[str] = None
    file_type: Optional[str] = None
    regex_pattern: Optional[str] = None
    anchor_before: Optional[str] = None
    anchor_after: Optional[str] = None
    max_length: Optional[int] = None
    required: Optional[bool] = None


def _validate_regex(pattern: str) -> None:
    """Validate regex pattern. Raises ValueError if invalid."""
    try:
        re.compile(pattern)
    except re.error as e:
        raise ValueError(f"Invalid regex: {e}")


@router.get("")
def list_rules(
    active_only: Optional[bool] = None,
    file_type: Optional[str] = None,
) -> list[dict]:
    """List rules with optional filters."""
    conn = database.get_connection(str(get_db_path()))
    try:
        rows = rules_list(conn, active_only=active_only, file_type=file_type)
        return [
            {
                "id": r.id,
                "name": r.name,
                "file_type": r.file_type,
                "regex_pattern": r.regex_pattern,
                "anchor_before": r.anchor_before,
                "anchor_after": r.anchor_after,
                "max_length": r.max_length,
                "required": r.required,
                "active": r.active,
            }
            for r in rows
        ]
    finally:
        conn.close()


@router.post("")
def create_rule(body: RuleCreate) -> dict:
    """Create rule. Validates regex pattern."""
    _validate_regex(body.regex_pattern)
    conn = database.get_connection(str(get_db_path()))
    try:
        rid = rule_insert(
            conn,
            name=body.name,
            regex_pattern=body.regex_pattern,
            file_type=body.file_type,
            anchor_before=body.anchor_before,
            anchor_after=body.anchor_after,
            max_length=body.max_length,
            required=body.required,
            active=body.active,
        )
        row = get_by_id(conn, rid)
        return {
            "id": row.id,
            "name": row.name,
            "file_type": row.file_type,
            "regex_pattern": row.regex_pattern,
            "anchor_before": row.anchor_before,
            "anchor_after": row.anchor_after,
            "max_length": row.max_length,
            "required": row.required,
            "active": row.active,
        }
    finally:
        conn.close()


@router.put("/{rule_id:int}")
def update_rule(rule_id: int, body: RuleUpdate) -> dict:
    """Update rule. Validates regex if provided."""
    if body.regex_pattern is not None:
        _validate_regex(body.regex_pattern)
    conn = database.get_connection(str(get_db_path()))
    try:
        existing = get_by_id(conn, rule_id)
        if existing is None:
            raise HTTPException(status_code=404, detail="Rule not found")
        rule_update(
            conn,
            rule_id,
            name=body.name,
            file_type=body.file_type,
            regex_pattern=body.regex_pattern,
            anchor_before=body.anchor_before,
            anchor_after=body.anchor_after,
            max_length=body.max_length,
            required=body.required,
        )
        row = get_by_id(conn, rule_id)
        return {
            "id": row.id,
            "name": row.name,
            "file_type": row.file_type,
            "regex_pattern": row.regex_pattern,
            "anchor_before": row.anchor_before,
            "anchor_after": row.anchor_after,
            "max_length": row.max_length,
            "required": row.required,
            "active": row.active,
        }
    finally:
        conn.close()


@router.delete("/{rule_id:int}")
def delete_rule(rule_id: int) -> dict:
    """Delete rule."""
    conn = database.get_connection(str(get_db_path()))
    try:
        if get_by_id(conn, rule_id) is None:
            raise HTTPException(status_code=404, detail="Rule not found")
        rule_delete(conn, rule_id)
        return {"deleted": rule_id}
    finally:
        conn.close()


@router.post("/{rule_id:int}/toggle")
def toggle_rule(rule_id: int) -> dict:
    """Toggle rule active status."""
    conn = database.get_connection(str(get_db_path()))
    try:
        row = get_by_id(conn, rule_id)
        if row is None:
            raise HTTPException(status_code=404, detail="Rule not found")
        new_active = not row.active
        set_active(conn, rule_id, new_active)
        return {"id": rule_id, "active": new_active}
    finally:
        conn.close()
