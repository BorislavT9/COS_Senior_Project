# Architecture (Week 3 Draft)

## Overview

- **Scanner:** Recursively lists files in a watch directory; computes file hashes (SHA-256).
- **Parsers:** PDF, DOCX, TXT, XLSX → plain text (or cell values for XLSX). Factory selects by extension.
- **Rules:** Regex patterns with optional `anchor_before` / `anchor_after` to restrict search window. Engine returns extraction candidates.
- **Validation:** Validates candidates (empty, max_length, required).
- **DB:** SQLite for documents, rules, extractions, and optional logs.

## Week 3 Scope

Structure and feasibility spikes only. No full UI, no background watchers, no full persistence of extractions yet.
