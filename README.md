# Automated Document Ingestion and User-Defined Information Extraction System

## Project Summary

This senior project implements an **automated document ingestion and user-defined information extraction system** with **algorithm-focused data structures**. Documents (PDF, DOCX, TXT, XLSX) are scanned, parsed to text, and rule-based extraction (regex + optional anchors) is applied to pull structured data. 

**Key Features:**
- **AVL Tree Indexing**: Self-balancing binary search trees for O(log n) field value lookups
- **In-Memory Storage**: DocumentStore (dict) + FieldIndex (AVL trees) for fast retrieval
- **JSON Persistence**: Non-relational storage format (no SQLite dependency for primary workflow)
- **CLI Interface**: Full-featured command-line interface for ingestion and querying
- **Deterministic Normalization**: Consistent value normalization for reliable indexing

## Repo Structure

```
/docs
  architecture.md
  diagrams/
/src
  ui/
    api.py
    routes_rules.py
    routes_scan.py
    routes_results.py
    routes_logs.py
    templates/
    static/
  scanner/
    scanner.py
    registry.py
  parsers/
    factory.py
    pdf_parser.py
    docx_parser.py
    txt_parser.py
    xlsx_parser.py
  rules/
    models.py
    engine.py
    repository.py
  validation/
    validator.py
    normalizer.py
  db/
    database.py
    documents_repo.py
    rules_repo.py
    extractions_repo.py
    logs_repo.py
  logging/
    logger.py
  services/
    ingestion_service.py
    export_service.py
  config.py
  main.py
/tests
  test_rules.py
  test_validation.py
  test_db.py
  test_pipeline.py
schema.sql
README.md
requirements.txt
```

## How to Run

### 1. Create virtual environment and install dependencies

```powershell
cd "C:\Senior Project"
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Create sample documents (optional)

```powershell
python scripts\create_sample_files.py
```

This creates `sample.pdf`, `sample.docx`, `sample.xlsx` in `sample_docs/`. `sample.txt` already exists there.

### 3. Set up extraction rules (via DB or UI)

Rules are stored in SQLite (legacy support). You can:
- Use the web UI: `python -m src.main --serve` then visit http://127.0.0.1:8000
- Or insert via SQLite directly

Example rules:
- `invoice_number`: pattern `INV-\d{4}-\d{3}`
- `date_field`: pattern `\d{4}-\d{2}-\d{2}`

### 4. Ingest documents (NEW CLI)

```powershell
# Ingest from default directory (sample_docs)
python -m src.main ingest

# Ingest from specific directory
python -m src.main ingest --watch_dir "C:\path\to\documents"
```

This will:
- Scan directory for supported files
- Extract text and apply rules
- Store results in `data_store/` (JSON files)
- Generate HTML report and auto-open it (Windows)

### 5. Query extracted data

```powershell
# Exact match query
python -m src.main query --field "invoice_number" --equals "INV-1234-567"

# Range query (lexicographic)
python -m src.main query --field "invoice_number" --range "INV-1000" "INV-2000"

# List all unique values for a field
python -m src.main list --field "invoice_number"
```

### 6. Export data

```powershell
# Export to JSON
python -m src.main export --format json --out "export.json"

# Export to CSV
python -m src.main export --format csv --out "export.csv"
```

### 7. Run tests

```powershell
pytest tests\ -v
```

### 8. Legacy commands (still supported)

```powershell
# Web UI server
python -m src.main --serve

# Legacy scan (uses DB)
python -m src.main --scan

# Demo mode
python -m src.main --demo
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /health | Health check |
| GET | /rules | List rules (filter: active_only, file_type) |
| POST | /rules | Create rule |
| PUT | /rules/{id} | Update rule |
| DELETE | /rules/{id} | Delete rule |
| POST | /rules/{id}/toggle | Toggle active status |
| POST | /scan | Trigger scan (body: optional { "watch_dir": "path" }) |
| GET | /results | List extractions (filter: document_id, rule_id, status) |
| GET | /logs | List logs (filter: level) |
| POST | /demo | Run demo (insert rules + scan) |

## Tech Stack

- Python 3.11+
- **Data Structures**: AVL Trees (self-balancing BST), Python dicts
- **Persistence**: JSON files (non-relational)
- **CLI**: argparse with subcommands
- PDF: pypdf | DOCX: python-docx | XLSX: openpyxl
- pytest for testing
- FastAPI + Jinja2 (optional web UI)

## Storage Architecture

### Primary Storage (ExtractionStore)

- **DocumentStore**: `dict[doc_id -> DocumentRecord]` - O(1) lookup
- **FieldIndex**: `dict[field_name -> AVLTree]` - O(log n) search/insert
- **Persistence**: JSON files in `data_store/`
  - `store.json`: all document records
  - `index_<field>.json`: inorder traversal of AVL tree per field

### Time Complexity

- **Insert document**: O(1) for document, O(log n) per indexed field
- **Exact query**: O(log n) where n is unique values for field
- **Range query**: O(log n + k) where k is number of results
- **List values**: O(n) inorder traversal

### Legacy Database (SQLite)

- **Rules**: Still stored in SQLite (can be managed via UI)
- **Logs**: Optional logging to SQLite
- **Extractions**: Primary storage is now ExtractionStore, not DB

## Data Models

- **DocumentRecord**: doc_id, file_path, file_type, file_hash, processed_at, status, error_message, extracted (dict), extracted_raw (dict)
- **ExtractionResult**: field, raw_value, normalized_value, status, error

## Windows 11 Compatible

All paths and commands are Windows 11 compatible. Use PowerShell or Command Prompt.
