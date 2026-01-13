"""
`retracesoftware.functional` can run in two modes:

- Native (C++ extension): fast, preferred when available.
- Pure Python: slower, but works on platforms where the extension cannot be loaded.

Set `RETRACESOFTWARE_FUNCTIONAL_PURE_PYTHON=1` (or `FUNCTIONAL_PURE_PYTHON=1`)
to force the pure-Python backend even if the native extension is available.
"""

from __future__ import annotations

import os
from types import ModuleType
from typing import Any


def _is_truthy_env(v: str | None) -> bool:
    if v is None:
        return False
    return v.strip().lower() in {"1", "true", "yes", "y", "on"}


_FORCE_PURE = _is_truthy_env(os.getenv("RETRACESOFTWARE_FUNCTIONAL_PURE_PYTHON")) or _is_truthy_env(
    os.getenv("FUNCTIONAL_PURE_PYTHON")
)

_backend_mod: ModuleType
__backend__: str

if not _FORCE_PURE:
    try:
        # NOTE: the compiled extension is built/installed as `_retracesoftware_functional`.
        import _retracesoftware_functional as _backend_mod  # type: ignore

        __backend__ = "native"
    except Exception:  # ImportError/OSError are the common cases; keep this broad for platform loader quirks.
        from . import _pure as _backend_mod

        __backend__ = "pure"
else:
    from . import _pure as _backend_mod

    __backend__ = "pure"


def __getattr__(name: str) -> Any:  # pragma: no cover
    return getattr(_backend_mod, name)


def _export_public(mod: ModuleType) -> None:
    g = globals()
    for k, v in mod.__dict__.items():
        if k.startswith("_"):
            continue
        g[k] = v


_export_public(_backend_mod)

__all__ = sorted([k for k in globals().keys() if not k.startswith("_")])
