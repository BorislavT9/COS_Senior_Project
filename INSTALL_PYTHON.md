# Installing Python for Windows

## Quick Installation Guide

### Option 1: Download from python.org (Recommended)

1. **Download Python:**
   - Go to: https://www.python.org/downloads/
   - Click "Download Python 3.11" or latest version
   - The installer will download automatically

2. **Run the Installer:**
   - Double-click the downloaded `.exe` file
   - **IMPORTANT:** Check "Add Python to PATH" at the bottom
   - Click "Install Now"
   - Wait for installation to complete

3. **Verify Installation:**
   ```powershell
   python --version
   ```
   Should show: `Python 3.11.x` or similar

### Option 2: Install via Microsoft Store

1. Open Microsoft Store
2. Search for "Python 3.11" or "Python 3.12"
3. Click "Install"
4. Wait for installation

### Option 3: Use Windows Package Manager (winget)

```powershell
winget install Python.Python.3.11
```

## After Installation

1. **Close and reopen PowerShell** (to refresh PATH)

2. **Verify Python works:**
   ```powershell
   python --version
   pip --version
   ```

3. **Navigate to project:**
   ```powershell
   cd "C:\Senior Project"
   ```

4. **Create virtual environment:**
   ```powershell
   python -m venv .venv
   ```

5. **Activate virtual environment:**
   ```powershell
   .venv\Scripts\activate
   ```

6. **Install dependencies:**
   ```powershell
   pip install -r requirements.txt
   ```

7. **Run the program:**
   ```powershell
   python -m src.main
   ```

## Troubleshooting

### If "python" still doesn't work after installation:

1. **Find Python installation:**
   ```powershell
   Get-ChildItem "C:\Users\$env:USERNAME\AppData\Local\Programs\Python" -Recurse -Filter "python.exe" | Select-Object -First 1 -ExpandProperty FullName
   ```

2. **Add to PATH manually:**
   - Press `Win + X` → System → Advanced system settings
   - Click "Environment Variables"
   - Under "User variables", find "Path" → Edit
   - Add: `C:\Users\YourName\AppData\Local\Programs\Python\Python311` (adjust version)
   - Add: `C:\Users\YourName\AppData\Local\Programs\Python\Python311\Scripts`
   - Click OK on all dialogs
   - **Restart PowerShell**

### Alternative: Use full path

If Python is installed but not in PATH, you can use the full path:

```powershell
# Find Python
$pythonPath = Get-ChildItem "C:\Users\$env:USERNAME\AppData\Local\Programs\Python" -Recurse -Filter "python.exe" | Select-Object -First 1 -ExpandProperty FullName

# Use it directly
& $pythonPath --version
& $pythonPath -m venv .venv
```

## Quick Test

After installation, test with:
```powershell
python -c "print('Python is working!')"
```

If you see "Python is working!", you're all set!
