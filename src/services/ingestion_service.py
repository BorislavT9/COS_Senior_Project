"""
Ingestion service: scan directory, parse files, extract with rules, validate, persist.
Uses ExtractionStore (AVL tree indexes) as primary storage, with optional DB logging.
"""

import uuid
from datetime import datetime
from pathlib import Path
from typing import Optional

from src.config import get_db_path, get_data_store_dir
from src.db import database
from src.db.logs_repo import insert as log_insert
from src.db.rules_repo import list_all as rules_list, to_rule
from src.logging.logger import get_logger
from src.parsers.factory import parse_file
from src.rules.engine import apply_rules
from src.scanner.registry import filter_supported, get_file_type, is_supported
from src.scanner.scanner import compute_file_hash, scan_directory
from src.store.models import DocumentRecord, ExtractionResult
from src.store.store import ExtractionStore
from src.validation.normalizer import normalize_value
from src.validation.validator import validate

LOG = get_logger("ingestion")


def run_scan(watch_dir: str, store: Optional[ExtractionStore] = None) -> dict:
    """
    Run full scan: discover files, process new/changed, extract, validate, persist to ExtractionStore.
    Returns summary dict with processed, skipped, failed, extractions counts.
    
    Args:
        watch_dir: Directory to scan
        store: ExtractionStore instance (if None, creates new and loads from disk)
    
    Returns:
        Summary dict with counts and errors
    """
    # Load or create store
    if store is None:
        store = ExtractionStore()
        store_dir = get_data_store_dir()
        if (store_dir / "store.json").exists():
            store.load(store_dir)
            LOG.info("Loaded existing store from %s", store_dir)
    
    # Optional: log to DB if available
    db_path = str(get_db_path())
    conn = None
    try:
        conn = database.get_connection(db_path)
        log_insert(conn, "INFO", f"Scan started: watch_dir={watch_dir}", watch_dir)
    except Exception:
        LOG.warning("Could not connect to DB for logging, continuing without DB logs")
        conn = None

    summary = {"processed": 0, "skipped": 0, "failed": 0, "extractions": 0, "errors": []}

    try:
        paths = scan_directory(watch_dir)
        supported = filter_supported(paths)

        for file_path in supported:
            try:
                file_hash = compute_file_hash(file_path)
                existing_doc = store.find_by_path_and_hash(file_path, file_hash)
                if existing_doc is not None:
                    summary["skipped"] += 1
                    continue

                file_type = get_file_type(file_path)
                doc_id = str(uuid.uuid4())
                processed_at = datetime.now().isoformat()

                # Parse
                try:
                    text = parse_file(file_path)
                except Exception as e:
                    # Create record with error
                    record = DocumentRecord(
                        doc_id=doc_id,
                        file_path=file_path,
                        file_type=file_type,
                        file_hash=file_hash,
                        processed_at=processed_at,
                        status="PARSE_FAILED",
                        error_message=str(e),
                    )
                    store.add_document(record)
                    if conn:
                        log_insert(conn, "ERROR", f"Parse failed: {file_path}", str(e))
                    summary["failed"] += 1
                    continue

                # Load rules (active, for this file_type or global/null)
                if conn:
                    rule_rows = rules_list(conn, active_only=True, file_type=file_type)
                    rules = [to_rule(r) for r in rule_rows]
                else:
                    # Fallback: try to get rules from DB anyway
                    try:
                        db_conn = database.get_connection(db_path)
                        rule_rows = rules_list(db_conn, active_only=True, file_type=file_type)
                        rules = [to_rule(r) for r in rule_rows]
                        db_conn.close()
                    except Exception:
                        rules = []
                        LOG.warning("Could not load rules, skipping extraction")

                # Apply rules and extract
                extracted = {}
                extracted_raw = {}
                extraction_results = []

                if rules:
                    candidates = apply_rules(text, rules)
                    candidates_by_rule = {c.rule_name: c for c in candidates}

                    for rule in rules:
                        candidate = candidates_by_rule.get(rule.name)
                        if candidate is None:
                            if rule.required:
                                extraction_results.append(
                                    ExtractionResult(
                                        field=rule.name,
                                        raw_value=None,
                                        normalized_value=None,
                                        status="NO_MATCH",
                                        error="required rule produced no match",
                                    )
                                )
                                summary["extractions"] += 1
                            continue

                        raw_val = candidate.raw_match
                        normalized_val = normalize_value(candidate.normalized_value)
                        
                        # Validate
                        is_valid, err_msg = validate(candidate, rule)
                        if not is_valid:
                            extraction_results.append(
                                ExtractionResult(
                                    field=rule.name,
                                    raw_value=raw_val,
                                    normalized_value=normalized_val,
                                    status="INVALID",
                                    error=err_msg,
                                )
                            )
                        else:
                            # SUCCESS: store in extracted dict and index
                            extracted[rule.name] = normalized_val
                            extracted_raw[rule.name] = raw_val
                            store.index_extraction(rule.name, normalized_val, doc_id)
                            extraction_results.append(
                                ExtractionResult(
                                    field=rule.name,
                                    raw_value=raw_val,
                                    normalized_value=normalized_val,
                                    status="SUCCESS",
                                )
                            )
                        summary["extractions"] += 1

                # Create document record
                record = DocumentRecord(
                    doc_id=doc_id,
                    file_path=file_path,
                    file_type=file_type,
                    file_hash=file_hash,
                    processed_at=processed_at,
                    status="SUCCESS",
                    extracted=extracted,
                    extracted_raw=extracted_raw,
                )
                store.add_document(record)
                summary["processed"] += 1

            except Exception as e:
                summary["failed"] += 1
                summary["errors"].append(f"{file_path}: {e}")
                if conn:
                    log_insert(conn, "ERROR", f"Processing failed: {file_path}", str(e))
                LOG.error("Processing failed: %s: %s", file_path, e)

        if conn:
            log_insert(conn, "INFO", f"Scan completed: processed={summary['processed']}, skipped={summary['skipped']}, failed={summary['failed']}", None)
    finally:
        if conn:
            conn.close()

    return summary


