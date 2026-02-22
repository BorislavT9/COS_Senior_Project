"""
ExtractionStore: In-memory data structures for document storage and field indexing.
Uses AVL trees for fast lookups and JSON files for persistence.
"""

import json
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional

from src.store.avl_tree import AVLTree
from src.store.models import DocumentRecord


class ExtractionStore:
    """
    Primary storage for extracted document information.
    
    Components:
    - documents: dict mapping doc_id -> DocumentRecord
    - indexes: dict mapping field_name -> AVLTree (keyed by normalized value)
    
    Persistence:
    - store.json: serialized documents
    - index_<field>.json: inorder traversal of AVL tree for each field
    """
    
    def __init__(self):
        self.documents: Dict[str, DocumentRecord] = {}
        self.indexes: Dict[str, AVLTree] = {}
    
    def add_document(self, record: DocumentRecord) -> None:
        """
        Add a document record to the store.
        
        Args:
            record: DocumentRecord to add
        
        Time complexity: O(1)
        """
        self.documents[record.doc_id] = record
    
    def index_extraction(self, field: str, value: str, doc_id: str) -> None:
        """
        Index an extracted field value for fast retrieval.
        
        Args:
            field: Field name (rule name)
            value: Normalized extracted value
            doc_id: Document ID
        
        Time complexity: O(log n) where n is number of unique values for this field
        """
        if field not in self.indexes:
            self.indexes[field] = AVLTree()
        
        self.indexes[field].insert(value, doc_id)
    
    def query_equals(self, field: str, value: str) -> List[DocumentRecord]:
        """
        Find all documents where field equals value.
        
        Args:
            field: Field name to query
            value: Normalized value to match
        
        Returns:
            List of DocumentRecord objects
        
        Time complexity: O(log n + k) where n is unique values, k is number of results
        """
        if field not in self.indexes:
            return []
        
        doc_ids = self.indexes[field].search(value)
        return [self.documents[doc_id] for doc_id in doc_ids if doc_id in self.documents]
    
    def query_range(self, field: str, low: str, high: str) -> List[DocumentRecord]:
        """
        Find all documents where field value is in range [low, high] (lexicographic).
        
        Args:
            field: Field name to query
            low: Lower bound (inclusive)
            high: Upper bound (inclusive)
        
        Returns:
            List of DocumentRecord objects
        
        Time complexity: O(log n + k) where n is unique values, k is number of results
        """
        if field not in self.indexes:
            return []
        
        doc_ids = self.indexes[field].range_query(low, high)
        return [self.documents[doc_id] for doc_id in doc_ids if doc_id in self.documents]
    
    def list_field_values(self, field: str) -> List[str]:
        """
        List all unique normalized values for a field, sorted.
        
        Args:
            field: Field name
        
        Returns:
            Sorted list of unique values
        
        Time complexity: O(n) where n is number of unique values
        """
        if field not in self.indexes:
            return []
        
        items = self.indexes[field].inorder_items()
        return [key for key, _ in items]
    
    def save(self, base_dir: Path) -> None:
        """
        Persist store to JSON files.
        
        Creates:
        - base_dir/store.json: all documents
        - base_dir/index_<field>.json: inorder traversal for each field index
        
        Args:
            base_dir: Directory to save files to
        """
        base_dir.mkdir(parents=True, exist_ok=True)
        
        # Save documents
        store_path = base_dir / "store.json"
        documents_dict = {}
        for doc_id, record in self.documents.items():
            documents_dict[doc_id] = {
                "doc_id": record.doc_id,
                "file_path": record.file_path,
                "file_type": record.file_type,
                "file_hash": record.file_hash,
                "processed_at": record.processed_at,
                "status": record.status,
                "error_message": record.error_message,
                "extracted": record.extracted,
                "extracted_raw": record.extracted_raw,
            }
        
        with open(store_path, "w", encoding="utf-8") as f:
            json.dump(documents_dict, f, indent=2, ensure_ascii=False)
        
        # Save indexes (inorder traversal)
        for field, tree in self.indexes.items():
            index_path = base_dir / f"index_{field}.json"
            items = tree.inorder_items()
            index_data = [{"key": key, "doc_ids": doc_ids} for key, doc_ids in items]
            
            with open(index_path, "w", encoding="utf-8") as f:
                json.dump(index_data, f, indent=2, ensure_ascii=False)
    
    def load(self, base_dir: Path) -> None:
        """
        Load store from JSON files and rebuild AVL trees.
        
        Args:
            base_dir: Directory to load files from
        """
        # Load documents
        store_path = base_dir / "store.json"
        if store_path.exists():
            with open(store_path, "r", encoding="utf-8") as f:
                documents_dict = json.load(f)
            
            for doc_id, data in documents_dict.items():
                self.documents[doc_id] = DocumentRecord(
                    doc_id=data["doc_id"],
                    file_path=data["file_path"],
                    file_type=data["file_type"],
                    file_hash=data["file_hash"],
                    processed_at=data["processed_at"],
                    status=data["status"],
                    error_message=data.get("error_message"),
                    extracted=data.get("extracted", {}),
                    extracted_raw=data.get("extracted_raw", {}),
                )
        
        # Load indexes and rebuild AVL trees
        for index_file in base_dir.glob("index_*.json"):
            field = index_file.stem.replace("index_", "")
            
            with open(index_file, "r", encoding="utf-8") as f:
                index_data = json.load(f)
            
            tree = AVLTree()
            for item in index_data:
                key = item["key"]
                doc_ids = item["doc_ids"]
                for doc_id in doc_ids:
                    tree.insert(key, doc_id)
            
            self.indexes[field] = tree
    
    def find_by_path_and_hash(self, file_path: str, file_hash: str) -> Optional[DocumentRecord]:
        """
        Find a document by file path and hash (for duplicate detection).
        
        Args:
            file_path: File path
            file_hash: File hash
        
        Returns:
            DocumentRecord if found, None otherwise
        
        Time complexity: O(n) where n is number of documents (linear scan)
        """
        for record in self.documents.values():
            if record.file_path == file_path and record.file_hash == file_hash:
                return record
        return None
