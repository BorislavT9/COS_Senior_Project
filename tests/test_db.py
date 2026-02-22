"""
Tests for database: schema init, insert and query operations.
"""

import os
import tempfile
from pathlib import Path

import pytest

from src.db import database
from src.db.documents_repo import insert as doc_insert, find_by_path_and_hash, list_all as docs_list
from src.db.rules_repo import insert as rule_insert, get_by_id as rule_get, list_all as rules_list
from src.db.extractions_repo import insert as ext_insert, list_all as ext_list
from src.db.logs_repo import insert as log_insert, list_all as logs_list


@pytest.fixture
def temp_db():
    """Create a temporary database for testing."""
    fd, path = tempfile.mkstemp(suffix=".db")
    os.close(fd)
    schema_path = Path(__file__).resolve().parent.parent / "schema.sql"
    database.init_db(path, str(schema_path))
    yield path
    try:
        os.unlink(path)
    except OSError:
        pass


def test_schema_init(temp_db: str) -> None:
    """Schema creates all required tables."""
    conn = database.get_connection(temp_db)
    tables = conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table'"
    ).fetchall()
    names = [t[0] for t in tables]
    assert "documents" in names
    assert "rules" in names
    assert "extractions" in names
    assert "logs" in names
    conn.close()


def test_documents_insert_and_query(temp_db: str) -> None:
    """Insert document and query by path/hash."""
    conn = database.get_connection(temp_db)
    doc_insert(conn, "/path/to/file.pdf", "abc123", ".pdf", status="SUCCESS")
    doc = find_by_path_and_hash(conn, "/path/to/file.pdf", "abc123")
    assert doc is not None
    assert doc.file_path == "/path/to/file.pdf"
    assert doc.file_hash == "abc123"
    assert doc.status == "SUCCESS"
    conn.close()


def test_rules_insert_and_query(temp_db: str) -> None:
    """Insert rule and retrieve."""
    conn = database.get_connection(temp_db)
    rid = rule_insert(conn, "test_rule", r"\d+", required=True)
    row = rule_get(conn, rid)
    assert row is not None
    assert row.name == "test_rule"
    assert row.regex_pattern == r"\d+"
    assert row.required is True
    conn.close()


def test_extractions_insert_and_list(temp_db: str) -> None:
    """Insert document, rule, extraction; list extractions."""
    conn = database.get_connection(temp_db)
    doc_id = doc_insert(conn, "/f.pdf", "h1", ".pdf", status="SUCCESS")
    rule_id = rule_insert(conn, "r1", r"\d+")
    ext_insert(conn, doc_id, rule_id, "SUCCESS", extracted_value="42")
    rows = ext_list(conn, document_id=doc_id)
    assert len(rows) == 1
    assert rows[0].extracted_value == "42"
    assert rows[0].status == "SUCCESS"
    conn.close()


def test_logs_insert_and_list(temp_db: str) -> None:
    """Insert log and list."""
    conn = database.get_connection(temp_db)
    log_insert(conn, "INFO", "test message", "ctx")
    rows = logs_list(conn, limit=10)
    assert len(rows) >= 1
    assert any(r.message == "test message" for r in rows)
    conn.close()
