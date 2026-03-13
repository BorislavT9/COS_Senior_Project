# Quick Start Guide - C++ Version

## Prerequisites

1. C++17 compiler (e.g., Visual Studio 2019+ with C++ Desktop workload)
2. CMake 3.16+
3. PowerShell or Command Prompt

## Step 1: Build

```powershell
cd "c:\Users\bobi9\Documents\Senior Project"
cmake -B build
cmake --build build --config Release
```

## Step 2: Run Demo (create rules + scan)

```powershell
.\build\Release\document_ingestion.exe --demo
```

This will:
- Create sample extraction rules (invoice_number, date_field)
- Scan `sample_docs/`
- Store results in `data_store/`
- Generate `data_store/report.html` (opens in browser on Windows)

## Step 3: Interactive Menu

```powershell
.\build\Release\document_ingestion.exe
```

You'll see:
1. **Search for new files** – Enter directory and keyword; found files are saved to history
2. **Show previously searched files** – View all saved files with paths and keywords
3. **Exit**

For full documentation, see [DOCUMENTATION.md](DOCUMENTATION.md).

## Step 4: CLI Commands

```powershell
# Ingest
.\build\Release\document_ingestion.exe ingest
.\build\Release\document_ingestion.exe ingest --watch_dir "C:\path\to\docs"

# Query
.\build\Release\document_ingestion.exe query --field "invoice_number" --equals "INV-2024-001"
.\build\Release\document_ingestion.exe list --field "invoice_number"

# Export
.\build\Release\document_ingestion.exe export --format json --out "export.json"
```

## Troubleshooting

### CMake not found

Install CMake from https://cmake.org/download/ or via Visual Studio Installer.

### SQLite / build errors

The build uses FetchContent to download SQLite if not found. Ensure you have internet access on first configure.

### No extractions

Run `--demo` first to create rules. Ensure `sample_docs/sample.txt` exists (contains "INV-2024-001").

## Output Files

- `data_store/store.json` – document records
- `data_store/index_<field>.json` – indexes per field
- `data_store/report.html` – HTML report
- `data/app.db` – SQLite (rules)
