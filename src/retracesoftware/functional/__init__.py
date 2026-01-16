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

# ---------------------------------------------------------------------------
# Convenience functions (originally from src/functional.py)
# ---------------------------------------------------------------------------

def sequence(*args):
    """Compose functions left-to-right: sequence(f, g, h)(x) == h(g(f(x)))."""
    if len(args) == 0:
        raise Exception("sequence requires at least one argument")
    elif len(args) == 1:
        return args[0]
    elif len(args) == 2:
        # sequence(g, f) => f(g(x))
        return _backend_mod.compose(args[1], args[0])
    else:
        return _backend_mod.compose(args[-1], sequence(*args[:-1]))


def when_not(test, then):
    """when_not(test, then)(x) -> then(x) if not test(x) else None."""
    return _backend_mod.if_then_else(test, None, then)


def when(test, then):
    """when(test, then)(x) -> then(x) if test(x) else None."""
    return _backend_mod.if_then_else(test, then, None)


def when_instanceof(cls, then):
    """when_instanceof(cls, then)(x) -> then(x) if isinstance(x, cls) else None."""
    return when(_backend_mod.isinstanceof(cls), then)


def lazy(func, *args):
    """lazy(func, *args) -> a thunk that calls func(*args) when invoked (ignores call-time args)."""
    return _backend_mod.partial(func, *args, required=0)


# Alias: singular form of dropargs
droparg = _backend_mod.dropargs


def wrap(transform, f):
    """wrap(transform, f) -> applies transform to args, calls f, then applies transform to result."""
    return _backend_mod.mapargs(transform=transform, function=sequence(f, transform))


def wrap_function(transform, f):
    """wrap_function(transform, f) -> applies transform to args before calling f."""
    return _backend_mod.mapargs(transform=transform, function=f)


def recursive_wrap(pred, transform, f):
    """recursive_wrap(pred, transform, f) -> recursively wraps when pred matches."""
    return wrap(when(pred, _backend_mod.partial(wrap, transform)), f)


__all__ = sorted([k for k in globals().keys() if not k.startswith("_")])
