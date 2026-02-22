"""
Main entry: CLI commands for ingestion, querying, listing, and export.
Uses ExtractionStore (AVL tree indexes) as primary storage.
"""

import argparse
import json
import os
import sys
from pathlib import Path

# Ensure project root is on path when run as python -m src.main
_PROJECT_ROOT = Path(__file__).resolve().parent.parent
if str(_PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(_PROJECT_ROOT))

from src.config import (
    ensure_dirs,
    get_db_path,
    get_schema_path,
    get_log_path,
    get_watch_dir,
    get_data_store_dir,
    get_search_history_path,
)
from src.logging.logger import get_logger, set_default_log_path
from src.db.database import init_db
from src.store.store import ExtractionStore
from src.store.search_history import SearchHistory
from src.services.ingestion_service import run_scan
from src.services.report_service import generate_html_report
from src.services.keyword_search_service import search_keyword_in_directory

LOG = get_logger("main")


def _ensure_db() -> None:
    """Create data dir and DB from schema if DB does not exist."""
    ensure_dirs()
    db_path = get_db_path()
    schema_path = get_schema_path()
    if not db_path.exists():
        LOG.info("Initializing database at %s", db_path)
        init_db(str(db_path), str(schema_path))
    else:
        init_db(str(db_path), str(schema_path))


def _run_scan(watch_dir: str) -> None:
    """Run ingestion scan (legacy, uses store)."""
    store = _load_store()
    summary = run_scan(watch_dir, store)
    
    # Save store
    store_dir = get_data_store_dir()
    store.save(store_dir)
    
    print("Scan complete:")
    print(f"  Processed: {summary['processed']}")
    print(f"  Skipped (unchanged): {summary['skipped']}")
    print(f"  Failed: {summary['failed']}")
    print(f"  Extractions: {summary['extractions']}")
    if summary.get("errors"):
        for e in summary["errors"]:
            print(f"  Error: {e}")


def _run_demo() -> None:
    """Deterministic demo: insert sample rules, scan sample_docs, print summary."""
    from src.config import get_db_path
    from src.db import database
    from src.db.rules_repo import insert as rule_insert, list_all as rules_list

    _ensure_db()
    db_path = str(get_db_path())
    conn = database.get_connection(db_path)

    # Insert sample rules if none exist
    existing = rules_list(conn, active_only=None)
    if not existing:
        rule_insert(conn, "invoice_number", r"INV-\d{4}-\d{3}", file_type=".txt", required=False, active=True)
        rule_insert(conn, "date_field", r"\d{4}-\d{2}-\d{2}", file_type=".txt", required=False, active=True)
        print("Inserted sample rules: invoice_number, date_field")
    else:
        print("Rules already exist, skipping rule insert")

    conn.close()

    # Run ingestion with store
    watch_dir = str(get_watch_dir())
    print(f"Scanning: {watch_dir}")
    store = _load_store()
    summary = run_scan(watch_dir, store)
    
    # Save store
    store_dir = get_data_store_dir()
    store.save(store_dir)
    
    print("\n--- Demo Scan Summary ---")
    print(f"Processed: {summary['processed']}")
    print(f"Skipped: {summary['skipped']}")
    print(f"Failed: {summary['failed']}")
    print(f"Extractions: {summary['extractions']}")
    print(f"\nStore saved to: {store_dir}")


def _run_spike_pdf(file_path: str) -> None:
    from src.spikes.spike_pdf import run as run_pdf
    run_pdf(file_path)


def _run_spike_docx(file_path: str) -> None:
    from src.spikes.spike_docx import run as run_docx
    run_docx(file_path)


def _run_spike_xlsx(file_path: str) -> None:
    from src.spikes.spike_xlsx import run as run_xlsx
    run_xlsx(file_path)


def _run_spike_regex() -> None:
    from src.spikes.spike_regex import run as run_regex
    run_regex()


def _load_store() -> ExtractionStore:
    """Load ExtractionStore from disk."""
    store = ExtractionStore()
    store_dir = get_data_store_dir()
    if (store_dir / "store.json").exists():
        store.load(store_dir)
        LOG.info("Loaded store from %s", store_dir)
    else:
        LOG.info("No existing store found, starting fresh")
    return store


def _cmd_ingest(watch_dir: str) -> None:
    """Ingest documents from a directory."""
    store = _load_store()
    summary = run_scan(watch_dir, store)
    
    # Save store
    store_dir = get_data_store_dir()
    store.save(store_dir)
    LOG.info("Saved store to %s", store_dir)
    
    # Generate report
    report_path = store_dir / "report.html"
    generate_html_report(store, report_path)
    LOG.info("Generated report: %s", report_path)
    
    # Auto-open report on Windows
    if os.name == "nt":
        try:
            os.startfile(str(report_path))
        except Exception as e:
            LOG.warning("Could not auto-open report: %s", e)
    
    # Print summary
    print("\n=== Ingestion Summary ===")
    print(f"Processed: {summary['processed']}")
    print(f"Skipped (unchanged): {summary['skipped']}")
    print(f"Failed: {summary['failed']}")
    print(f"Total extractions: {summary['extractions']}")
    if summary.get("errors"):
        print("\nErrors:")
        for e in summary["errors"]:
            print(f"  - {e}")


def _cmd_query(field: str, equals: str = None, range_low: str = None, range_high: str = None) -> None:
    """Query documents by field value."""
    store = _load_store()
    
    if equals is not None:
        results = store.query_equals(field, equals)
        print(f"\nFound {len(results)} document(s) where {field} = '{equals}':")
        for doc in results:
            print(f"  - {doc.file_path} (ID: {doc.doc_id})")
            if doc.extracted:
                print(f"    Extracted: {doc.extracted}")
    elif range_low is not None and range_high is not None:
        results = store.query_range(field, range_low, range_high)
        print(f"\nFound {len(results)} document(s) where {field} in range ['{range_low}', '{range_high}']:")
        for doc in results:
            print(f"  - {doc.file_path} (ID: {doc.doc_id})")
            if doc.extracted:
                print(f"    Extracted: {doc.extracted}")
    else:
        print("Error: Must specify either --equals or --range with both low and high")
        sys.exit(1)


def _cmd_list(field: str) -> None:
    """List all unique values for a field."""
    store = _load_store()
    values = store.list_field_values(field)
    
    if not values:
        print(f"No values found for field '{field}'")
        return
    
    print(f"\nUnique values for field '{field}' ({len(values)} total):")
    for value in values:
        print(f"  - {value}")


def _display_statistics(store: ExtractionStore, history: SearchHistory) -> None:
    """Display statistics panel."""
    stats = history.get_statistics()
    total_docs = len(store.documents)
    
    print("\n" + "=" * 60)
    print(" " * 15 + "STATISTICS PANEL")
    print("=" * 60)
    print(f"  Total Documents Stored:     {total_docs}")
    print(f"  Total Searches Performed:  {stats['total_searches']}")
    print(f"  Total Files Found:          {stats['total_files_found']}")
    print(f"  Total Files Stored:         {stats['total_files_stored']}")
    print("=" * 60)


def _prompt_file_type() -> str:
    """Prompt user for file type."""
    print("\nSelect file type to search:")
    print("  1. PDF (.pdf)")
    print("  2. DOCX (.docx)")
    print("  3. TXT (.txt)")
    print("  4. XLSX (.xlsx)")
    print("  5. All types")
    
    while True:
        choice = input("\nEnter choice (1-5): ").strip()
        file_types = {
            "1": ".pdf",
            "2": ".docx",
            "3": ".txt",
            "4": ".xlsx",
            "5": None,  # All types
        }
        if choice in file_types:
            return file_types[choice]
        print("Invalid choice. Please enter 1-5.")


def _prompt_directory() -> str:
    """Prompt user for directory."""
    while True:
        directory = input("\nEnter directory/folder path to search: ").strip()
        if not directory:
            print("Directory cannot be empty. Please try again.")
            continue
        
        # Remove quotes if present
        directory = directory.strip('"').strip("'")
        
        path = Path(directory)
        if not path.exists():
            print(f"Directory does not exist: {directory}")
            retry = input("Try again? (y/n): ").strip().lower()
            if retry != 'y':
                return None
            continue
        
        if not path.is_dir():
            print(f"Path is not a directory: {directory}")
            retry = input("Try again? (y/n): ").strip().lower()
            if retry != 'y':
                return None
            continue
        
        return str(path.resolve())


def _prompt_keyword() -> str:
    """Prompt user for keyword."""
    while True:
        keyword = input("\nEnter keyword/symbol to search for: ").strip()
        if not keyword:
            print("Keyword cannot be empty. Please try again.")
            continue
        return keyword


def _interactive_search_new_files() -> None:
    """Interactive search for new files."""
    print("\n" + "=" * 60)
    print(" " * 15 + "SEARCH FOR NEW FILES")
    print("=" * 60)
    
    # Prompt for inputs
    file_type = _prompt_file_type()
    directory = _prompt_directory()
    
    if directory is None:
        print("Search cancelled.")
        return
    
    keyword = _prompt_keyword()
    
    print(f"\nSearching for '{keyword}' in {directory}...")
    if file_type:
        print(f"File type filter: {file_type}")
    else:
        print("File type filter: All types")
    
    # Perform search
    results = search_keyword_in_directory(directory, keyword, file_type)
    
    # Count files found
    files_found = len([r for r in results if r["status"] == "FOUND"])
    files_with_matches = [r for r in results if r["matches"] > 0]
    
    # Display results
    print("\n" + "=" * 60)
    print(" " * 20 + "SEARCH RESULTS")
    print("=" * 60)
    
    if files_with_matches:
        print(f"\nFound '{keyword}' in {files_found} file(s):\n")
        for result in files_with_matches:
            print(f"  ✓ {result['file_path']}")
            print(f"    Matches: {result['matches']} | Type: {result['file_type']}")
    else:
        print(f"\nNo files found containing '{keyword}'")
    
    # Show files checked
    print(f"\nTotal files checked: {len(results)}")
    
    # Store results in ExtractionStore and save search history
    store = _load_store()
    history = SearchHistory(get_search_history_path())
    
    # Ingest found files into store
    files_stored = 0
    if files_with_matches:
        print("\nStoring found files...")
        summary = run_scan(directory, store)
        files_stored = summary["processed"]
        
        # Save store
        store_dir = get_data_store_dir()
        store.save(store_dir)
        print(f"Stored {files_stored} file(s) in database.")
    
    # Save search to history
    history.add_search(
        file_type=file_type or "ALL",
        directory=directory,
        keyword=keyword,
        files_found=files_found,
        files_stored=files_stored,
    )
    
    # Display statistics
    _display_statistics(store, history)
    
    print("\nSearch completed!")


def _show_previous_searches() -> None:
    """Show previously searched files."""
    history = SearchHistory(get_search_history_path())
    searches = history.get_all_searches()
    
    print("\n" + "=" * 60)
    print(" " * 15 + "PREVIOUSLY SEARCHED FILES")
    print("=" * 60)
    
    if not searches:
        print("\nNo previous searches found.")
        return
    
    print(f"\nTotal searches: {len(searches)}\n")
    
    for i, search in enumerate(searches, 1):
        print(f"{i}. Search #{i}")
        print(f"   File Type:    {search.file_type}")
        print(f"   Directory:    {search.directory}")
        print(f"   Keyword:       {search.keyword}")
        print(f"   Files Found:   {search.files_found}")
        print(f"   Files Stored:  {search.files_stored}")
        print(f"   Date/Time:     {search.timestamp}")
        print()
    
    # Display statistics
    store = _load_store()
    _display_statistics(store, history)


def _interactive_menu() -> None:
    """Main interactive menu."""
    while True:
        print("\n" + "=" * 60)
        print(" " * 10 + "DOCUMENT SEARCH & EXTRACTION SYSTEM")
        print("=" * 60)
        print("\n  Main Menu:")
        print("  1. Search for new files")
        print("  2. Show previously searched files")
        print("  3. Exit")
        
        choice = input("\nSelect option (1-3): ").strip()
        
        if choice == "1":
            _interactive_search_new_files()
        elif choice == "2":
            _show_previous_searches()
        elif choice == "3":
            print("\nThank you for using the Document Search System. Goodbye!")
            break
        else:
            print("\nInvalid choice. Please enter 1, 2, or 3.")


def _cmd_export(format_type: str, output_path: str) -> None:
    """Export store data to JSON or CSV."""
    store = _load_store()
    output = Path(output_path)
    
    if format_type == "json":
        # Export as JSON
        export_data = {
            "documents": [
                {
                    "doc_id": doc.doc_id,
                    "file_path": doc.file_path,
                    "file_type": doc.file_type,
                    "file_hash": doc.file_hash,
                    "processed_at": doc.processed_at,
                    "status": doc.status,
                    "error_message": doc.error_message,
                    "extracted": doc.extracted,
                    "extracted_raw": doc.extracted_raw,
                }
                for doc in store.documents.values()
            ],
            "indexes": {
                field: [
                    {"key": key, "doc_ids": doc_ids}
                    for key, doc_ids in tree.inorder_items()
                ]
                for field, tree in store.indexes.items()
            }
        }
        
        with open(output, "w", encoding="utf-8") as f:
            json.dump(export_data, f, indent=2, ensure_ascii=False)
        
        print(f"Exported {len(store.documents)} documents to {output}")
    
    elif format_type == "csv":
        # Export as CSV (flattened extractions)
        import csv
        
        rows = []
        for doc in store.documents.values():
            if doc.extracted:
                for field, value in doc.extracted.items():
                    rows.append({
                        "doc_id": doc.doc_id,
                        "file_path": doc.file_path,
                        "file_type": doc.file_type,
                        "field": field,
                        "value": value,
                        "raw_value": doc.extracted_raw.get(field, ""),
                        "status": doc.status,
                        "processed_at": doc.processed_at,
                    })
        
        if rows:
            with open(output, "w", newline="", encoding="utf-8") as f:
                writer = csv.DictWriter(f, fieldnames=rows[0].keys())
                writer.writeheader()
                writer.writerows(rows)
            print(f"Exported {len(rows)} extraction rows to {output}")
        else:
            print("No extractions to export")
    else:
        print(f"Error: Unknown format '{format_type}'. Use 'json' or 'csv'")
        sys.exit(1)


def main() -> None:
    ensure_dirs()
    set_default_log_path(get_log_path())
    global LOG
    LOG = get_logger("main")

    parser = argparse.ArgumentParser(
        description="Document Ingestion & Extraction System (AVL Tree Indexed)",
        prog="python -m src.main"
    )
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")
    
    # Ingest command
    ingest_parser = subparsers.add_parser("ingest", help="Ingest documents from a directory")
    ingest_parser.add_argument("--watch_dir", type=str, default=None, help="Directory to scan (default: sample_docs)")
    
    # Query command
    query_parser = subparsers.add_parser("query", help="Query documents by field value")
    query_parser.add_argument("--field", type=str, required=True, help="Field name to query")
    query_parser.add_argument("--equals", type=str, default=None, help="Exact value to match")
    query_parser.add_argument("--range", nargs=2, metavar=("LOW", "HIGH"), help="Range query [low, high]")
    
    # List command
    list_parser = subparsers.add_parser("list", help="List unique values for a field")
    list_parser.add_argument("--field", type=str, required=True, help="Field name")
    
    # Export command
    export_parser = subparsers.add_parser("export", help="Export store data")
    export_parser.add_argument("--format", type=str, choices=["json", "csv"], required=True, help="Export format")
    export_parser.add_argument("--out", type=str, required=True, help="Output file path")
    
    # Legacy commands (for backward compatibility)
    parser.add_argument("--serve", action="store_true", help="Start FastAPI server with uvicorn")
    parser.add_argument("--scan", metavar="DIR", nargs="?", const="", default=None, help="[Legacy] Run scan")
    parser.add_argument("--demo", action="store_true", help="[Legacy] Run deterministic demo")
    parser.add_argument("--spike", choices=["pdf", "docx", "xlsx", "regex"], help="Run a feasibility spike")
    parser.add_argument("path", nargs="?", default=None, help="File path for pdf/docx/xlsx spike")
    
    args = parser.parse_args()

    # New CLI commands
    if args.command == "ingest":
        watch_dir = args.watch_dir if args.watch_dir else str(get_watch_dir())
        _cmd_ingest(watch_dir)
        return
    
    if args.command == "query":
        if args.range:
            range_low, range_high = args.range
        else:
            range_low, range_high = None, None
        _cmd_query(args.field, args.equals, range_low, range_high)
        return
    
    if args.command == "list":
        _cmd_list(args.field)
        return
    
    if args.command == "export":
        _cmd_export(args.format, args.out)
        return

    # Legacy commands
    if args.serve:
        _ensure_db()
        from src.ui.api import create_app
        import uvicorn
        app = create_app()
        uvicorn.run(app, host="127.0.0.1", port=8000)
        return

    if args.scan is not None:
        _ensure_db()
        watch_dir = args.scan if args.scan else str(get_watch_dir())
        _run_scan(watch_dir)
        return

    if args.demo:
        _run_demo()
        return

    if args.spike:
        if args.spike in ("pdf", "docx", "xlsx") and not args.path:
            LOG.error("Spike %s requires a file path.", args.spike)
            sys.exit(1)
        if args.spike == "pdf":
            _run_spike_pdf(args.path)
        elif args.spike == "docx":
            _run_spike_docx(args.path)
        elif args.spike == "xlsx":
            _run_spike_xlsx(args.path)
        elif args.spike == "regex":
            _run_spike_regex()
        return

    # No command: show interactive menu
    _interactive_menu()


if __name__ == "__main__":
    main()
