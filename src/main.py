"""
Main entry: init folders and DB, CLI for spikes, optional minimal FastAPI /health.
"""

import argparse
import sys
from pathlib import Path

# Ensure project root is on path when run as python -m src.main
_PROJECT_ROOT = Path(__file__).resolve().parent.parent
if str(_PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(_PROJECT_ROOT))

from src.config import ensure_dirs, get_db_path, get_schema_path, get_log_path
from src.logging.logger import get_logger, set_default_log_path
from src.db.database import init_db

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
        # Still ensure schema is run for fresh installs; idempotent CREATE IF NOT EXISTS
        init_db(str(db_path), str(schema_path))


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


def main() -> None:
    ensure_dirs()
    set_default_log_path(get_log_path())
    global LOG
    LOG = get_logger("main")

    parser = argparse.ArgumentParser(description="Document Ingestion – Week 3")
    parser.add_argument("--spike", choices=["pdf", "docx", "xlsx", "regex"], help="Run a feasibility spike")
    parser.add_argument("path", nargs="?", default=None, help="File path for pdf/docx/xlsx spike")
    parser.add_argument("--serve", action="store_true", help="Start minimal FastAPI server with /health")
    args = parser.parse_args()

    if args.serve:
        _ensure_db()
        from fastapi import FastAPI
        import uvicorn
        app = FastAPI(title="Document Ingestion (Week 3)")

        @app.get("/health")
        def health():
            return {"status": "ok"}

        uvicorn.run(app, host="127.0.0.1", port=8000)
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

    # No --spike and no --serve: init only (DB + folders), then optional message
    _ensure_db()
    LOG.info("Folders and DB initialized. Use --spike or --serve.")


if __name__ == "__main__":
    main()
