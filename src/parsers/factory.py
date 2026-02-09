"""
Parser factory: choose parser by file extension and return extracted text.
"""

from pathlib import Path

from . import pdf_parser
from . import docx_parser
from . import txt_parser
from . import xlsx_parser

_EXT_MAP = {
    ".pdf": pdf_parser.parse_pdf,
    ".docx": docx_parser.parse_docx,
    ".txt": txt_parser.parse_txt,
    ".xlsx": xlsx_parser.parse_xlsx,
}


def parse_file(file_path: str) -> str:
    """
    Select parser by file extension and return extracted text.
    Raises ValueError if extension is not supported.
    """
    ext = Path(file_path).suffix.lower()
    parser_fn = _EXT_MAP.get(ext)
    if parser_fn is None:
        raise ValueError(f"Unsupported file type: {ext} (path: {file_path})")
    return parser_fn(file_path)
