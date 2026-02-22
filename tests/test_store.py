"""
Tests for ExtractionStore.
"""

import json
import tempfile
from pathlib import Path

import pytest

from src.store.models import DocumentRecord
from src.store.store import ExtractionStore


def test_store_add_document():
    """Test adding documents to store."""
    store = ExtractionStore()
    
    doc = DocumentRecord(
        doc_id="test-1",
        file_path="/test/file.txt",
        file_type=".txt",
        file_hash="abc123",
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
        extracted={"field1": "value1"},
    )
    
    store.add_document(doc)
    assert "test-1" in store.documents
    assert store.documents["test-1"] == doc


def test_store_index_extraction():
    """Test indexing extractions."""
    store = ExtractionStore()
    
    store.index_extraction("invoice_number", "INV-001", "doc1")
    store.index_extraction("invoice_number", "INV-002", "doc2")
    store.index_extraction("invoice_number", "INV-001", "doc3")  # Duplicate value
    
    assert "invoice_number" in store.indexes
    assert store.indexes["invoice_number"].search("INV-001") == {"doc1", "doc3"}
    assert store.indexes["invoice_number"].search("INV-002") == {"doc2"}


def test_store_query_equals():
    """Test exact value query."""
    store = ExtractionStore()
    
    doc1 = DocumentRecord(
        doc_id="doc1",
        file_path="/file1.txt",
        file_type=".txt",
        file_hash="hash1",
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
        extracted={"invoice": "INV-001"},
    )
    doc2 = DocumentRecord(
        doc_id="doc2",
        file_path="/file2.txt",
        file_type=".txt",
        file_hash="hash2",
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
        extracted={"invoice": "INV-002"},
    )
    
    store.add_document(doc1)
    store.add_document(doc2)
    store.index_extraction("invoice", "INV-001", "doc1")
    store.index_extraction("invoice", "INV-002", "doc2")
    
    results = store.query_equals("invoice", "INV-001")
    assert len(results) == 1
    assert results[0].doc_id == "doc1"


def test_store_query_range():
    """Test range query."""
    store = ExtractionStore()
    
    docs = []
    for i in range(5):
        doc = DocumentRecord(
            doc_id=f"doc{i}",
            file_path=f"/file{i}.txt",
            file_type=".txt",
            file_hash=f"hash{i}",
            processed_at="2024-01-01T00:00:00",
            status="SUCCESS",
            extracted={"number": f"NUM-{i:03d}"},
        )
        store.add_document(doc)
        store.index_extraction("number", f"NUM-{i:03d}", f"doc{i}")
    
    results = store.query_range("number", "NUM-001", "NUM-003")
    assert len(results) == 3
    doc_ids = {r.doc_id for r in results}
    assert doc_ids == {"doc1", "doc2", "doc3"}


def test_store_list_field_values():
    """Test listing unique field values."""
    store = ExtractionStore()
    
    store.index_extraction("field", "value1", "doc1")
    store.index_extraction("field", "value2", "doc2")
    store.index_extraction("field", "value1", "doc3")  # Duplicate
    
    values = store.list_field_values("field")
    assert values == ["value1", "value2"]


def test_store_save_and_load():
    """Test persistence: save and load."""
    store = ExtractionStore()
    
    # Add documents
    doc1 = DocumentRecord(
        doc_id="doc1",
        file_path="/file1.txt",
        file_type=".txt",
        file_hash="hash1",
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
        extracted={"field1": "value1"},
        extracted_raw={"field1": "value1_raw"},
    )
    store.add_document(doc1)
    store.index_extraction("field1", "value1", "doc1")
    
    # Save to temp directory
    with tempfile.TemporaryDirectory() as tmpdir:
        store_dir = Path(tmpdir)
        store.save(store_dir)
        
        # Verify files exist
        assert (store_dir / "store.json").exists()
        assert (store_dir / "index_field1.json").exists()
        
        # Load into new store
        store2 = ExtractionStore()
        store2.load(store_dir)
        
        # Verify data
        assert len(store2.documents) == 1
        assert "doc1" in store2.documents
        assert store2.documents["doc1"].file_path == "/file1.txt"
        assert store2.documents["doc1"].extracted == {"field1": "value1"}
        
        # Verify index
        assert "field1" in store2.indexes
        assert store2.indexes["field1"].search("value1") == {"doc1"}


def test_store_find_by_path_and_hash():
    """Test finding document by path and hash."""
    store = ExtractionStore()
    
    doc1 = DocumentRecord(
        doc_id="doc1",
        file_path="/file1.txt",
        file_type=".txt",
        file_hash="hash1",
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
    )
    doc2 = DocumentRecord(
        doc_id="doc2",
        file_path="/file1.txt",
        file_type=".txt",
        file_hash="hash2",  # Different hash
        processed_at="2024-01-01T00:00:00",
        status="SUCCESS",
    )
    
    store.add_document(doc1)
    store.add_document(doc2)
    
    found = store.find_by_path_and_hash("/file1.txt", "hash1")
    assert found is not None
    assert found.doc_id == "doc1"
    
    found = store.find_by_path_and_hash("/file1.txt", "hash2")
    assert found is not None
    assert found.doc_id == "doc2"
    
    found = store.find_by_path_and_hash("/nonexistent.txt", "hash1")
    assert found is None
