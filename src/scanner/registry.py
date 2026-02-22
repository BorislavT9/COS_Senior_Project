"""
File type registry for scanner: supported extensions and detection.
"""

from pathlib import Path

# Supported extensions (lowercase)
SUPPORTED_EXTENSIONS = {".pdf", ".docx", ".txt", ".xlsx"}


def get_file_type(file_path: str) -> str:
    """Return file type (extension) for supported files, or empty string."""
    ext = Path(file_path).suffix.lower()
    return ext if ext in SUPPORTED_EXTENSIONS else ""


def is_supported(file_path: str) -> bool:
    """Return True if file is supported (PDF, DOCX, TXT, XLSX)."""
    return get_file_type(file_path) != ""


def filter_supported(paths: list[str]) -> list[str]:
    """Filter list of paths to only supported file types."""
    return [p for p in paths if is_supported(p)]
