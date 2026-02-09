-- Week 3 draft schema for Automated Document Ingestion and Extraction System
-- SQLite

-- Documents ingested from watch directory
CREATE TABLE IF NOT EXISTS documents (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_path TEXT NOT NULL,
    file_hash TEXT NOT NULL,
    file_type TEXT NOT NULL,
    processed_at TEXT,
    status TEXT NOT NULL DEFAULT 'pending'
);

-- User-defined extraction rules (regex + optional anchors)
CREATE TABLE IF NOT EXISTS rules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    file_type TEXT,
    regex_pattern TEXT NOT NULL,
    anchor_before TEXT,
    anchor_after TEXT,
    max_length INTEGER,
    required INTEGER NOT NULL DEFAULT 0,
    active INTEGER NOT NULL DEFAULT 1
);

-- Extractions per document and rule
CREATE TABLE IF NOT EXISTS extractions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    document_id INTEGER NOT NULL,
    rule_id INTEGER NOT NULL,
    extracted_value TEXT,
    status TEXT NOT NULL DEFAULT 'ok',
    error_message TEXT,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (document_id) REFERENCES documents(id),
    FOREIGN KEY (rule_id) REFERENCES rules(id)
);

-- Application logs (optional audit)
CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    level TEXT NOT NULL,
    message TEXT NOT NULL,
    context TEXT,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);
