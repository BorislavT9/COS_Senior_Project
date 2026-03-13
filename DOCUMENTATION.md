# Document Search & Extraction System — Full Documentation

## 1. Overview

This system is an **automated document ingestion and user-defined information extraction system** implemented in C++17. It provides:

- **Keyword search** — Search directories for documents containing a keyword; results are saved to history.
- **Rule-based extraction** — Extract structured data (e.g., invoice numbers, dates) from documents using regex rules.
- **AVL tree indexing** — O(log n) lookups and range queries on extracted fields.
- **Search history** — Persistent record of all keyword searches with file paths and match counts.

---

## 2. System Components

### 2.1 Directory Structure

```
Senior Project/
├── CMakeLists.txt              # C++17 build configuration
├── schema.sql                  # SQLite schema (rules, logs)
├── build.bat, run.bat          # Build and run scripts
│
├── include/document_ingestion/ # Public headers
│   ├── avl_tree.hpp            # AVL tree index
│   ├── config.hpp              # Paths, project root
│   ├── database.hpp            # SQLite connection
│   ├── ingestion_service.hpp   # Scan orchestration
│   ├── keyword_search_service.hpp
│   ├── models.hpp              # DocumentRecord, ExtractionResult
│   ├── parsers.hpp             # Text extraction from files
│   ├── registry.hpp            # Supported file types
│   ├── report_service.hpp     # HTML report generation
│   ├── rule_engine.hpp        # Regex extraction
│   ├── scanner.hpp            # Directory scanning, hashing
│   ├── search_history.hpp     # Search history persistence
│   └── store.hpp              # ExtractionStore
│
├── src/
│   ├── main.cpp               # CLI entry, interactive menu
│   ├── config.cpp
│   ├── db/                    # SQLite, rules repo, logs
│   ├── logging/               # Logger
│   ├── parsers/               # TXT, PDF, DOCX, XLSX parsers
│   ├── rules/                 # Rule engine, models
│   ├── scanner/               # Directory scan, file hash
│   ├── services/              # Ingestion, keyword search, report
│   ├── store/                 # AVL tree, ExtractionStore, SearchHistory
│   └── validation/             # Normalizer, validator
│
├── docs/                      # Architecture, build guides
├── sample_docs/               # Sample input files
├── data_store/                # Runtime output
│   ├── store.json             # Document records
│   ├── index_<field>.json     # AVL index per field
│   ├── search_history.json    # Keyword search history
│   └── report.html            # HTML report
├── data/                      # SQLite database (app.db)
└── third_party/               # miniz, picosha2
```

### 2.2 Component Summary

| Component | Role |
|-----------|------|
| **Scanner** | Recursively lists files in a directory; computes SHA-256 hash for change detection |
| **Registry** | Supported extensions: `.pdf`, `.docx`, `.txt`, `.xlsx` |
| **Parsers** | Extract plain text from each file type (TXT direct, DOCX/XLSX via miniz + pugixml, PDF optional) |
| **Rule Engine** | Applies regex patterns with optional anchors to extract structured data |
| **Validation** | Normalizes and validates extracted values (length, required, etc.) |
| **Store** | ExtractionStore: documents map + AVL tree indexes per field |
| **Search History** | Persists keyword searches and found files to `search_history.json` |
| **Services** | Ingestion (`run_scan`), keyword search, HTML report generation |

---

## 3. Interactive Menu

Running `.\run.bat` (or the executable with no arguments) starts the interactive menu:

### Option 1: Search for new files

1. Enter a directory path to search (e.g., `C:\Users\...\Documents`).
2. Enter a keyword (e.g., `Victor`).
3. The system scans all supported files in the directory and its subdirectories.
4. Each file containing the keyword is displayed with its path and match count.
5. **All found files are saved** to `data_store/search_history.json`.
6. Optionally, the system ingests the directory and applies extraction rules.

### Option 2: Show previously searched files

- Displays **all previously found files** with:
  - Full file path
  - The keyword that was searched
  - Match count for that file

Example output:

```
--- Previously searched files ---
  C:\Users\bobi9\Documents\ENG\Rough Draft.docx
    Keyword: "Victor" (69 matches)
  C:\Users\bobi9\Documents\HTY\Discussion2.docx
    Keyword: "Victor" (1 matches)
```

### Option 3: Exit

Exits the program.

---

## 4. Search History

### 4.1 Storage

- **File**: `data_store/search_history.json`
- **Format**: JSON array of search records

### 4.2 Data Model

Each search record (`SearchRecord`) contains:

| Field | Description |
|-------|-------------|
| `search_id` | Unique UUID |
| `file_type` | e.g., "ALL" |
| `directory` | Directory that was searched |
| `keyword` | Search term |
| `timestamp` | ISO timestamp |
| `files_found` | Count of files that matched |
| `files_stored` | Count ingested into ExtractionStore |
| `found_files` | List of `{file_path, matches}` for each found file |

### 4.3 API

- `add_search(file_type, directory, keyword, found_files, files_stored)` — Appends a new search and saves to disk.
- `get_all_searches()` — Returns all searches, sorted by timestamp (newest first).
- `get_statistics()` — Returns totals for searches and files found/stored.

---

## 5. Ingestion and Extraction

### 5.1 Flow

1. **Scan** — `scan_directory()` recursively lists all files.
2. **Filter** — Keep only supported extensions (`.pdf`, `.docx`, `.txt`, `.xlsx`).
3. **Change detection** — Compute SHA-256 hash; skip if path+hash already in store.
4. **Parse** — Extract plain text from each file.
5. **Load rules** — Active rules from SQLite (`data/app.db`).
6. **Extract** — Apply regex with optional `anchor_before`/`anchor_after`.
7. **Validate** — Normalize and validate each extraction.
8. **Store** — Add document to ExtractionStore; index each field in AVL tree.
9. **Save** — Persist to `store.json` and `index_<field>.json`.
10. **Report** — Generate `report.html`.

### 5.2 CLI Commands

```powershell
# Ingest documents (default: sample_docs)
.\run.bat ingest
.\run.bat ingest --watch_dir "C:\path\to\documents"

# Query extracted data
.\run.bat query --field "invoice_number" --equals "INV-2024-001"
.\run.bat query --field "invoice_number" --range "INV-1000" "INV-2000"
.\run.bat list --field "invoice_number"

# Export to JSON or CSV
.\run.bat export --format json --out "export.json"
.\run.bat export --format csv --out "export.csv"

# Demo: create sample rules and scan
.\run.bat --demo
```

---

## 6. Data Structures

### 6.1 ExtractionStore

- **Documents**: `unordered_map<doc_id, DocumentRecord>` — O(1) lookup.
- **Indexes**: `map<field_name, AVLTree>` — O(log n) for exact and range queries.

### 6.2 AVL Tree

- Self-balancing binary search tree.
- Key: normalized extracted value.
- Value: set of document IDs.
- Operations: insert (O(log n)), search (O(log n)), range query (O(log n + k)).

### 6.3 Persistence

| Data | File | Format |
|------|------|--------|
| Documents | `data_store/store.json` | JSON |
| Field indexes | `data_store/index_<field>.json` | JSON (inorder) |
| Search history | `data_store/search_history.json` | JSON |
| Rules | `data/app.db` | SQLite |
| Logs | `data/app.db` | SQLite |

---

## 7. Keyword Search

### 7.1 Process

1. `search_keyword_in_directory(directory, keyword)` scans the directory.
2. For each supported file: parse to text, count case-insensitive substring matches.
3. Returns `KeywordSearchResult` for each file: `{file_path, matches, status}`.
4. Files with `matches > 0` are passed to `history.add_search()`.
5. Search history is saved immediately.

### 7.2 Supported File Types

- TXT, DOCX, XLSX (full support)
- PDF (optional, requires Poppler)

---

## 8. Configuration

- **Project root** — Set from executable path; contains `CMakeLists.txt` or `schema.sql`.
- **Paths** (relative to project root):
  - `data_store/` — Store, indexes, search history, report
  - `data/` — SQLite database
  - `sample_docs/` — Default watch directory
  - `logs/` — Application logs

---

## 9. Build and Run

### Build

```powershell
cd "c:\Users\bobi9\Documents\Senior Project"
.\build.bat
# or: cmake -B build; cmake --build build --config Release
```

### Run

```powershell
.\run.bat                    # Interactive menu
.\run.bat --demo             # Create rules, scan sample_docs
.\run.bat ingest             # Ingest default directory
.\run.bat query --field ...  # Query extracted data
```

---

## 10. Dependencies

- **C++17** compiler (MSVC 2019+, GCC 9+, Clang 10+)
- **CMake** 3.16+
- **SQLite** — FetchContent or vcpkg
- **nlohmann/json** — JSON
- **pugixml** — XML (DOCX, XLSX)
- **miniz** — ZIP (DOCX, XLSX)
- **PicoSHA2** — SHA-256 hashing
