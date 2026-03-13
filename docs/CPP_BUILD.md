# C++ Build Guide

This guide explains how to build and run the project (C++).

## Prerequisites

- **C++17** compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)
- **CMake** 3.16+
- **Git** (for FetchContent dependencies)

### Optional: vcpkg (recommended for SQLite)

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Install SQLite
.\vcpkg install sqlite3:x64-windows
```

## Build

### With vcpkg (if SQLite installed)

```powershell
cd "C:\Users\bobi9\Documents\Senior Project"
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### Without vcpkg (uses FetchContent for SQLite)

```powershell
cd "C:\Users\bobi9\Documents\Senior Project"
cmake -B build
cmake --build build --config Release
```

The first configure will download SQLite amalgamation, nlohmann/json, pugixml, and cpp-httplib.

## Run

```powershell
# From project root (so schema.sql and sample_docs are found)
cd "C:\Users\bobi9\Documents\Senior Project"
.\build\Release\document_ingestion.exe
```

Or with explicit path:

```powershell
.\build\Release\document_ingestion.exe ingest --watch_dir sample_docs
.\build\Release\document_ingestion.exe query --field invoice_number --equals "INV-1234-567"
.\build\Release\document_ingestion.exe list --field invoice_number
.\build\Release\document_ingestion.exe export --format json --out export.json
.\build\Release\document_ingestion.exe --demo
```

## Supported Features

- **TXT**: Full support
- **DOCX**: Full support (via miniz + pugixml)
- **XLSX**: Full support (via miniz + pugixml)
- **PDF**: Requires `-DENABLE_PDF=ON` and Poppler library

## Project Structure

```
include/document_ingestion/   # Headers
src/                           # C++ sources
  store/       # AVL tree, ExtractionStore, SearchHistory
  scanner/     # File scanning, hashing
  parsers/     # TXT, PDF, DOCX, XLSX
  rules/       # Rule engine, models
  validation/  # Normalizer, validator
  db/          # SQLite, rules repo, logs
  services/    # Ingestion, report, keyword search
  logging/     # Logger
third_party/   # miniz, picosha2
```

## Dependencies (auto-fetched)

- **nlohmann/json** – JSON
- **pugixml** – XML parsing (DOCX, XLSX)
- **cpp-httplib** – HTTP server (for future web UI)
- **SQLite** – find_package or FetchContent
- **miniz** – ZIP (DOCX, XLSX are ZIP files)
- **PicoSHA2** – SHA-256 hashing
