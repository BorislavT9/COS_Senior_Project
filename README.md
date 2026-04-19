# Automated Document Ingestion and User-Defined Information Extraction System

## Project Summary

This senior project implements an **automated document ingestion and user-defined information extraction system** with **algorithm-focused data structures**. Documents (PDF, DOCX, TXT, XLSX) are scanned, parsed to text, and rule-based extraction (regex + optional anchors) is applied to pull structured data.

**Key Features:**
- **AVL Tree Indexing**: Self-balancing binary search trees for O(log n) field value lookups
- **In-Memory Storage**: DocumentStore + FieldIndex (AVL trees) for fast retrieval
- **JSON Persistence**: Non-relational storage format
- **CLI Interface**: Full-featured command-line interface for ingestion and querying
- **Deterministic Normalization**: Consistent value normalization for reliable indexing

## Documentation

- **[DOCUMENTATION.md](DOCUMENTATION.md)** — Full system documentation (components, flows, search history, CLI)
- **[docs/architecture.md](docs/architecture.md)** — Architecture, data structures, data flow
- **[docs/CPP_BUILD.md](docs/CPP_BUILD.md)** — Build guide
- **[QUICKSTART.md](QUICKSTART.md)** — Quick start

## Repo Structure

```
/docs
  architecture.md
  CPP_BUILD.md
/include/document_ingestion/   # C++ headers
/src                          # C++ sources
  store/, scanner/, parsers/, rules/, validation/, db/, services/
/third_party
  miniz/, picosha2.h
/sample_docs                  # Sample documents to ingest
/data_store                   # JSON store output
schema.sql
CMakeLists.txt
README.md
```

## Prerequisites

- **C++17** compiler (MSVC 2019+, GCC 9+, Clang 10+)
- **CMake** 3.16+
- **Git** (for FetchContent dependencies)

## How to Run

### 1. Build

```powershell
cd "c:\Users\bobi9\Documents\Senior Project"
cmake -B build
cmake --build build --config Release
```

*(First build may download SQLite amalgamation, nlohmann/json, and pugixml.)*

### 2. Run

```powershell
cd "c:\Users\bobi9\Documents\Senior Project"
.\run.bat
```

No arguments: **Interactive menu**:
- 1. Search for new files — Enter directory and keyword; found files are saved to history
- 2. Show previously searched files — View all saved files with paths and keywords
- 3. Exit

### 3. Demo (create rules + scan)

```powershell
.\build\Release\document_ingestion.exe --demo
```

### 4. Ingest documents

```powershell
# Default: sample_docs
.\build\Release\document_ingestion.exe ingest

# Custom directory
.\build\Release\document_ingestion.exe ingest --watch_dir "C:\path\to\documents"
```

### 5. Query extracted data

```powershell
# Exact match
.\build\Release\document_ingestion.exe query --field "invoice_number" --equals "INV-2024-001"

# Range query
.\build\Release\document_ingestion.exe query --field "invoice_number" --range "INV-1000" "INV-2000"

# List all values
.\build\Release\document_ingestion.exe list --field "invoice_number"
```

### 6. Export

```powershell
.\build\Release\document_ingestion.exe export --format json --out "export.json"
.\build\Release\document_ingestion.exe export --format csv --out "export.csv"
```

## Tech Stack

- **C++17**
- **Data Structures**: AVL Trees (self-balancing BST), std::unordered_map
- **Persistence**: JSON files (nlohmann/json)
- **CLI**: argparse-style
- **Parsers**: TXT (direct), DOCX/XLSX (miniz + pugixml), PDF (optional Poppler)
- **Database**: SQLite (rules)

## Sample Documents

Place TXT, DOCX, XLSX, or PDF files in `sample_docs/`. A `sample.txt` with invoice content is included.

## Windows 11 Compatible

All paths and commands are Windows 11 compatible. Use PowerShell or Command Prompt.
