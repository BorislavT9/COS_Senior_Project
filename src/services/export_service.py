"""
Export service: export extraction results (e.g. CSV, JSON).
"""

from typing import Any

from src.config import get_db_path
from src.db import database
from src.db.extractions_repo import list_all as extractions_list
from src.db.documents_repo import get_by_id as doc_get
from src.db.rules_repo import get_by_id as rule_get


def get_results_export(
    document_id: int | None = None,
    rule_id: int | None = None,
    status: str | None = None,
) -> list[dict[str, Any]]:
    """
    Return extraction results as list of dicts for export (JSON/CSV).
    """
    conn = database.get_connection(str(get_db_path()))
    try:
        rows = extractions_list(conn, document_id=document_id, rule_id=rule_id, status=status, limit=10000)
        out = []
        for r in rows:
            doc = doc_get(conn, r.document_id)
            rule_row = rule_get(conn, r.rule_id)
            out.append({
                "extraction_id": r.id,
                "document_id": r.document_id,
                "document_path": doc.file_path if doc else None,
                "rule_id": r.rule_id,
                "rule_name": rule_row.name if rule_row else None,
                "extracted_value": r.extracted_value,
                "status": r.status,
                "error_message": r.error_message,
                "created_at": r.created_at,
            })
        return out
    finally:
        conn.close()
