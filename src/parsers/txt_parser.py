"""
TXT parser: read plain text file.
"""

from pathlib import Path


def parse_txt(file_path: str) -> str:
    """
    Read plain text file at file_path. Returns content as string (UTF-8).
    """
    path = Path(file_path)
    if not path.exists() or not path.is_file():
        raise FileNotFoundError(f"TXT not found: {file_path}")
    return path.read_text(encoding="utf-8", errors="replace")
