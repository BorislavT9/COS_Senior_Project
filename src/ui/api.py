"""
FastAPI application setup and route registration.
"""

from pathlib import Path

from fastapi import FastAPI, Request
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates

from .routes_rules import router as rules_router
from .routes_scan import router as scan_router
from .routes_results import router as results_router
from .routes_documents import router as documents_router
from .routes_logs import router as logs_router


def create_app() -> FastAPI:
    app = FastAPI(title="Document Ingestion & Extraction", version="1.0.0")

    # JSON API routes
    app.include_router(rules_router)
    app.include_router(scan_router)
    app.include_router(results_router)
    app.include_router(documents_router)
    app.include_router(logs_router)

    # HTML pages (Jinja2)
    templates_dir = Path(__file__).parent / "templates"
    static_dir = Path(__file__).parent / "static"
    if templates_dir.exists():
        templates = Jinja2Templates(directory=str(templates_dir))
        if static_dir.exists():
            app.mount("/static", StaticFiles(directory=str(static_dir)), name="static")

        @app.get("/")
        def index(request: Request):
            return templates.TemplateResponse("index.html", {"request": request})

        @app.get("/rules-page")
        def rules_page(request: Request):
            return templates.TemplateResponse("rules.html", {"request": request})

        @app.get("/results-page")
        def results_page(request: Request):
            return templates.TemplateResponse("results.html", {"request": request})

        @app.get("/logs-page")
        def logs_page(request: Request):
            return templates.TemplateResponse("logs.html", {"request": request})

    # Health
    @app.get("/health")
    def health():
        return {"status": "ok"}

    # Demo endpoint (deterministic: insert rules, run scan, return summary)
    @app.post("/demo")
    def run_demo():
        from src.config import get_db_path, get_watch_dir
        from src.db import database
        from src.db.rules_repo import insert as rule_insert, list_all as rules_list
        from src.services.ingestion_service import run_scan

        conn = database.get_connection(str(get_db_path()))
        existing = rules_list(conn, active_only=None)
        if not existing:
            rule_insert(conn, "invoice_number", r"INV-\d{4}-\d{3}", file_type=".txt", required=False, active=True)
            rule_insert(conn, "date_field", r"\d{4}-\d{2}-\d{2}", file_type=".txt", required=False, active=True)
        conn.close()

        summary = run_scan(str(get_watch_dir()))
        return {"message": "Demo completed", "summary": summary}

    return app
