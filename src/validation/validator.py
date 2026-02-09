"""
Validation: check ExtractionCandidate against Rule (empty, max_length, required).
"""

from src.rules.models import ExtractionCandidate, Rule


def validate(candidate: ExtractionCandidate, rule: Rule) -> tuple[bool, str]:
    """
    Validate candidate against rule. Checks: empty, max_length if set, required logic.
    Returns (is_valid, error_message). error_message is "" when valid.
    """
    value = candidate.normalized_value

    if rule.required and (value is None or value == ""):
        return False, "required field is empty"

    if rule.max_length is not None and len(value) > rule.max_length:
        return False, f"value length {len(value)} exceeds max_length {rule.max_length}"

    return True, ""
