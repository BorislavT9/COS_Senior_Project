"""
Pipeline test: given sample text file and rule, run ingestion and confirm extraction row saved.
"""

import os
import tempfile
from pathlib import Path

import pytest

from src.db import database
from src.db.rules_repo import insert as rule_insert
from src.db.extractions_repo import list_all as ext_list
from src.services.ingestion_service import run_scan


@pytest.fixture
def sample_txt(tmp_path: Path) -> Path:
    """Create a sample text file."""
    f = tmp_path / "invoice.txt"
    f.write_text(
        "Sample invoice.\nInvoice #INV-2024-001\nDate: 2024-01-15",
        encoding="utf-8",
    )
    return f


@pytest.fixture
def pipeline_db(tmp_path: Path):
    """Create DB with schema and a rule, use tmp_path for DB file."""
    db_path = tmp_path / "pipeline.db"
    schema_path = Path(__file__).resolve().parent.parent / "schema.sql"
    database.init_db(str(db_path), str(schema_path))
    conn = database.get_connection(str(db_path))
    rule_insert(
        conn,
        "invoice_number",
        r"INV-\d{4}-\d{3}",
        file_type=".txt",
        required=False,
        active=True,
    )
    conn.close()
    return db_path


def test_pipeline_extraction(sample_txt: Path, pipeline_db: Path, monkeypatch) -> None:
    """
    Run scan on sample_txt with invoice rule; verify extraction row is saved.
    """
    # Patch get_db_path to use our temp DB
    def _get_db():
        return pipeline_db

    monkeypatch.setattr("src.services.ingestion_service.get_db_path", _get_db)

    watch_dir = str(sample_txt.parent)
    summary = run_scan(watch_dir)

    assert summary["processed"] >= 1
    assert summary["extractions"] >= 1

    conn = database.get_connection(str(pipeline_db))
    exts = ext_list(conn, limit=100)
    conn.close()

    inv_exts = [e for e in exts if e.extracted_value and "INV-" in (e.extracted_value or "")]
    assert len(inv_exts) >= 1
    assert any("INV-2024-001" in (e.extracted_value or "") for e in inv_exts)
