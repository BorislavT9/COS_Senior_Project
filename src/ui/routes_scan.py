"""
Scan API route: trigger directory scan.
"""

from typing import Optional

from fastapi import APIRouter
from pydantic import BaseModel

from src.config import get_watch_dir
from src.services.ingestion_service import run_scan

router = APIRouter(tags=["scan"])


class ScanRequest(BaseModel):
    watch_dir: Optional[str] = None


@router.post("/scan")
def trigger_scan(body: Optional[ScanRequest] = None) -> dict:
    """
    Trigger scan. Uses watch_dir from request body, or default from config.
    """
    watch_dir = (
        (body.watch_dir if body and body.watch_dir else None)
        or str(get_watch_dir())
    )
    summary = run_scan(watch_dir)
    return {"watch_dir": watch_dir, "summary": summary}
