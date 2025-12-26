# retracesoftware_functional

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
pip install retracesoftware_functional
```

## Quick start

```python
import retracesoftware_functional as fn

# Example: simple composition (see module docs for full API)
# ...
```

## License

Apache-2.0
