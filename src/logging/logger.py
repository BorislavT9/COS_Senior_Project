"""
Configured logger: time, level, module, message. Console + rotating file.
"""

import logging
import sys
from logging.handlers import RotatingFileHandler
from pathlib import Path
from typing import Optional

# Format: time, level, module, message
LOG_FORMAT = "%(asctime)s | %(levelname)-8s | %(name)s | %(message)s"
DATE_FORMAT = "%Y-%m-%d %H:%M:%S"

# Default log path if not provided (will be overridden when config is loaded)
_default_log_path: Optional[Path] = None


def set_default_log_path(path: Path) -> None:
    """Set the default log file path for get_logger (used by main after config load)."""
    global _default_log_path
    _default_log_path = path


def get_logger(name: str, log_path: Optional[Path] = None) -> logging.Logger:
    """
    Return a configured logger with console and optional rotating file handler.
    Format: time, level, module, message.
    """
    logger = logging.getLogger(name)
    if logger.handlers:
        return logger

    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter(LOG_FORMAT, datefmt=DATE_FORMAT)

    # Console
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.INFO)
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    # Rotating file (use log_path, or _default_log_path, or skip file)
    path = log_path or _default_log_path
    if path is not None:
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        fh = RotatingFileHandler(
            path,
            maxBytes=2 * 1024 * 1024,  # 2 MB
            backupCount=3,
            encoding="utf-8",
        )
        fh.setLevel(logging.DEBUG)
        fh.setFormatter(formatter)
        logger.addHandler(fh)

    return logger
