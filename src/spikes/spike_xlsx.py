"""
Feasibility spike: read XLSX path, print sample of extracted cell values (first sheet).
"""

import sys
from pathlib import Path

_ROOT = Path(__file__).resolve().parent.parent.parent
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from src.parsers.xlsx_parser import parse_xlsx


def run(file_path: str) -> None:
    path = Path(file_path)
    if not path.exists():
        print(f"File not found: {file_path}", file=sys.stderr)
        sys.exit(1)
    text = parse_xlsx(file_path)
    lines = text.strip().split("\n")[:20]  # first 20 rows
    print("--- Sample of first sheet (first 20 rows) ---")
    for i, line in enumerate(lines, 1):
        print(f"{i}: {line}")
    if text.count("\n") >= 20:
        print("... (more rows)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python -m src.spikes.spike_xlsx <path/to/file.xlsx>", file=sys.stderr)
        sys.exit(1)
    run(sys.argv[1])
