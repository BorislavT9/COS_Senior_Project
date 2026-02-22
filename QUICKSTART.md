# Quick Start Guide - Running and Testing

## Prerequisites

1. Python 3.11+ installed
2. PowerShell or Command Prompt

## Step 1: Setup Environment

```powershell
# Navigate to project directory
cd "C:\Senior Project"

# Create virtual environment
python -m venv .venv

# Activate virtual environment
.venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

## Step 2: Initialize Database and Create Sample Rules

The system needs extraction rules to work. You have two options:

### Option A: Use the Demo Command (Easiest)

```powershell
# This will create sample rules and run a scan
python -m src.main --demo
```

### Option B: Use Web UI to Create Rules

```powershell
# Start the web server
python -m src.main --serve
```

Then open http://127.0.0.1:8000 in your browser and create rules via the UI.

### Option C: Create Rules via Python Script

Create a file `setup_rules.py`:

```python
from src.config import get_db_path, ensure_dirs
from src.db import database
from src.db.database import init_db
from src.db.rules_repo import insert as rule_insert
from pathlib import Path

ensure_dirs()
db_path = get_db_path()
schema_path = Path("schema.sql")
init_db(str(db_path), str(schema_path))

conn = database.get_connection(str(db_path))

# Insert sample rules
rule_insert(conn, "invoice_number", r"INV-\d{4}-\d{3}", file_type=".txt", required=False, active=True)
rule_insert(conn, "date_field", r"\d{4}-\d{2}-\d{2}", file_type=".txt", required=False, active=True)
rule_insert(conn, "amount", r"\$\d+\.\d{2}", file_type=".txt", required=False, active=True)

conn.close()
print("Rules created successfully!")
```

Run it:
```powershell
python setup_rules.py
```

## Step 3: Create Sample Documents (if needed)

```powershell
python scripts\create_sample_files.py
```

This creates sample files in `sample_docs/` directory.

## Step 4: Run the Program (Interactive Menu)

### Option A: Interactive Menu (Recommended)

Simply run:
```powershell
python -m src.main
```

You'll see a menu with:
1. **Search for new files** - Interactive search with prompts for:
   - File type (PDF, DOCX, TXT, XLSX, or All)
   - Directory/folder path
   - Keyword/symbol to search for
   - Automatically stores found files
   - Shows statistics panel

2. **Show previously searched files** - View all past searches with:
   - File type, directory, keyword
   - Number of files found and stored
   - Timestamp
   - Statistics panel

3. **Exit** - Quit the program

### Option B: Command-Line Commands (Advanced)

```powershell
# Ingest from default directory (sample_docs)
python -m src.main ingest

# Or specify a custom directory
python -m src.main ingest --watch_dir "C:\path\to\your\documents"
```

**What happens:**
- Scans directory for PDF, DOCX, TXT, XLSX files
- Extracts text and applies rules
- Stores results in `data_store/` (JSON files)
- Generates HTML report and opens it automatically (Windows)

**Output:**
- Console summary (processed, skipped, failed, extractions)
- `data_store/store.json` - all documents
- `data_store/index_<field>.json` - AVL tree indexes per field
- `data_store/search_history.json` - search history
- `data_store/report.html` - visual report

## Step 5: Query Extracted Data

### Exact Match Query

```powershell
python -m src.main query --field "invoice_number" --equals "INV-1234-567"
```

### Range Query

```powershell
python -m src.main query --field "invoice_number" --range "INV-1000" "INV-2000"
```

### List All Values

```powershell
python -m src.main list --field "invoice_number"
```

## Step 6: Export Data

```powershell
# Export to JSON
python -m src.main export --format json --out "export.json"

# Export to CSV
python -m src.main export --format csv --out "export.csv"
```

## Step 7: Run Tests

```powershell
# Run all tests
pytest tests\ -v

# Run specific test file
pytest tests\test_avl.py -v
pytest tests\test_store.py -v
pytest tests\test_ingestion_with_store.py -v

# Run with coverage (if pytest-cov installed)
pytest tests\ --cov=src --cov-report=html
```

## Complete Demo Workflow

Here's a complete end-to-end demo:

```powershell
# 1. Setup (one time)
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt

# 2. Create rules (one time, or use --demo)
python -m src.main --demo

# 3. Run interactive menu
python -m src.main

# In the menu:
#   - Select option 1: Search for new files
#   - Choose file type (e.g., 3 for TXT)
#   - Enter directory: ./sample_docs
#   - Enter keyword: invoice
#   - View results and statistics

# 4. View previous searches
#   - Select option 2: Show previously searched files
#   - See all past searches with statistics

# 5. (Optional) Use command-line for advanced queries
python -m src.main query --field "invoice_number" --equals "INV-1234-567"
python -m src.main list --field "invoice_number"

# 6. Export
python -m src.main export --format json --out "demo_export.json"

# 7. Run tests
pytest tests\ -v
```

## Troubleshooting

### No rules found / No extractions

**Problem:** Ingestion completes but no extractions are made.

**Solution:** 
1. Check that rules exist in the database:
   ```powershell
   python -m src.main --serve
   # Visit http://127.0.0.1:8000/rules
   ```
2. Ensure rules are active and match your file types
3. Verify regex patterns match content in your documents

### Store not loading

**Problem:** `data_store/store.json` not found error.

**Solution:** This is normal on first run. The store will be created after first ingestion.

### Import errors

**Problem:** `ModuleNotFoundError` or import errors.

**Solution:**
1. Ensure you're in the project root directory
2. Activate virtual environment: `.venv\Scripts\activate`
3. Install dependencies: `pip install -r requirements.txt`
4. Run as module: `python -m src.main <command>`

### Database errors

**Problem:** SQLite database errors.

**Solution:**
1. Delete `data/app.db` and let it recreate
2. Or run: `python -m src.main` (initializes DB)

## Testing Individual Components

### Test AVL Tree

```powershell
python -c "from src.store.avl_tree import AVLTree; t = AVLTree(); t.insert('test', 'doc1'); print(t.search('test'))"
```

### Test Store

```powershell
python -c "from src.store.store import ExtractionStore; s = ExtractionStore(); print('Store created')"
```

### Test Normalization

```powershell
python -c "from src.validation.normalizer import normalize_value; print(normalize_value('  TEST   VALUE  '))"
```

## Viewing Generated Files

After ingestion, check these directories:

- `data_store/` - JSON store files
- `logs/` - Application logs
- `data/` - SQLite database (for rules)

## Next Steps

- Read `README.md` for detailed documentation
- Check `docs/architecture.md` for system design
- Explore the code in `src/store/` for AVL tree implementation
