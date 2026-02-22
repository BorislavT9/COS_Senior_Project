# Architecture

## Overview

This system uses **algorithm-focused data structures** for storing and retrieving extracted information:

- **Scanner:** Recursively lists files in a watch directory; computes file hashes (SHA-256). Registry filters supported extensions (PDF, DOCX, TXT, XLSX).
- **Parsers:** PDF (pypdf), DOCX (python-docx), TXT (plain), XLSX (openpyxl) → plain text. Factory selects by extension.
- **Rules:** Regex patterns with optional `anchor_before` / `anchor_after` to restrict search window. Engine returns extraction candidates (first match per rule).
- **Validation:** Validates candidates (empty, max_length, required). Status: SUCCESS, NO_MATCH, INVALID.
- **Normalization:** Deterministic normalization (trim, collapse whitespace, optional casing) for consistent indexing.
- **Storage:** **ExtractionStore** with AVL tree indexes (primary), SQLite for rules (legacy).

## Core Data Structures

### ExtractionStore

**Components:**
- `documents: dict[str, DocumentRecord]` - O(1) document lookup by ID
- `indexes: dict[str, AVLTree]` - O(log n) field value lookups

**AVL Tree Index:**
- Self-balancing binary search tree
- Key: normalized extracted value (string)
- Value: set of document IDs
- Time complexity: O(log n) for insert/search, O(log n + k) for range queries

### Persistence Format

**JSON Files (non-relational):**
- `store.json`: Serialized DocumentRecord objects
- `index_<field>.json`: Inorder traversal of AVL tree (list of {key, doc_ids})

**Rebuilding AVL Trees:**
- On load, reconstruct AVL trees from inorder JSON data
- Insert operations maintain balance automatically

## Layers

| Layer | Responsibility |
|-------|----------------|
| **CLI (main.py)** | Command-line interface: ingest, query, list, export |
| **Services** | Ingestion (scan→parse→extract→validate→store), Report generation |
| **Store** | ExtractionStore, AVL tree implementation, JSON persistence |
| **Rules (engine, models)** | Regex extraction with anchors, rule models |
| **Validation** | Normalize, validate extracted values |
| **Parsers** | File-type-specific text extraction |
| **Scanner** | Directory scan, file hash, registry |
| **DB (repositories)** | SQLite for rules (legacy), optional logging |

## Data Flow (New MVP)

1. User runs `python -m src.main ingest --watch_dir <dir>`
2. Load ExtractionStore from `data_store/` (if exists)
3. Ingestion service scans directory, filters supported files
4. For each file:
   - Compute hash
   - Check if path+hash exists in store → skip if unchanged
   - Else: parse file to text
   - Load active rules from DB (by file_type or global)
   - Apply rules → extract, normalize, validate
   - Create DocumentRecord with extracted fields
   - Add to store.documents (O(1))
   - Index each successful extraction: `store.index_extraction(field, normalized_value, doc_id)` (O(log n))
5. Save store to JSON files
6. Generate HTML report and auto-open (Windows)

## Query Operations

### Exact Lookup
```python
store.query_equals(field, value)  # O(log n)
```
- Searches AVL tree for exact key match
- Returns list of DocumentRecord objects

### Range Query
```python
store.query_range(field, low, high)  # O(log n + k)
```
- Traverses AVL tree to find all keys in [low, high]
- Returns list of DocumentRecord objects

### List Values
```python
store.list_field_values(field)  # O(n)
```
- Inorder traversal of AVL tree
- Returns sorted list of unique normalized values

## Hash-Based Change Detection

- Same file_path + same file_hash in store → skip (unchanged)
- Different hash or new path → process as new document

## Why AVL Trees?

1. **Balanced**: Guarantees O(log n) operations regardless of insertion order
2. **Range Queries**: Efficient lexicographic range searches
3. **Sorted Traversal**: Inorder traversal provides sorted values
4. **Algorithm Focus**: Demonstrates understanding of data structures and algorithms
5. **Scalability**: Better than linear scan for large datasets

## Time Complexity Summary

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Insert document | O(1) | Dict insertion |
| Index extraction | O(log n) | AVL insert per field |
| Exact query | O(log n) | AVL search |
| Range query | O(log n + k) | k = number of results |
| List values | O(n) | Inorder traversal |
| Save/Load | O(n) | Serialize all data |
