# Automated Document Ingestion and User-Defined Information Extraction System

## Project Summary

This senior project implements an **automated document ingestion and user-defined information extraction system**. Documents (PDF, DOCX, TXT, XLSX) are scanned, parsed to text, and rule-based extraction (regex + optional anchors) is applied to pull structured data. Validation and persistence are supported via SQLite.

## Week 3 Status

- **Done:** Project scaffold with module interfaces, DB schema draft, basic logging, and 4 feasibility spikes (PDF/DOCX/TXT/XLSX parsing + regex extraction).
- **Scope:** Structure and proof-of-feasibility only. No full UI or full pipeline yet.

## Repo Structure

```
/docs
  architecture.md
  diagrams/          (empty)
/src
  ui/
  scanner/
  parsers/
  rules/
  validation/
  db/
  logging/
  spikes/
  config.py
  main.py
/tests
  test_rules.py
  test_validation.py
README.md
requirements.txt
schema.sql
```

## How to Run

### 1. Create virtual environment and install dependencies

```powershell
cd "c:\Users\bobi9\Documents\Senior Project"
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Run spikes (feasibility)

```powershell
# PDF: print first 1000 chars of extracted text
python -m src.main --spike pdf path\to\file.pdf

# DOCX
python -m src.main --spike docx path\to\file.docx

# XLSX: sample of first sheet cell values
python -m src.main --spike xlsx path\to\file.xlsx

# Regex: sample text + sample rule
python -m src.main --spike regex
```

Optional: create sample PDF/DOCX/XLSX in `sample_docs/` (sample.txt is already there):

```powershell
python scripts\create_sample_files.py
```

Or run spike modules directly:

```powershell
python -m src.spikes.spike_pdf sample_docs\sample.pdf
python -m src.spikes.spike_docx sample_docs\sample.docx
python -m src.spikes.spike_xlsx sample_docs\sample.xlsx
python -m src.spikes.spike_regex
```

### 3. Run tests

```powershell
pytest tests\ -v
```

### 4. Run main (init and optional health API)

```powershell
# Initialize folders and DB only
python -m src.main

# Start minimal FastAPI server; then GET http://127.0.0.1:8000/health
python -m src.main --serve
```

## Tech Stack (Week 3)

- Python 3.11+
- FastAPI (minimal `/health` only for Week 3)
- SQLite (local DB)
- Rule-based extraction: regex + optional anchors
- Logging: console + file
