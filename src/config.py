"""
Central configuration for the Document Ingestion and Extraction system.
Creates data/, logs/, and sample_docs/ at runtime if missing.
"""

from pathlib import Path

# Paths (defaults)
WATCH_DIR: str = "./sample_docs"
DB_PATH: str = "./data/app.db"
LOG_PATH: str = "./logs/app.log"
SCHEMA_PATH: str = "schema.sql"
DATA_STORE_DIR: str = "./data_store"  # JSON-based store directory

# Resolve relative to project root (parent of src)
_PROJECT_ROOT = Path(__file__).resolve().parent.parent


def ensure_dirs() -> None:
    """Create data/, logs/, sample_docs/, and data_store/ if they do not exist."""
    for name in ("data", "logs", "sample_docs", "data_store"):
        d = _PROJECT_ROOT / name
        d.mkdir(parents=True, exist_ok=True)


def get_watch_dir() -> Path:
    """Return resolved watch directory path."""
    return _PROJECT_ROOT / WATCH_DIR.lstrip("./")


def get_db_path() -> Path:
    """Return resolved database file path."""
    return _PROJECT_ROOT / DB_PATH.lstrip("./")


def get_log_path() -> Path:
    """Return resolved log file path."""
    return _PROJECT_ROOT / LOG_PATH.lstrip("./")


def get_schema_path() -> Path:
    """Return resolved schema.sql path (project root)."""
    return _PROJECT_ROOT / SCHEMA_PATH


def get_data_store_dir() -> Path:
    """Return resolved data_store directory path."""
    return _PROJECT_ROOT / DATA_STORE_DIR.lstrip("./")


def get_search_history_path() -> Path:
    """Return resolved search history file path."""
    return _PROJECT_ROOT / "data_store" / "search_history.json"
