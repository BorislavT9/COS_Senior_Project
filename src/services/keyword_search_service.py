"""
Keyword search service: search for keywords in document content.
"""

from pathlib import Path
from typing import List, Dict

from src.parsers.factory import parse_file
from src.scanner.registry import filter_supported, get_file_type, is_supported
from src.scanner.scanner import scan_directory
from src.store.models import DocumentRecord


def search_keyword_in_directory(
    directory: str,
    keyword: str,
    file_type_filter: str = None
) -> List[Dict]:
    """
    Search for a keyword in all files in a directory.
    
    Args:
        directory: Directory to search
        keyword: Keyword to search for (case-insensitive)
        file_type_filter: Optional file type filter (e.g., ".txt", ".pdf")
    
    Returns:
        List of dicts with file_path, matches (count), and status
    """
    results = []
    paths = scan_directory(directory)
    supported = filter_supported(paths)
    
    # Filter by file type if specified
    if file_type_filter:
        supported = [p for p in supported if get_file_type(p) == file_type_filter]
    
    keyword_lower = keyword.lower()
    
    for file_path in supported:
        try:
            # Parse file to text
            text = parse_file(file_path)
            text_lower = text.lower()
            
            # Count occurrences
            matches = text_lower.count(keyword_lower)
            
            if matches > 0:
                results.append({
                    "file_path": file_path,
                    "matches": matches,
                    "status": "FOUND",
                    "file_type": get_file_type(file_path),
                })
            else:
                results.append({
                    "file_path": file_path,
                    "matches": 0,
                    "status": "NOT_FOUND",
                    "file_type": get_file_type(file_path),
                })
        except Exception as e:
            results.append({
                "file_path": file_path,
                "matches": 0,
                "status": "ERROR",
                "error": str(e),
                "file_type": get_file_type(file_path),
            })
    
    return results
