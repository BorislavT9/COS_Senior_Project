"""
Scanner: list files recursively and compute file hash (SHA-256).
"""

import hashlib
from pathlib import Path


def scan_directory(path: str) -> list[str]:
    """
    List all files under path recursively. Returns list of absolute file paths as strings.
    """
    p = Path(path)
    if not p.exists():
        return []
    if not p.is_dir():
        return [str(p.resolve())]
    return [str(f.resolve()) for f in p.rglob("*") if f.is_file()]


def compute_file_hash(path: str) -> str:
    """
    Compute SHA-256 hash of file at path. Returns hex digest string.
    """
    p = Path(path)
    if not p.exists() or not p.is_file():
        raise FileNotFoundError(f"File not found: {path}")
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()
