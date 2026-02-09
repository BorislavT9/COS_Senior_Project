"""
PDF parser: extract text using pypdf.
"""

from pathlib import Path

from pypdf import PdfReader


def parse_pdf(file_path: str) -> str:
    """
    Extract text from PDF at file_path. Returns concatenated text from all pages.
    """
    path = Path(file_path)
    if not path.exists() or not path.is_file():
        raise FileNotFoundError(f"PDF not found: {file_path}")
    reader = PdfReader(str(path))
    parts = []
    for page in reader.pages:
        text = page.extract_text()
        if text:
            parts.append(text)
    return "\n".join(parts)
