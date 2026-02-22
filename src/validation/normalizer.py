"""
Normalization utilities for extracted text values.
Deterministic normalization: trim, collapse whitespace, optional casing, punctuation cleanup.
"""

import re
from typing import Optional


def normalize_value(value: str, uppercase: bool = False, remove_punctuation: bool = False) -> str:
    """
    Normalize extracted value deterministically.
    
    Steps:
    1. Strip leading/trailing whitespace
    2. Collapse internal whitespace to single space
    3. Optional: convert to uppercase (for IDs)
    4. Optional: remove surrounding punctuation
    
    Args:
        value: The raw value to normalize
        uppercase: If True, convert to uppercase (default: False)
        remove_punctuation: If True, remove surrounding punctuation (default: False)
    
    Returns:
        Normalized string
    
    Time complexity: O(n) where n is length of value
    """
    if value is None or not isinstance(value, str):
        return ""
    
    # Step 1: Strip
    s = value.strip()
    
    # Step 2: Collapse whitespace
    s = re.sub(r"\s+", " ", s)
    
    # Step 3: Optional uppercase
    if uppercase:
        s = s.upper()
    
    # Step 4: Optional punctuation removal (only surrounding, not internal)
    if remove_punctuation:
        s = re.sub(r"^[^\w\s]+|[^\w\s]+$", "", s)
    
    return s


def normalize_field_name(name: str) -> str:
    """
    Normalize field name to a safe identifier.
    
    Args:
        name: Raw field name
    
    Returns:
        Safe identifier (lowercase, underscores for spaces, alphanumeric + underscore only)
    
    Time complexity: O(n) where n is length of name
    """
    if not name:
        return "unnamed_field"
    
    # Convert to lowercase
    s = name.lower()
    
    # Replace spaces and special chars with underscores
    s = re.sub(r"[^\w]+", "_", s)
    
    # Remove leading/trailing underscores
    s = s.strip("_")
    
    # Ensure it starts with a letter or underscore
    if s and not s[0].isalpha() and s[0] != "_":
        s = "_" + s
    
    if not s:
        return "unnamed_field"
    
    return s


# Backward compatibility: keep old normalize function
def normalize(value: str) -> str:
    """
    Legacy normalize function (backward compatibility).
    Calls normalize_value with default options.
    """
    return normalize_value(value)
