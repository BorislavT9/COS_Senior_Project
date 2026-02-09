"""
Abstract-like base interface for parsers: parse(file_path) -> str.
"""


def parse(file_path: str) -> str:
    """
    Base interface: parse file at file_path and return extracted text.
    Implementations live in pdf_parser, docx_parser, txt_parser, xlsx_parser.
    Use factory.parse_file() to dispatch by extension.
    """
    raise NotImplementedError("Use parse_file() from parsers.factory or a concrete parser module.")
