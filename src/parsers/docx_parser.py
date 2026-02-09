"""
DOCX parser: extract text using python-docx.
"""

from pathlib import Path

from docx import Document


def parse_docx(file_path: str) -> str:
    """
    Extract text from DOCX at file_path. Returns text from all paragraphs.
    """
    path = Path(file_path)
    if not path.exists() or not path.is_file():
        raise FileNotFoundError(f"DOCX not found: {file_path}")
    doc = Document(str(path))
    return "\n".join(p.text for p in doc.paragraphs if p.text.strip())
