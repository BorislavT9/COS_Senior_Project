"""
Feasibility spike: load small sample text, apply a sample Rule, print result.
"""

import sys
from pathlib import Path

_ROOT = Path(__file__).resolve().parent.parent.parent
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from src.rules.models import Rule
from src.rules.engine import apply_rules

SAMPLE_TEXT = """
Invoice #INV-2024-001
Date: 2024-01-15
Customer: Acme Corp
Total: $1,234.56
---
Section A
Order-ID: ORD-9999
Section B
"""


def run() -> None:
    rule = Rule(
        name="invoice_id",
        regex_pattern=r"#?(INV-\d{4}-\d{3})",
        anchor_before="Invoice",
        anchor_after="Date",
    )
    rules = [rule]
    candidates = apply_rules(SAMPLE_TEXT, rules)
    print("--- Sample text (snippet) ---")
    print(SAMPLE_TEXT[:300])
    print("--- Rule: invoice_id (anchor_before=Invoice, anchor_after=Date) ---")
    print("Matches:", candidates)
    for c in candidates:
        print(f"  {c.rule_name}: {c.normalized_value!r} [{c.start_idx}:{c.end_idx}]")


if __name__ == "__main__":
    run()
