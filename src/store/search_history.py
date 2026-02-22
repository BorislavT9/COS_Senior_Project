"""
Search history storage for tracking user searches.
"""

import json
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Optional


class SearchRecord:
    """Record of a single search operation."""
    
    def __init__(
        self,
        search_id: str,
        file_type: str,
        directory: str,
        keyword: str,
        timestamp: str,
        files_found: int = 0,
        files_stored: int = 0
    ):
        self.search_id = search_id
        self.file_type = file_type
        self.directory = directory
        self.keyword = keyword
        self.timestamp = timestamp
        self.files_found = files_found
        self.files_stored = files_stored
    
    def to_dict(self) -> Dict:
        """Convert to dictionary for JSON serialization."""
        return {
            "search_id": self.search_id,
            "file_type": self.file_type,
            "directory": self.directory,
            "keyword": self.keyword,
            "timestamp": self.timestamp,
            "files_found": self.files_found,
            "files_stored": self.files_stored,
        }
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'SearchRecord':
        """Create from dictionary."""
        return cls(
            search_id=data["search_id"],
            file_type=data["file_type"],
            directory=data["directory"],
            keyword=data["keyword"],
            timestamp=data["timestamp"],
            files_found=data.get("files_found", 0),
            files_stored=data.get("files_stored", 0),
        )


class SearchHistory:
    """Manages search history persistence."""
    
    def __init__(self, history_file: Path):
        self.history_file = history_file
        self.searches: List[SearchRecord] = []
        self.load()
    
    def load(self) -> None:
        """Load search history from file."""
        if self.history_file.exists():
            try:
                with open(self.history_file, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    self.searches = [SearchRecord.from_dict(item) for item in data]
            except Exception:
                self.searches = []
        else:
            self.searches = []
    
    def save(self) -> None:
        """Save search history to file."""
        self.history_file.parent.mkdir(parents=True, exist_ok=True)
        data = [search.to_dict() for search in self.searches]
        with open(self.history_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
    
    def add_search(
        self,
        file_type: str,
        directory: str,
        keyword: str,
        files_found: int = 0,
        files_stored: int = 0
    ) -> SearchRecord:
        """Add a new search record."""
        import uuid
        search_id = str(uuid.uuid4())
        timestamp = datetime.now().isoformat()
        
        record = SearchRecord(
            search_id=search_id,
            file_type=file_type,
            directory=directory,
            keyword=keyword,
            timestamp=timestamp,
            files_found=files_found,
            files_stored=files_stored,
        )
        
        self.searches.append(record)
        self.save()
        return record
    
    def get_all_searches(self) -> List[SearchRecord]:
        """Get all search records, most recent first."""
        return sorted(self.searches, key=lambda x: x.timestamp, reverse=True)
    
    def get_statistics(self) -> Dict[str, int]:
        """Get overall statistics."""
        total_searches = len(self.searches)
        total_files_found = sum(s.files_found for s in self.searches)
        total_files_stored = sum(s.files_stored for s in self.searches)
        
        return {
            "total_searches": total_searches,
            "total_files_found": total_files_found,
            "total_files_stored": total_files_stored,
        }
