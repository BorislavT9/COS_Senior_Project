# Complete Project Documentation

## Automated Document Ingestion and User-Defined Information Extraction System

**Version:** 2.0  
**Author:** BorislavT9  
**Repository:** https://github.com/BorislavT9/COS_Senior_Project  
**Last Updated:** 2024

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Core Features](#core-features)
4. [Installation & Setup](#installation--setup)
5. [Usage Guide](#usage-guide)
6. [Data Structures & Algorithms](#data-structures--algorithms)
7. [File Structure](#file-structure)
8. [API Reference](#api-reference)
9. [Configuration](#configuration)
10. [Testing](#testing)
11. [Performance Characteristics](#performance-characteristics)
12. [Troubleshooting](#troubleshooting)
13. [Future Enhancements](#future-enhancements)

---

## Project Overview

### Purpose

This senior project implements an **automated document ingestion and user-defined information extraction system** with a focus on **algorithm-focused data structures**. The system scans documents (PDF, DOCX, TXT, XLSX), extracts text, applies user-defined extraction rules (regex patterns with optional anchors), and stores results using efficient in-memory data structures with JSON-based persistence.

### Key Design Principles

1. **Algorithm-Focused**: Uses AVL trees for O(log n) indexing operations
2. **Non-Relational Storage**: JSON-based persistence without SQLite dependency for primary workflow
3. **Deterministic Normalization**: Consistent value processing for reliable indexing
4. **Interactive Interface**: User-friendly menu system with search history
5. **Extensible**: Modular design allowing easy addition of new parsers and rules

### Technology Stack

- **Language:** Python 3.11+
- **Data Structures:** AVL Trees, Python dicts
- **Persistence:** JSON files
- **Parsers:** pypdf (PDF), python-docx (DOCX), openpyxl (XLSX)
- **Web Framework:** FastAPI (optional UI)
- **Testing:** pytest
- **CLI:** argparse with interactive menu

---

## System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Interface Layer                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Interactive  │  │   CLI Commands│  │  Web UI (opt)│     │
│  │    Menu      │  │               │  │             │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                    Service Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  Ingestion   │  │   Keyword    │  │    Report    │     │
│  │   Service    │  │    Search    │  │  Generation  │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                    Core Storage Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Extraction  │  │  Search      │  │   AVL Tree   │     │
│  │    Store    │  │  History      │  │  Indexes     │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                    Processing Layer                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Scanner    │  │   Parsers    │  │   Rules     │     │
│  │              │  │   (Factory)  │  │   Engine    │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

### Component Layers

#### 1. User Interface Layer
- **Interactive Menu**: Menu-driven interface for file search
- **CLI Commands**: Command-line interface for advanced operations
- **Web UI**: Optional FastAPI-based web interface

#### 2. Service Layer
- **Ingestion Service**: Orchestrates document processing pipeline
- **Keyword Search Service**: Searches for keywords in document content
- **Report Service**: Generates HTML reports

#### 3. Core Storage Layer
- **ExtractionStore**: Primary storage with AVL tree indexes
- **SearchHistory**: Tracks user searches
- **AVL Tree Indexes**: Self-balancing BST for field value indexing

#### 4. Processing Layer
- **Scanner**: Discovers and hashes files
- **Parsers**: Extract text from various file formats
- **Rules Engine**: Applies regex patterns with anchors

---

## Core Features

### 1. Document Ingestion

- **Supported Formats**: PDF, DOCX, TXT, XLSX
- **Automatic Detection**: File type detection by extension
- **Hash-Based Deduplication**: Skips unchanged files
- **Error Handling**: Graceful handling of parse failures

### 2. Rule-Based Extraction

- **Regex Patterns**: Flexible pattern matching
- **Anchors**: Optional before/after anchors for context
- **Validation**: Max length, required field checks
- **Normalization**: Deterministic value normalization

### 3. AVL Tree Indexing

- **Self-Balancing**: Guarantees O(log n) operations
- **Field Indexing**: Separate index per extraction field
- **Range Queries**: Efficient lexicographic range searches
- **Sorted Traversal**: Inorder traversal for sorted output

### 4. Interactive Menu System

- **File Type Selection**: Choose PDF, DOCX, TXT, XLSX, or All
- **Directory Input**: Search any directory
- **Keyword Search**: Find any keyword/symbol in files
- **Search History**: View all previous searches
- **Statistics Panel**: Real-time statistics display

### 5. Search History

- **Persistent Storage**: JSON-based history file
- **Search Tracking**: Records all search parameters
- **Statistics Aggregation**: Cumulative statistics across searches
- **Timestamp Tracking**: Full audit trail

### 6. Query Operations

- **Exact Match**: O(log n) exact value lookup
- **Range Query**: O(log n + k) range searches
- **List Values**: O(n) sorted value listing
- **Keyword Search**: Full-text keyword search

---

## Installation & Setup

### Prerequisites

- Python 3.11 or higher
- Windows 10/11 (or compatible OS)
- PowerShell or Command Prompt
- Internet connection (for initial package installation)

### Step-by-Step Installation

#### 1. Clone Repository

```powershell
git clone https://github.com/BorislavT9/COS_Senior_Project.git
cd COS_Senior_Project
```

#### 2. Create Virtual Environment

```powershell
python -m venv .venv
```

#### 3. Activate Virtual Environment

**Windows PowerShell:**
```powershell
.venv\Scripts\activate
```

**Windows Command Prompt:**
```cmd
.venv\Scripts\activate.bat
```

#### 4. Install Dependencies

```powershell
pip install -r requirements.txt
```

#### 5. Initialize Database (Optional)

```powershell
python -m src.main --demo
```

This creates sample rules and initializes the database.

### Verification

Test the installation:

```powershell
python -m src.main --help
```

You should see the help menu.

---

## Usage Guide

### Interactive Menu (Recommended)

#### Starting the Program

```powershell
python -m src.main
```

#### Menu Options

**Option 1: Search for New Files**

1. Select file type (1-5)
2. Enter directory path
3. Enter keyword to search
4. View results and statistics

**Option 2: Show Previously Searched Files**

- Displays all past searches
- Shows statistics panel
- Includes timestamps and results

**Option 3: Exit**

- Quits the program

### Command-Line Interface

#### Ingest Documents

```powershell
# Default directory (sample_docs)
python -m src.main ingest

# Custom directory
python -m src.main ingest --watch_dir "C:\path\to\documents"
```

#### Query Extracted Data

```powershell
# Exact match
python -m src.main query --field "invoice_number" --equals "INV-1234-567"

# Range query
python -m src.main query --field "invoice_number" --range "INV-1000" "INV-2000"
```

#### List Field Values

```powershell
python -m src.main list --field "invoice_number"
```

#### Export Data

```powershell
# JSON export
python -m src.main export --format json --out "export.json"

# CSV export
python -m src.main export --format csv --out "export.csv"
```

### Web UI (Optional)

```powershell
python -m src.main --serve
```

Then visit: http://127.0.0.1:8000

---

## Data Structures & Algorithms

### AVL Tree Implementation

#### Overview

The AVL tree is a self-balancing binary search tree that maintains balance through rotations, ensuring O(log n) operations regardless of insertion order.

#### Key Operations

**Insert Operation: O(log n)**
```python
tree.insert(key, doc_id)
```
- Performs normal BST insert
- Updates node heights
- Balances tree using rotations if needed

**Search Operation: O(log n)**
```python
doc_ids = tree.search(key)
```
- Recursive search through tree
- Returns set of document IDs

**Range Query: O(log n + k)**
```python
doc_ids = tree.range_query(low, high)
```
- Traverses tree to find all keys in range
- k = number of results

**Inorder Traversal: O(n)**
```python
items = tree.inorder_items()
```
- Returns sorted list of (key, doc_ids) tuples

#### Rotations

**Left Rotation**: Fixes right-heavy subtree
**Right Rotation**: Fixes left-heavy subtree
**Left-Right Rotation**: Double rotation for left-right case
**Right-Left Rotation**: Double rotation for right-left case

#### Balance Factor

Balance factor = height(left) - height(right)
- Must be in [-1, 0, 1] for balanced tree
- Violations trigger rotations

### ExtractionStore

#### Structure

```python
class ExtractionStore:
    documents: Dict[str, DocumentRecord]  # O(1) lookup
    indexes: Dict[str, AVLTree]            # O(log n) search
```

#### Operations

| Operation | Time Complexity | Description |
|-----------|----------------|-------------|
| `add_document()` | O(1) | Add document to store |
| `index_extraction()` | O(log n) | Index field value |
| `query_equals()` | O(log n + k) | Exact match query |
| `query_range()` | O(log n + k) | Range query |
| `list_field_values()` | O(n) | List all values |
| `save()` | O(n) | Persist to JSON |
| `load()` | O(n) | Load from JSON |

### Normalization Algorithm

#### Steps

1. **Strip**: Remove leading/trailing whitespace
2. **Collapse**: Replace multiple whitespace with single space
3. **Optional Uppercase**: Convert to uppercase (configurable)
4. **Optional Punctuation Removal**: Remove surrounding punctuation

#### Deterministic Behavior

- Same input always produces same output
- Critical for consistent indexing
- Enables reliable lookups

---

## File Structure

```
COS_Senior_Project/
│
├── src/                          # Source code
│   ├── __init__.py
│   ├── main.py                   # Entry point with interactive menu
│   ├── config.py                 # Configuration management
│   │
│   ├── store/                    # Storage layer
│   │   ├── __init__.py
│   │   ├── avl_tree.py          # AVL tree implementation
│   │   ├── models.py            # Data models (DocumentRecord, etc.)
│   │   ├── store.py             # ExtractionStore
│   │   └── search_history.py    # Search history management
│   │
│   ├── services/                 # Business logic
│   │   ├── __init__.py
│   │   ├── ingestion_service.py # Document ingestion pipeline
│   │   ├── keyword_search_service.py # Keyword search
│   │   ├── report_service.py    # HTML report generation
│   │   └── export_service.py    # Data export
│   │
│   ├── parsers/                  # File parsers
│   │   ├── __init__.py
│   │   ├── base.py              # Base parser interface
│   │   ├── factory.py           # Parser factory
│   │   ├── pdf_parser.py        # PDF parser
│   │   ├── docx_parser.py       # DOCX parser
│   │   ├── txt_parser.py        # TXT parser
│   │   └── xlsx_parser.py        # XLSX parser
│   │
│   ├── rules/                    # Extraction rules
│   │   ├── __init__.py
│   │   ├── models.py            # Rule models
│   │   ├── engine.py            # Rule application engine
│   │   └── repository.py        # Rule repository
│   │
│   ├── validation/               # Validation & normalization
│   │   ├── __init__.py
│   │   ├── normalizer.py        # Value normalization
│   │   └── validator.py         # Rule validation
│   │
│   ├── scanner/                  # File scanning
│   │   ├── __init__.py
│   │   ├── scanner.py            # Directory scanning
│   │   └── registry.py          # File type registry
│   │
│   ├── db/                       # Database layer (legacy)
│   │   ├── __init__.py
│   │   ├── database.py          # DB connection
│   │   ├── documents_repo.py     # Documents repository
│   │   ├── rules_repo.py        # Rules repository
│   │   ├── extractions_repo.py  # Extractions repository
│   │   └── logs_repo.py         # Logs repository
│   │
│   ├── ui/                       # Web UI (optional)
│   │   ├── __init__.py
│   │   ├── api.py               # FastAPI app
│   │   ├── routes_*.py           # Route handlers
│   │   └── templates/           # HTML templates
│   │
│   └── logging/                  # Logging
│       ├── __init__.py
│       └── logger.py            # Logger setup
│
├── tests/                        # Test suite
│   ├── test_avl.py              # AVL tree tests
│   ├── test_store.py            # Store tests
│   ├── test_ingestion_with_store.py # Integration tests
│   ├── test_db.py               # Database tests
│   ├── test_pipeline.py         # Pipeline tests
│   ├── test_rules.py            # Rules tests
│   └── test_validation.py       # Validation tests
│
├── docs/                         # Documentation
│   ├── architecture.md          # Architecture documentation
│   └── diagrams/                # Architecture diagrams
│
├── data_store/                   # Runtime data (created at runtime)
│   ├── store.json               # Document store
│   ├── index_*.json             # AVL tree indexes
│   └── search_history.json      # Search history
│
├── data/                         # Database files (created at runtime)
│   └── app.db                   # SQLite database
│
├── logs/                         # Log files (created at runtime)
│   └── app.log                  # Application logs
│
├── sample_docs/                  # Sample documents
│   ├── sample.txt
│   ├── sample.pdf
│   ├── sample.docx
│   └── sample.xlsx
│
├── scripts/                      # Utility scripts
│   └── create_sample_files.py   # Sample file generator
│
├── .gitignore                    # Git ignore rules
├── pytest.ini                   # Pytest configuration
├── requirements.txt              # Python dependencies
├── schema.sql                    # Database schema
│
├── README.md                     # Project readme
├── QUICKSTART.md                 # Quick start guide
├── INTERACTIVE_FEATURES.md       # Interactive features guide
├── INSTALL_PYTHON.md            # Python installation guide
└── DOCUMENTATION.md              # This file
```

---

## API Reference

### Main Entry Point

#### `src.main.main()`

Main entry point that handles command-line arguments and interactive menu.

**Usage:**
```python
python -m src.main [command] [options]
```

### ExtractionStore API

#### `ExtractionStore()`

Creates a new extraction store instance.

#### `add_document(record: DocumentRecord) -> None`

Adds a document record to the store.

**Time Complexity:** O(1)

#### `index_extraction(field: str, value: str, doc_id: str) -> None`

Indexes an extracted field value.

**Time Complexity:** O(log n)

#### `query_equals(field: str, value: str) -> List[DocumentRecord]`

Finds all documents where field equals value.

**Time Complexity:** O(log n + k)

#### `query_range(field: str, low: str, high: str) -> List[DocumentRecord]`

Finds all documents where field value is in range [low, high].

**Time Complexity:** O(log n + k)

#### `list_field_values(field: str) -> List[str]`

Lists all unique normalized values for a field, sorted.

**Time Complexity:** O(n)

#### `save(base_dir: Path) -> None`

Persists store to JSON files.

#### `load(base_dir: Path) -> None`

Loads store from JSON files and rebuilds AVL trees.

### AVLTree API

#### `AVLTree()`

Creates a new AVL tree instance.

#### `insert(key: str, doc_id: str) -> None`

Inserts a key-value pair into the tree.

**Time Complexity:** O(log n)

#### `search(key: str) -> Set[str]`

Searches for a key and returns document IDs.

**Time Complexity:** O(log n)

#### `range_query(low: str, high: str) -> Set[str]`

Finds all document IDs for keys in range [low, high].

**Time Complexity:** O(log n + k)

#### `inorder_items() -> List[Tuple[str, List[str]]]`

Gets all items in sorted order (inorder traversal).

**Time Complexity:** O(n)

### SearchHistory API

#### `SearchHistory(history_file: Path)`

Creates a search history manager.

#### `add_search(file_type: str, directory: str, keyword: str, files_found: int, files_stored: int) -> SearchRecord`

Adds a new search record.

#### `get_all_searches() -> List[SearchRecord]`

Gets all search records, most recent first.

#### `get_statistics() -> Dict[str, int]`

Gets overall statistics.

---

## Configuration

### Configuration File: `src/config.py`

#### Paths

- `WATCH_DIR`: Default watch directory ("./sample_docs")
- `DB_PATH`: Database file path ("./data/app.db")
- `LOG_PATH`: Log file path ("./logs/app.log")
- `DATA_STORE_DIR`: Data store directory ("./data_store")

#### Functions

- `ensure_dirs()`: Creates necessary directories
- `get_watch_dir()`: Returns watch directory path
- `get_db_path()`: Returns database path
- `get_log_path()`: Returns log file path
- `get_data_store_dir()`: Returns data store directory
- `get_search_history_path()`: Returns search history file path

### Environment Variables

Currently, all configuration is file-based. Future versions may support environment variables.

---

## Testing

### Running Tests

```powershell
# Run all tests
pytest tests\ -v

# Run specific test file
pytest tests\test_avl.py -v

# Run with coverage
pytest tests\ --cov=src --cov-report=html
```

### Test Coverage

#### Unit Tests

- **test_avl.py**: AVL tree operations and balancing
- **test_store.py**: Store operations and persistence
- **test_validation.py**: Normalization and validation
- **test_rules.py**: Rule engine functionality

#### Integration Tests

- **test_ingestion_with_store.py**: End-to-end ingestion
- **test_pipeline.py**: Full pipeline testing
- **test_db.py**: Database operations

### Test Data

Tests use temporary directories and mock data to ensure isolation and reproducibility.

---

## Performance Characteristics

### Time Complexity Summary

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Insert document | O(1) | Dict insertion |
| Index extraction | O(log n) | AVL insert per field |
| Exact query | O(log n) | AVL search |
| Range query | O(log n + k) | k = number of results |
| List values | O(n) | Inorder traversal |
| Save/Load | O(n) | Serialize all data |
| Keyword search | O(n × m) | n = files, m = avg file size |

### Space Complexity

- **DocumentStore**: O(d) where d = number of documents
- **FieldIndex**: O(v) where v = unique values per field
- **Total**: O(d + Σv) across all fields

### Scalability

- **Small datasets** (< 1,000 docs): Excellent performance
- **Medium datasets** (1,000 - 100,000 docs): Good performance
- **Large datasets** (> 100,000 docs): May require optimization

### Optimization Opportunities

1. **Batch Operations**: Group multiple inserts
2. **Lazy Loading**: Load indexes on demand
3. **Compression**: Compress JSON files
4. **Caching**: Cache frequently accessed data

---

## Troubleshooting

### Common Issues

#### Python Not Found

**Problem:** `Python was not found`

**Solution:**
1. Install Python from https://www.python.org/downloads/
2. Ensure "Add Python to PATH" is checked during installation
3. Restart terminal after installation

#### Import Errors

**Problem:** `ModuleNotFoundError` or `ImportError`

**Solution:**
1. Activate virtual environment: `.venv\Scripts\activate`
2. Install dependencies: `pip install -r requirements.txt`
3. Verify installation: `pip list`

#### No Rules Found

**Problem:** Ingestion completes but no extractions

**Solution:**
1. Create rules via web UI: `python -m src.main --serve`
2. Or use demo mode: `python -m src.main --demo`
3. Verify rules are active in database

#### Store Not Loading

**Problem:** `data_store/store.json` not found

**Solution:**
- This is normal on first run
- Store will be created after first ingestion
- Ensure `data_store/` directory exists

#### Database Errors

**Problem:** SQLite database errors

**Solution:**
1. Delete `data/app.db` to recreate
2. Run: `python -m src.main` (initializes DB)
3. Check file permissions

### Debug Mode

Enable verbose logging:

```python
# In src/logging/logger.py
LOG.setLevel(logging.DEBUG)
```

### Getting Help

1. Check logs: `logs/app.log`
2. Review error messages in console
3. Verify file paths and permissions
4. Check GitHub issues: https://github.com/BorislavT9/COS_Senior_Project/issues

---

## Future Enhancements

### Planned Features

1. **Multi-threading**: Parallel document processing
2. **Incremental Updates**: Update indexes without full rebuild
3. **Compression**: Compress JSON storage files
4. **Advanced Queries**: Full-text search, fuzzy matching
5. **Export Formats**: Additional export formats (XML, Excel)
6. **Web UI Enhancements**: Real-time updates, better visualization
7. **Rule Templates**: Pre-built rule templates
8. **Batch Operations**: Process multiple directories
9. **Scheduled Scans**: Automatic periodic scanning
10. **API Endpoints**: RESTful API for external integration

### Performance Improvements

1. **Index Optimization**: B-tree variants for better cache performance
2. **Memory Mapping**: Memory-mapped files for large datasets
3. **Query Optimization**: Query planning and optimization
4. **Caching Layer**: Redis/Memcached integration

### Code Quality

1. **Type Hints**: Complete type annotations
2. **Documentation**: Comprehensive docstrings
3. **Code Coverage**: Increase test coverage to 90%+
4. **Linting**: Strict linting rules (pylint, mypy)

---

## Contributing

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make changes
4. Add tests
5. Run test suite
6. Submit pull request

### Code Style

- Follow PEP 8
- Use type hints
- Write docstrings
- Keep functions focused

### Testing Requirements

- All new features must include tests
- Maintain >80% code coverage
- Tests must pass before PR

---

## License

This project is part of a senior project/coursework. Please refer to the repository for license information.

---

## Contact & Support

- **GitHub:** https://github.com/BorislavT9/COS_Senior_Project
- **Issues:** https://github.com/BorislavT9/COS_Senior_Project/issues
- **Author:** BorislavT9

---

## Changelog

### Version 2.0 (Current)

- ✅ AVL tree indexing implementation
- ✅ Interactive menu system
- ✅ Search history persistence
- ✅ Keyword search functionality
- ✅ JSON-based non-relational storage
- ✅ Comprehensive test suite
- ✅ Enhanced documentation

### Version 1.0 (Initial)

- Basic document ingestion
- SQLite-based storage
- Rule-based extraction
- Web UI
- Basic CLI

---

## Acknowledgments

- Python community for excellent libraries
- FastAPI for web framework
- All contributors and testers

---

**End of Documentation**
