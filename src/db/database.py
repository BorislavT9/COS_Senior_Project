"""
SQLite database utilities: init from schema, get connection.
"""

import sqlite3
from pathlib import Path


def init_db(db_path: str, schema_path: str) -> None:
    """
    Create database file and tables by executing schema.sql.
    Creates parent directory if missing.
    """
    path = Path(db_path)
    path.parent.mkdir(parents=True, exist_ok=True)
    schema = Path(schema_path)
    if not schema.exists():
        raise FileNotFoundError(f"Schema file not found: {schema_path}")

    with open(schema, "r", encoding="utf-8") as f:
        sql = f.read()

    conn = sqlite3.connect(str(path))
    try:
        conn.executescript(sql)
        conn.commit()
    finally:
        conn.close()


def get_connection(db_path: str) -> sqlite3.Connection:
    """Return a new SQLite connection to the given path."""
    Path(db_path).parent.mkdir(parents=True, exist_ok=True)
    return sqlite3.connect(str(db_path))
