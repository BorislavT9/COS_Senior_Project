"""
Feasibility spike: read PDF path, print first 1000 chars of extracted text.
"""

import sys
from pathlib import Path

# Project root on path when run as python -m src.spikes.spike_pdf
_ROOT = Path(__file__).resolve().parent.parent.parent
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from src.parsers.pdf_parser import parse_pdf


def run(file_path: str) -> None:
    path = Path(file_path)
    if not path.exists():
        print(f"File not found: {file_path}", file=sys.stderr)
        sys.exit(1)
    text = parse_pdf(file_path)
    sample = text[:1000] if len(text) >= 1000 else text
    print("--- First 1000 chars of extracted text ---")
    print(sample)
    if len(text) > 1000:
        print("... (truncated)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python -m src.spikes.spike_pdf <path/to/file.pdf>", file=sys.stderr)
        sys.exit(1)
    run(sys.argv[1])
