# Interactive Features Guide

## Overview

The program now includes a fully interactive menu system with search history and statistics tracking.

## Running the Interactive Menu

Simply run:
```powershell
python -m src.main
```

No command-line arguments needed! The program will display an interactive menu.

## Menu Options

### Option 1: Search for New Files

When you select this option, the program will:

1. **Ask for File Type:**
   - 1. PDF (.pdf)
   - 2. DOCX (.docx)
   - 3. TXT (.txt)
   - 4. XLSX (.xlsx)
   - 5. All types

2. **Ask for Directory:**
   - Enter the folder path to search
   - Program validates the path exists
   - Supports both absolute and relative paths
   - Can handle paths with or without quotes

3. **Ask for Keyword:**
   - Enter any keyword or symbol to search for
   - Case-insensitive search
   - Searches through all file content

4. **Perform Search:**
   - Scans all files in the directory
   - Searches for the keyword in file content
   - Shows results with match counts

5. **Store Results:**
   - Automatically stores found files in the database
   - Saves search to history
   - Displays statistics panel

### Option 2: Show Previously Searched Files

Displays:
- All previous searches (most recent first)
- File type, directory, and keyword for each search
- Number of files found and stored
- Timestamp of each search
- Statistics panel with totals

### Option 3: Exit

Quits the program.

## Statistics Panel

The statistics panel (displayed after searches and when viewing history) shows:

- **Total Documents Stored**: Number of documents in the database
- **Total Searches Performed**: Number of searches executed
- **Total Files Found**: Total files found across all searches
- **Total Files Stored**: Total files stored across all searches

## Search History Storage

Search history is automatically saved to:
- `data_store/search_history.json`

Each search record includes:
- Unique search ID
- File type filter
- Directory searched
- Keyword searched
- Number of files found
- Number of files stored
- Timestamp

## Example Workflow

```
1. Run: python -m src.main

2. Select option 1 (Search for new files)

3. Select file type: 3 (TXT)

4. Enter directory: ./sample_docs

5. Enter keyword: invoice

6. View results:
   - Files containing "invoice"
   - Match counts
   - Statistics panel

7. Select option 2 to view search history

8. See all previous searches with statistics
```

## Command-Line Commands Still Available

The original command-line interface is still available:

```powershell
# Ingest documents
python -m src.main ingest --watch_dir "path"

# Query by field
python -m src.main query --field "invoice_number" --equals "INV-123"

# List field values
python -m src.main list --field "invoice_number"

# Export data
python -m src.main export --format json --out "export.json"
```

## Features Summary

✅ Interactive menu system  
✅ File type selection (PDF, DOCX, TXT, XLSX, All)  
✅ Directory path input with validation  
✅ Keyword/symbol search (any character)  
✅ Search history persistence  
✅ Statistics panel display  
✅ Automatic file storage  
✅ Previous searches viewing  
