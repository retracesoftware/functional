# retracesoftware-functional

High-performance functional utilities for Python, implemented as a native C/C++ extension.

## Features

- Call composition and chaining
- Lazy evaluation and side-effects helpers
- Predicate combinators (and/or/not, type/case/when)
- Memoization and caching primitives
- Argument transformation, partial application, method invocation
- Thread-local proxy helpers

Backed by efficient C++ implementations with Python 3.11 vectorcall support where possible.

## Installation

```bash
pip install retracesoftware-functional
```

## Pure-Python fallback

By default, `retracesoftware.functional` will use the native C/C++ extension when it is available, and
automatically fall back to a pure-Python implementation when the extension cannot be imported/loaded
(e.g. unsupported platform, missing wheel).

To **force** the pure-Python backend even when the native extension is available, set either:

- `RETRACESOFTWARE_FUNCTIONAL_PURE_PYTHON=1`
- `FUNCTIONAL_PURE_PYTHON=1`

## Quick start

```python
import retracesoftware.functional as fn

# Example: simple composition (see module docs for full API)
# ...
```

## License

Apache-2.0
