"""
Report generation service: creates HTML reports from ExtractionStore.
"""

from pathlib import Path
from typing import List

from src.store.models import DocumentRecord
from src.store.store import ExtractionStore


def generate_html_report(store: ExtractionStore, output_path: Path) -> None:
    """
    Generate an HTML report showing recent extractions.
    
    Args:
        store: ExtractionStore instance
        output_path: Path to save HTML file
    """
    # Get recent documents (last 50)
    documents = list(store.documents.values())
    documents.sort(key=lambda d: d.processed_at, reverse=True)
    recent_docs = documents[:50]
    
    html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document Extraction Report</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        h1 {
            color: #333;
        }
        .summary {
            background-color: white;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        table {
            width: 100%;
            border-collapse: collapse;
            background-color: white;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #4CAF50;
            color: white;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        .status-success {
            color: green;
            font-weight: bold;
        }
        .status-failed {
            color: red;
            font-weight: bold;
        }
        .extracted-values {
            font-family: monospace;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <h1>Document Extraction Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p><strong>Total Documents:</strong> {total_docs}</p>
        <p><strong>Indexed Fields:</strong> {indexed_fields}</p>
        <p><strong>Recent Documents Shown:</strong> {recent_count}</p>
    </div>
    <h2>Recent Extractions</h2>
    <table>
        <thead>
            <tr>
                <th>File Path</th>
                <th>File Type</th>
                <th>Status</th>
                <th>Processed At</th>
                <th>Extracted Fields</th>
            </tr>
        </thead>
        <tbody>
{rows}
        </tbody>
    </table>
</body>
</html>"""
    
    # Generate table rows
    rows = []
    for doc in recent_docs:
        status_class = "status-success" if doc.status == "SUCCESS" else "status-failed"
        extracted_str = ", ".join([f"{k}: {v}" for k, v in doc.extracted.items()]) if doc.extracted else "None"
        if len(extracted_str) > 100:
            extracted_str = extracted_str[:100] + "..."
        
        rows.append(f"""            <tr>
                <td>{doc.file_path}</td>
                <td>{doc.file_type}</td>
                <td class="{status_class}">{doc.status}</td>
                <td>{doc.processed_at}</td>
                <td class="extracted-values">{extracted_str}</td>
            </tr>""")
    
    rows_html = "\n".join(rows) if rows else "            <tr><td colspan='5'>No documents processed yet.</td></tr>"
    
    # Format summary
    indexed_fields = ", ".join(sorted(store.indexes.keys())) if store.indexes else "None"
    
    html = html.format(
        total_docs=len(store.documents),
        indexed_fields=indexed_fields,
        recent_count=len(recent_docs),
        rows=rows_html
    )
    
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(html)
